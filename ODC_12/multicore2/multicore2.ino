static const BaseType_t pro_cpu = 0;
static const BaseType_t app_cpu = 1;

static const int led_pin = 2;

static SemaphoreHandle_t bin_sem;

void triggerTask(void *parameters){
  pinMode(led_pin, OUTPUT);

  while(1){
    xSemaphoreGive(bin_sem);
      
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void ledTask(void *parameters){
  while(1){
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    Serial.println("Blink");
    int state = digitalRead(led_pin);
    digitalWrite(led_pin, !state);
  }
}

void setup() {
  bin_sem = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore(triggerTask, "trigger task", 1024, NULL, 2, NULL, pro_cpu);
  xTaskCreatePinnedToCore(ledTask, "blink task", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {
  
}
