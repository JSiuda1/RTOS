#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define MAX_BUFF 255
#define configCHECK_FOR_STACK_OVERFLOW 2

static QueueHandle_t queueA;
static QueueHandle_t queueB;

static uint8_t queueA_length = 10;
static uint8_t queueB_length = 10;
char command[] = "delay";

void taskA(void *parameter){
  char msg_buff[MAX_BUFF];
  uint8_t id = 0;
  char msg_read;
  int cmd_length = strlen(command);
  char receiveMsg[MAX_BUFF];
  memset(msg_buff, 0, MAX_BUFF);
  memset(receiveMsg, 0, MAX_BUFF);
  
  while(1){
    //Serial.println("NIe wiem");
    if(Serial.available()){
      msg_read = Serial.read();
      if(id < MAX_BUFF - 1){
        msg_buff[id++] = msg_read;
      }else{
        memset(msg_buff, 0, MAX_BUFF);
        id = 0;    
      }

      if(msg_read == '\n'){
        msg_buff[id - 1] = '\0';
        //Serial.println(msg_buff);
  
        if(memcmp(msg_buff, command, cmd_length) == 0){
          char *tail = msg_buff + cmd_length;
          int user_delay = abs(atoi(tail));
          //Serial.println(user_delay);
  
          if(xQueueSend(queueA, (void*)&user_delay, 10) == pdFALSE){
            Serial.println("QueueA is FULL");
          }
          
          
        }
        memset(msg_buff, 0, MAX_BUFF);
        id = 0;
      }
    }
    
    if(xQueueReceive(queueB, &receiveMsg, 10) == pdTRUE){
      Serial.print("Task A: ");
      Serial.println(receiveMsg);
      memset(receiveMsg, 0, MAX_BUFF);  
    }
    //Serial.println("Task A przed delay");
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void taskB(void *parameter){
  const uint8_t led_pin = 2;
  int led_delay = 100;
  static uint64_t blinksCounter = 0;
  char blinkMsg[MAX_BUFF];

  pinMode(led_pin, OUTPUT);
  
  memset(blinkMsg, 0, MAX_BUFF);
  
  while(1){
    
    if(xQueueReceive(queueA, &led_delay, 0) == pdTRUE){
      Serial.print("Delay set to ");
      Serial.println(led_delay);
    }
    
    if((blinksCounter % 10 == 0)&& blinksCounter){
      sprintf(blinkMsg, "Blinks %d times", blinksCounter);
      //Serial.println(blinkMsg);
      if(xQueueSend(queueB, &blinkMsg, 10) == pdFALSE){
        Serial.println("QueueB is FULL");
      }
      memset(blinkMsg, 0, MAX_BUFF);
    }
    

    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    blinksCounter++;
  }
}


void setup() {
  Serial.begin(115200);

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  queueA = xQueueCreate(queueA_length, sizeof(int));
  queueB = xQueueCreate(queueB_length, sizeof(char[MAX_BUFF]));

  xTaskCreatePinnedToCore(taskA, "Task A", 10000, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(taskB, "Task B", 10000, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}


void loop() {
  // put your main code here, to run repeatedly:

}
