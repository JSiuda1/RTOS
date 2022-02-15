#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint16_t timer_divider = 8; //10MHz
static const uint64_t timer_max_count = 1000000;

static hw_timer_t *timer = NULL;
static TaskHandle_t proccesing_task = NULL;
static QueueHandle_t val_queue = NULL;
static SemaphoreHandle_t mutex = NULL;
static int queue_length = 10;
static float global_avg = 0;

void IRAM_ATTR onTimer(){
  static uint8_t samples;
  int val = 0;

  BaseType_t task_woken = pdFALSE;
  
  val = analogRead(A0);
  
  if(xQueueSend(val_queue,(void*)&val, 0) == pdFALSE){
    Serial.println("queue full");
  }
  
  samples++;
  if(samples % 10 == 9){
    //notify
    vTaskNotifyGiveFromISR(proccesing_task, &task_woken);
    if(task_woken){
      portYIELD_FROM_ISR();
    }
    samples = 0;
  }
}

void proccesingTask(void *parameter){
  float avg;
  int val;
  
  while(1){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    avg = 0.0;
    
    for(int i=0; i<10; ++i){
      xQueueReceive(val_queue, (void*)&val, 10);
      avg += val;
    }
    avg = avg / 10;
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    global_avg = avg;
    xSemaphoreGive(mutex);
  }
}

void serialTask(void *parameter){
  const int buff_size = 255;
  char buff[buff_size];
  char id = 0;
  char serial_input;
  char command[] = "avg";

  memset(buff, 0, buff_size);
  
  while(1){
    if(Serial.available() > 0){
      serial_input = Serial.read();
      
      if(id < buff_size - 1){
        buff[id++] = serial_input;
      }else{
        memset(buff, 0, buff_size);
        id = 0;
      }

      if(serial_input == '\n'){
        buff[id - 1] = '\0';

        if(memcmp(buff, command, sizeof(command)) == 0){
           xSemaphoreTake(mutex, portMAX_DELAY);
           Serial.println(global_avg);
           xSemaphoreGive(mutex);
        }else{
          Serial.println(buff);
        }

        memset(buff, 0, buff_size);
        id = 0;
      }

      vTaskDelay(20 / portTICK_PERIOD_MS);
    }
  }
}


void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  Serial.println("---Write avg---");

  val_queue = xQueueCreate(queue_length, sizeof(int));
  if(val_queue == NULL){
    Serial.println("Kolejka nie działa");
  }
  mutex = xSemaphoreCreateMutex();
  
  timer = timerBegin(0, timer_divider, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, timer_max_count, true);
  timerAlarmEnable(timer);

  xTaskCreatePinnedToCore(proccesingTask, "Proccesing task", 1500, NULL, 1, &proccesing_task, app_cpu);
  xTaskCreatePinnedToCore(serialTask, "Serial task", 1500, NULL, 1, NULL, app_cpu);

  
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
