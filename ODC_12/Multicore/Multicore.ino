static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

static const TickType_t time_hog = 200;

static void hog_delay(uint32_t ms){
  for (uint32_t i = 0; i < ms; i++) {
    for (uint32_t j = 0; j < 40000; j++) {
      asm("nop");
    }
  }
}

void doTaskL(void *parameter){
  TickType_t timestamp;
  char str[20];

  while(1){
    sprintf(str, "Task L, Core %i\n", xPortGetCoreID());
    Serial.print(str);

    hog_delay(time_hog);
  } 
}

void doTaskH(void *parameters) {
  TickType_t timestamp;
  char str[20];

  while (1) {

    sprintf(str, "Task H, Core %i\r\n", xPortGetCoreID());
    Serial.print(str);
    
    hog_delay(time_hog);
  }
}

void setup() {
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Priority Inheritance Demo---");

  xTaskCreatePinnedToCore(doTaskL, "Task L", 2048, NULL, 1, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(doTaskH, "Task H", 2048, NULL, 2, NULL, tskNO_AFFINITY);

  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
