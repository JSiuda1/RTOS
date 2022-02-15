#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define MAX_BUFFER_SIZE 255


char *msgPtr = NULL;
bool msgFlag = false;

void listenTask(void *parameter){
  char msg;
  char buf[MAX_BUFFER_SIZE];
  uint8_t id = 0;
  memset(buf, 0, MAX_BUFFER_SIZE);

  while(1){
    if(Serial.available() > 0){
      msg = Serial.read();
      if(id < MAX_BUFFER_SIZE - 1){
        buf[id] = msg;
        id++;
      }else{
        memset(buf, 0 , MAX_BUFFER_SIZE);
        id = 0;
      }

      if(msg == '\n'){
        buf[id - 1] = '\0';
        if(!msgFlag){
          msgPtr = (char*)pvPortMalloc(id * sizeof(char));
        }
        
        configASSERT(msgPtr); //if is NULL
        
        memcpy(msgPtr, buf, id);
        msgFlag = true;
        
        memset(buf, 0 , MAX_BUFFER_SIZE);
        id = 0;
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void echoTask(void *parameter){
  while(1){
      //Serial.println("Hello");
    if(msgFlag){
      //Serial.println("World");
      Serial.println(msgPtr);

      Serial.print("Free heap: ");
      Serial.println(xPortGetFreeHeapSize());

      vPortFree(msgPtr);
      msgPtr = NULL;
      msgFlag = false;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}


void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  xTaskCreatePinnedToCore(listenTask, "Listen Task", 2048, NULL, 2, NULL, app_cpu);
  xTaskCreatePinnedToCore(echoTask, "echo Task", 2048, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
