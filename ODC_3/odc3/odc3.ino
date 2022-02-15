#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif


void testTask(void *parameter){
  while(1){
    int a = 1;
    int b[100];

    for(int i=0; i < 100; ++i){
      b[i] = a + 1;
    }
    //Serial.println(b[0]);
  
    Serial.print("High water mark (words): ");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));

    Serial.print("Heap befor malloc (bytes): ");
    Serial.println(xPortGetFreeHeapSize());

    int *ptr = (int*)pvPortMalloc(1024 * sizeof(int));

    if(ptr == NULL){
      Serial.println("Not enough heap");
    }else{
      for(int i=0; i<1024; ++i){
        ptr[i] = 3;
      }  
    }
    

    Serial.print("Heap after malloc (bytes): ");
    Serial.println(xPortGetFreeHeapSize());

    vPortFree(ptr);
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("--FreeRTOS MEMORY---");

  xTaskCreatePinnedToCore(testTask, "Test task", 1500, NULL, 1, NULL, app_cpu);

  //delete setup and loop task
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
