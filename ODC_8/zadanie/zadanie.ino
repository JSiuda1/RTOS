#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static TimerHandle_t led_timer = NULL;
static const uint8_t led_pin = 2;

void myTimerCallback(TimerHandle_t xTimer){
  digitalWrite(led_pin, LOW);
}

void serialTask(void *parameter){
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  
  while(1){
    if(Serial.available() > 0){
      digitalWrite(led_pin, HIGH);
      Serial.read();
      xTimerStart(led_timer, portMAX_DELAY);
    }
  }
}


void setup() {
  Serial.begin(115200);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("Write something");

  led_timer = xTimerCreate("led off timer", 5000 / portTICK_PERIOD_MS, pdFALSE, (void*)0, myTimerCallback);
  if(led_timer == NULL){
    Serial.println("Unable to create timer");
  }
  
  xTaskCreatePinnedToCore(serialTask, "Task 1", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
