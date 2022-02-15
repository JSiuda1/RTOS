#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
enum {BUF_SIZE = 5};                  // Size of buffer array
static const int num_prod_tasks = 5;  // Number of producer tasks
static const int num_cons_tasks = 2;  // Number of consumer tasks
static const int num_writes = 3;      // Num times each producer writes to buf

// Globals
static int buf[BUF_SIZE];             // Shared buffer
static int head = 0;                  // Writing index to buffer
static int tail = 0;                  // Reading index to buffer
static SemaphoreHandle_t bin_sem;     // Waits for parameter to be read

static SemaphoreHandle_t mutex;
static SemaphoreHandle_t head_sem;
static SemaphoreHandle_t tail_sem;

//*****************************************************************************
// Tasks

// Producer: write a given number of times to shared buffer
void producer(void *parameters) {

  // Copy the parameters into a local variable
  int num = *(int *)parameters;

  // Release the binary semaphore
  xSemaphoreGive(bin_sem);

  // Fill shared buffer with task number
  for (int i = 0; i < num_writes; i++) {

    
    xSemaphoreTake(tail_sem, portMAX_DELAY); //wait for place in buffer if its full

    // Critical section (accessing shared buffer)
    while(xSemaphoreTake(mutex, portMAX_DELAY) == pdFALSE);  //sharde buffor protection

    buf[head] = num;
    head = (head + 1) % BUF_SIZE;
    
    xSemaphoreGive(mutex);
    
    xSemaphoreGive(head_sem); //add one to head_sem and inform that data is in buffer
  }

  // Delete self task
  vTaskDelete(NULL);
}

// Consumer: continuously read from shared buffer
void consumer(void *parameters) {

  int val;

  // Read from buffer
  while (1) {

    // Critical section (accessing shared buffer and Serial)
    xSemaphoreTake(head_sem, portMAX_DELAY);
    while(xSemaphoreTake(mutex, portMAX_DELAY) == pdFALSE);
    
    val = buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    Serial.println(val);
    xSemaphoreGive(mutex);
    
    xSemaphoreGive(tail_sem);
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  char task_name[12];
  
  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Semaphore Alternate Solution---");

  // Create mutexes and semaphores before starting tasks
  bin_sem = xSemaphoreCreateBinary();
  mutex = xSemaphoreCreateMutex();
  head_sem = xSemaphoreCreateCounting(BUF_SIZE, 0);
  tail_sem = xSemaphoreCreateCounting(BUF_SIZE, 5);

  // Start producer tasks (wait for each to read argument)
  for (int i = 0; i < num_prod_tasks; i++) {
    sprintf(task_name, "Producer %i", i);
    xTaskCreatePinnedToCore(producer,
                            task_name,
                            1024,
                            (void *)&i,
                            1,
                            NULL,
                            app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }

  // Start consumer tasks
  for (int i = 0; i < num_cons_tasks; i++) {
    sprintf(task_name, "Consumer %i", i);
    xTaskCreatePinnedToCore(consumer,
                            task_name,
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);
  }

  // Notify that all tasks have been created
  Serial.println("All tasks created");
}

void loop() {
  
  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
