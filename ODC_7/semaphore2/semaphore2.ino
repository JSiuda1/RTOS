#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const int num_task = 5;

static SemaphoreHandle_t sem_params;
static SemaphoreHandle_t serial_mutex;

typedef struct Message{
  char body[20];
  uint8_t len;
} Message;

void myTask(void *parameter){

  Message msg = *(Message*)parameter;
  //give semaphore
  xSemaphoreGive(sem_params);

  //take mutex
  while(xSemaphoreTake(serial_mutex, portMAX_DELAY) == pdFALSE);
  
  Serial.print("Received: ");
  Serial.print(msg.body);
  Serial.print(" | len:");
  Serial.println(msg.len);
  //give mutex
  xSemaphoreGive(serial_mutex);
  

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  vTaskDelete(NULL);
}

void setup() {
  Message msg;
  char text[] = "Hello space!";
  char task_name[40];
  
  Serial.begin(115200);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("--FreeRTOS semaphore--");

  sem_params = xSemaphoreCreateCounting(num_task, 0);
  serial_mutex = xSemaphoreCreateMutex();

  strcpy(msg.body, text);
  msg.len = strlen(text);

  for(int i = 0; i < num_task; ++i){
     sprintf(task_name, "Task %d", i);

     xTaskCreatePinnedToCore(myTask, task_name, 1024, (void*)&msg, 1, NULL, app_cpu);
  }

  for (int i=0; i < num_task; ++i){
    xSemaphoreTake(sem_params, portMAX_DELAY);
  }

  
  while(xSemaphoreTake(serial_mutex, portMAX_DELAY) == pdFALSE);
  Serial.println("All tasks created!");
  xSemaphoreGive(serial_mutex);
}

void loop() {
  // put your main code here, to run repeatedly:
  vTaskDelay(1000 / portTICK_PERIOD_MS);

}
