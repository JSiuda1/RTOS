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
static volatile int isr_counter;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;


void IRAM_ATTR onTimer(){

    portENTER_CRITICAL_ISR(&spinlock);
    isr_counter++;
    portEXIT_CRITICAL_ISR(&spinlock);

}

void printTask(void *parameter){
  while(1){
    while(isr_counter > 0){
      Serial.println(isr_counter);

      portENTER_CRITICAL(&spinlock);
      isr_counter--;
      portEXIT_CRITICAL(&spinlock);
    }

    vTaskDelay(task_delay);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(task_delay);

  xTaskCreatePinnedToCore(printTask, "print Task", 1024, NULL, 1, NULL, app_cpu);
  
  timer = timerBegin(0, timer_divider, true);

  timerAttachInterrupt(timer, &onTimer, true);

  timerAlarmWrite(timer, timer_max_count, true);

  timerAlarmEnable(timer);

}

void loop() {
  // put your main code here, to run repeatedly:

}
