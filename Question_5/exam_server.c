 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <arpa/inet.h>
 #include <sys/socket.h>
 #include <signal.h>

 #define PORT 8080
 #define MAX_CLIENTS 4
 #define BUFFER_SIZE 1024
 #define USERNAME_SIZE 50
 #define ANSWER_SIZE 10

 typedef struct {
     char question[256];
     char options[4][100];
     char correct_answer;
 } ExamQuestion;
 
 // Client information structure
 typedef struct {
     int socket_fd;
     char username[USERNAME_SIZE];
     int authenticated;
     int active;
     pthread_t thread_id;
 } ClientInfo;

 ClientInfo clients[MAX_CLIENTS];
 pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
 
 // Server socket
 int server_socket;
 
 // Exam questions database
 ExamQuestion questions[] = {
     {
         "What is the time complexity of binary search?",
         {"A) O(n)", "B) O(log n)", "C) O(n^2)", "D) O(1)"},
         'B'
     },
     {
         "Which data structure uses LIFO principle?",
         {"A) Queue", "B) Stack", "C) Tree", "D) Graph"},
         'B'
     },
     {
         "What does CPU stand for?",
         {"A) Central Processing Unit", "B) Computer Personal Unit", 
          "C) Central Processor Unit", "D) Computer Processing Unit"},
         'A'
     },
     {
         "In TCP/IP, what layer handles routing?",
         {"A) Application", "B) Transport", "C) Network", "D) Data Link"},
         'C'
     }
 };
 
 int num_questions = sizeof(questions) / sizeof(questions[0]);

 void initialize_clients() {
     for (int i = 0; i < MAX_CLIENTS; i++) {
         clients[i].socket_fd = -1;
         clients[i].authenticated = 0;
         clients[i].active = 0;
         memset(clients[i].username, 0, USERNAME_SIZE);
     }
 }
 
 int add_client(int socket_fd, const char* username) {
     pthread_mutex_lock(&clients_mutex);
     
     for (int i = 0; i < MAX_CLIENTS; i++) {
         if (!clients[i].active) {
             clients[i].socket_fd = socket_fd;
             strncpy(clients[i].username, username, USERNAME_SIZE - 1);
             clients[i].authenticated = 1;
             clients[i].active = 1;
             pthread_mutex_unlock(&clients_mutex);
             return i;
         }
     }
     
     pthread_mutex_unlock(&clients_mutex);
     return -1;  // No space available
 }

 void remove_client(int index) {
     pthread_mutex_lock(&clients_mutex);
     
     if (index >= 0 && index < MAX_CLIENTS) {
         clients[index].active = 0;
         clients[index].authenticated = 0;
         close(clients[index].socket_fd);
         printf("[SERVER] Client %s disconnected\n", clients[index].username);
     }
     
     pthread_mutex_unlock(&clients_mutex);
 }

 void get_active_students(char* buffer, int size) {
     pthread_mutex_lock(&clients_mutex);
     
     snprintf(buffer, size, "\n=== Active Students ===\n");
     int count = 0;
     
     for (int i = 0; i < MAX_CLIENTS; i++) {
         if (clients[i].active && clients[i].authenticated) {
             char temp[100];
             snprintf(temp, sizeof(temp), "%d. %s\n", ++count, clients[i].username);
             strncat(buffer, temp, size - strlen(buffer) - 1);
         }
     }
     
     if (count == 0) {
         strncat(buffer, "No students currently active.\n", 
                 size - strlen(buffer) - 1);
     }
     
     strncat(buffer, "=======================\n", size - strlen(buffer) - 1);
     
     pthread_mutex_unlock(&clients_mutex);
 }

 void format_question(ExamQuestion* q, char* buffer, int size) {
     snprintf(buffer, size, 
              "\n=== EXAM QUESTION ===\n%s\n%s\n%s\n%s\n%s\n"
              "====================\n",
              q->question, q->options[0], q->options[1], 
              q->options[2], q->options[3]);
 }

 void* handle_client(void* arg) {
     int client_socket = *(int*)arg;
     free(arg);
     
     char buffer[BUFFER_SIZE];
     char username[USERNAME_SIZE];
     int client_index = -1;
     

     memset(buffer, 0, BUFFER_SIZE);
     int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
     
     if (bytes_received <= 0) {
         close(client_socket);
         return NULL;
     }
     if (sscanf(buffer, "USERNAME:%s", username) != 1) {
         send(client_socket, "AUTH_FAILED:Invalid format\n", 27, 0);
         close(client_socket);
         return NULL;
     }
     
     client_index = add_client(client_socket, username);
     
     if (client_index == -1) {
         send(client_socket, "AUTH_FAILED:Server full\n", 24, 0);
         close(client_socket);
         return NULL;
     }
     
  
     snprintf(buffer, BUFFER_SIZE, "AUTH_SUCCESS:Welcome %s!\n", username);
     send(client_socket, buffer, strlen(buffer), 0);
     
     printf("[SERVER] Student %s authenticated successfully\n", username);
     
     // Step 2: Send active students list
     get_active_students(buffer, BUFFER_SIZE);
     send(client_socket, buffer, strlen(buffer), 0);
     
     // Step 3: Conduct exam - send questions and check answers
     int correct_count = 0;
     
     for (int q = 0; q < num_questions; q++) {
         memset(buffer, 0, BUFFER_SIZE);
         format_question(&questions[q], buffer, BUFFER_SIZE);
         send(client_socket, buffer, strlen(buffer), 0);
         
         printf("[SERVER] Sent question %d to %s\n", q + 1, username);
         
         // Receive answer
         memset(buffer, 0, BUFFER_SIZE);
         bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
         
         if (bytes_received <= 0) {
             printf("[SERVER] %s disconnected during exam\n", username);
             break;
         }
         char student_answer;
         if (sscanf(buffer, "ANSWER:%c", &student_answer) != 1) {
             send(client_socket, "ERROR:Invalid answer format\n", 28, 0);
             continue;
         }
         char feedback[BUFFER_SIZE];
         if (student_answer == questions[q].correct_answer) {
             snprintf(feedback, BUFFER_SIZE, 
                     "Server: Correct! The answer is %c\n", 
                     questions[q].correct_answer);
             correct_count++;
         } else {
             snprintf(feedback, BUFFER_SIZE, 
                     "Server: Incorrect. The correct answer is %c\n", 
                     questions[q].correct_answer);
         }
         
         send(client_socket, feedback, strlen(feedback), 0);
         printf("[SERVER] %s answered question %d: %c (%s)\n", 
                username, q + 1, student_answer, 
                (student_answer == questions[q].correct_answer) ? "Correct" : "Incorrect");
     }
     
     // Step 4: Send exam completion message with score
     snprintf(buffer, BUFFER_SIZE, 
              "\n=== EXAM COMPLETE ===\nYour Score: %d/%d\n"
              "Exam session ended. Thank you, %s.\n"
              "====================\n",
              correct_count, num_questions, username);
     send(client_socket, buffer, strlen(buffer), 0);
     
     printf("[SERVER] %s completed exam. Score: %d/%d\n", 
            username, correct_count, num_questions);
     
     // Step 5: Cleanup
     remove_client(client_index);
     
     return NULL;
 }

 void signal_handler(int sig) {
     printf("\n[SERVER] Shutting down...\n");
     
     // Close all client connections
     pthread_mutex_lock(&clients_mutex);
     for (int i = 0; i < MAX_CLIENTS; i++) {
         if (clients[i].active) {
             close(clients[i].socket_fd);
         }
     }
     pthread_mutex_unlock(&clients_mutex);
     
     // Close server socket
     close(server_socket);
     
     exit(0);
 }

 int main() {
     struct sockaddr_in server_addr, client_addr;
     socklen_t client_len = sizeof(client_addr);
     
     // Setup signal handler
     signal(SIGINT, signal_handler);
     
     // Initialize clients array
     initialize_clients();
     
     printf("========================================\n");
     printf("Online Examination Platform - Server\n");
     printf("========================================\n");
     printf("Configuration:\n");
     printf("  - Port: %d\n", PORT);
     printf("  - Max concurrent clients: %d\n", MAX_CLIENTS);
     printf("  - Number of questions: %d\n", num_questions);
     printf("========================================\n\n");
     
     // Create socket
     server_socket = socket(AF_INET, SOCK_STREAM, 0);
     if (server_socket == -1) {
         perror("Socket creation failed");
         exit(1);
     }
     
     // Set socket options to reuse address
     int opt = 1;
     if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, 
                    &opt, sizeof(opt)) < 0) {
         perror("setsockopt failed");
         exit(1);
     }
     
     // Configure server address
     server_addr.sin_family = AF_INET;
     server_addr.sin_addr.s_addr = INADDR_ANY;
     server_addr.sin_port = htons(PORT);
     
     // Bind socket
     if (bind(server_socket, (struct sockaddr*)&server_addr, 
              sizeof(server_addr)) < 0) {
         perror("Bind failed");
         exit(1);
     }
     
     // Listen for connections
     if (listen(server_socket, MAX_CLIENTS) < 0) {
         perror("Listen failed");
         exit(1);
     }
     
     printf("[SERVER] Listening on port %d...\n\n", PORT);
     
     // Main accept loop - handle incoming connections
     while (1) {
         int* client_socket = malloc(sizeof(int));
         *client_socket = accept(server_socket, 
                                (struct sockaddr*)&client_addr, 
                                &client_len);
         
         if (*client_socket < 0) {
             perror("Accept failed");
             free(client_socket);
             continue;
         }
         
         printf("[SERVER] New connection from %s:%d\n",
                inet_ntoa(client_addr.sin_addr),
                ntohs(client_addr.sin_port));
         
         // Create thread to handle client
         pthread_t thread_id;
         if (pthread_create(&thread_id, NULL, handle_client, 
                           client_socket) != 0) {
             perror("Thread creation failed");
             close(*client_socket);
             free(client_socket);
             continue;
         }
         
         // Detach thread so resources are freed automatically
         pthread_detach(thread_id);
     }
     
     // Cleanup
     close(server_socket);
     pthread_mutex_destroy(&clients_mutex);
     
     return 0;
 }