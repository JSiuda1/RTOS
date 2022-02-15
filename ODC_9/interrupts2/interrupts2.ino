#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint16_t timer_divider = 80; //MHz, prescaler, clock ticks at 10MHz now
static const uint64_t timer_max_count = 1000000;
static const TickType_t task_delay = 2000 / portTICK_PERIOD_MS;

static const int adc_pin = A0;

static hw_timer_t *timer = NULL;
static volatile uint16_t val;
static SemaphoreHandle_t bin_sem = NULL;


void IRAM_ATTR onTimer(){

    BaseType_t task_woken = pdFALSE;
    val = analogRead(adc_pin);

    xSemaphoreGiveFromISR(bin_sem, &task_woken);

    if(task_woken){
      portYIELD_FROM_ISR();
    }
}

void printTask(void *parameter){
  while(1){
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    Serial.println(val);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(task_delay);

  bin_sem = xSemaphoreCreateBinary();

  if(bin_sem == NULL){
    Serial.println("Could not create semaphore");
    ESP.restart();
  }

  
  xTaskCreatePinnedToCore(printTask, "print Task", 1024, NULL, 1, NULL, app_cpu);
  
  timer = timerBegin(0, timer_divider, true);

  timerAttachInterrupt(timer, &onTimer, true);

  timerAlarmWrite(timer, timer_max_count, true);

  timerAlarmEnable(timer);

}

void loop() {
  // put your main code here, to run repeatedly:

}
