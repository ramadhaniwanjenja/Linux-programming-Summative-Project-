
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <sys/socket.h>

 
 #define SERVER_IP "127.0.0.1"  
 #define PORT 8080
 #define BUFFER_SIZE 1024

 void clear_input_buffer() {
     int c;
     while ((c = getchar()) != '\n' && c != EOF);
 }

 void get_user_input(const char* prompt, char* buffer, int size) {
     printf("%s", prompt);
     fflush(stdout);
     
     if (fgets(buffer, size, stdin) != NULL) {
         // Remove trailing newline
         size_t len = strlen(buffer);
         if (len > 0 && buffer[len - 1] == '\n') {
             buffer[len - 1] = '\0';
         }
     }
 }
 
 int main() {
     int client_socket;
     struct sockaddr_in server_addr;
     char buffer[BUFFER_SIZE];
     char username[50];
     
     printf("========================================\n");
     printf("Online Examination Platform - Client\n");
     printf("========================================\n\n");
     
     // Step 1: Create socket
     client_socket = socket(AF_INET, SOCK_STREAM, 0);
     if (client_socket == -1) {
         perror("Socket creation failed");
         exit(1);
     }
     
     // Step 2: Configure server address
     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(PORT);
     
     // Convert IP address from text to binary
     if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
         perror("Invalid address");
         close(client_socket);
         exit(1);
     }
     
     // Step 3: Connect to server
     printf("Connecting to exam server...\n");
     if (connect(client_socket, (struct sockaddr*)&server_addr, 
                 sizeof(server_addr)) < 0) {
         perror("Connection failed");
         printf("Make sure the server is running on %s:%d\n", SERVER_IP, PORT);
         close(client_socket);
         exit(1);
     }
     
     printf("Connected to server successfully!\n\n");
     
     // Step 4: Authentication - send username
     get_user_input("Enter your username (e.g., student_001): ", username, sizeof(username));
     
     snprintf(buffer, BUFFER_SIZE, "USERNAME:%s", username);
     send(client_socket, buffer, strlen(buffer), 0);

     memset(buffer, 0, BUFFER_SIZE);
     int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
     
     if (bytes_received <= 0) {
         printf("Connection lost during authentication\n");
         close(client_socket);
         exit(1);
     }

     if (strncmp(buffer, "AUTH_SUCCESS", 12) == 0) {
         printf("\n%s\n", buffer);
     } else {
         printf("\nAuthentication failed: %s\n", buffer);
         close(client_socket);
         exit(1);
     }
     memset(buffer, 0, BUFFER_SIZE);
     bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
     if (bytes_received > 0) {
         printf("%s\n", buffer);
     }
     
     printf("Starting exam...\n");
     printf("========================================\n\n");
     
     int question_number = 1;
     
     while (1) {
         memset(buffer, 0, BUFFER_SIZE);
         bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
         
         if (bytes_received <= 0) {
             printf("\nConnection to server lost\n");
             break;
         }
         
         if (strstr(buffer, "EXAM COMPLETE") != NULL) {
             printf("\n%s\n", buffer);
             break;
         }
   
         printf("Question %d:\n%s\n", question_number, buffer);

         char answer[10];
         int valid_answer = 0;
         
         while (!valid_answer) {
             get_user_input("Your answer (A/B/C/D): ", answer, sizeof(answer));
       
             if (answer[0] >= 'a' && answer[0] <= 'd') {
                 answer[0] = answer[0] - 'a' + 'A';
             }

             if (strlen(answer) == 1 && answer[0] >= 'A' && answer[0] <= 'D') {
                 valid_answer = 1;
             } else {
                 printf("Invalid answer! Please enter A, B, C, or D.\n");
             }
         }
         
         // Send answer to server
         snprintf(buffer, BUFFER_SIZE, "ANSWER:%c", answer[0]);
         send(client_socket, buffer, strlen(buffer), 0);
         
         // Receive feedback
         memset(buffer, 0, BUFFER_SIZE);
         bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
         
         if (bytes_received <= 0) {
             printf("\nConnection to server lost\n");
             break;
         }
         
         // Display feedback
         printf("\n%s\n", buffer);
         printf("----------------------------------------\n\n");
         
         question_number++;
     }
     
     // Step 7: Cleanup
     printf("========================================\n");
     printf("Exam session ended. Thank you, %s!\n", username);
     printf("========================================\n");
     
     close(client_socket);
     
     return 0;
 }