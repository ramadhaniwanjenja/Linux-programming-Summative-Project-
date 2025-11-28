
 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <unistd.h>
 #include <time.h>
 

 
 #define MAX_QUEUE_SIZE 8        
 #define BARISTA_PREP_TIME 4     
 #define WAITER_SERVE_TIME 3     
#define TOTAL_DRINKS 15        

 typedef struct {
     int drinks[MAX_QUEUE_SIZE];  
     int front;                    
     int rear;
     int count;                    
 } OrderQueue;
 
 // Global shared resources
 OrderQueue queue = {.front = 0, .rear = 0, .count = 0};
 int drinks_prepared = 0;         
 int drinks_served = 0;           
 
 // Synchronization primitives
 pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;  
 pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;  
 pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER; 
 

 int is_queue_full() {
     return queue.count == MAX_QUEUE_SIZE;
 }
 
 
 int is_queue_empty() {
     return queue.count == 0;
 }
 

 void enqueue_drink(int drink_id) {
     queue.drinks[queue.rear] = drink_id;
     queue.rear = (queue.rear + 1) % MAX_QUEUE_SIZE;  
     queue.count++;
 }
 

 int dequeue_drink() {
     int drink_id = queue.drinks[queue.front];
     queue.front = (queue.front + 1) % MAX_QUEUE_SIZE;  
     queue.count--;
     return drink_id;
 }
 

 void* barista_thread(void* arg) {
     for (int i = 1; i <= TOTAL_DRINKS; i++) {
         pthread_mutex_lock(&queue_mutex);
        
         while (is_queue_full()) {
             printf("[BARISTA] Queue is full (%d drinks). Waiting for space...\n",  queue.count);
             pthread_cond_wait(&queue_not_full, &queue_mutex);
         }
         
         pthread_mutex_unlock(&queue_mutex);
         printf("[BARISTA] Preparing drink #%d... (takes %d seconds)\n", 
                i, BARISTA_PREP_TIME);
         sleep(BARISTA_PREP_TIME);
         pthread_mutex_lock(&queue_mutex);
         enqueue_drink(i);
         drinks_prepared++;
         printf("[BARISTA] Drink #%d ready! Added to queue. Queue size: %d/%d\n", 
                i, queue.count, MAX_QUEUE_SIZE);
         pthread_cond_signal(&queue_not_empty);
         
         pthread_mutex_unlock(&queue_mutex);
         
     }
     
     printf("[BARISTA] Finished preparing all %d drinks. Going on break.\n", 
            TOTAL_DRINKS);
     
     return NULL;
 }
 
 void* waiter_thread(void* arg) {
     while (1) {
      
         pthread_mutex_lock(&queue_mutex);
      
         while (is_queue_empty() && drinks_prepared < TOTAL_DRINKS) {
             printf("[WAITER] No drinks in queue. Waiting... Queue size: %d\n", 
                    queue.count);
             pthread_cond_wait(&queue_not_empty, &queue_mutex);
         }
        
         if (is_queue_empty() && drinks_prepared >= TOTAL_DRINKS) {
             pthread_mutex_unlock(&queue_mutex);
             break;  
         }
       
         int drink_id = dequeue_drink();
         printf("[WAITER] Picked up drink #%d from queue. Queue size: %d/%d\n", 
                drink_id, queue.count, MAX_QUEUE_SIZE);
     
         pthread_cond_signal(&queue_not_full);
         
         pthread_mutex_unlock(&queue_mutex);
       
         printf("[WAITER] Serving drink #%d to customer... (takes %d seconds)\n", 
                drink_id, WAITER_SERVE_TIME);
         sleep(WAITER_SERVE_TIME);
         
         pthread_mutex_lock(&queue_mutex);
         drinks_served++;
         printf("[WAITER] Drink #%d served successfully! Total served: %d\n", 
                drink_id, drinks_served);
         pthread_mutex_unlock(&queue_mutex);
     }
     
     printf("[WAITER] All drinks served. Shift complete!\n");
     
     return NULL;
 }

 int main() {
     pthread_t barista, waiter;
     
     printf("========================================\n");
     printf("Coffee Shop Order System Simulation\n");
     printf("========================================\n");
     printf("Configuration:\n");
     printf("  - Max queue size: %d drinks\n", MAX_QUEUE_SIZE);
     printf("  - Barista prep time: %d seconds/drink\n", BARISTA_PREP_TIME);
     printf("  - Waiter serve time: %d seconds/drink\n", WAITER_SERVE_TIME);
     printf("  - Total drinks to process: %d\n", TOTAL_DRINKS);
     printf("========================================\n\n");
     

     if (pthread_create(&barista, NULL, barista_thread, NULL) != 0) {
         perror("Failed to create barista thread");
         return 1;
     }
     
     if (pthread_create(&waiter, NULL, waiter_thread, NULL) != 0) {
         perror("Failed to create waiter thread");
         return 1;
     }
    
     pthread_join(barista, NULL);
     pthread_join(waiter, NULL);
     
 
     printf("\n========================================\n");
     printf("Simulation Complete!\n");
     printf("========================================\n");
     printf("Summary:\n");
     printf("  - Drinks prepared: %d\n", drinks_prepared);
     printf("  - Drinks served: %d\n", drinks_served);
     printf("  - Final queue size: %d\n", queue.count);
     printf("========================================\n");
     
     // Cleanup
     pthread_mutex_destroy(&queue_mutex);
     pthread_cond_destroy(&queue_not_full);
     pthread_cond_destroy(&queue_not_empty);
     
     return 0;
 }

 