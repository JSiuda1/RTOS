#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static TaskHandle_t task_1 = NULL;
static TaskHandle_t task_2 = NULL;

static const uint8_t led_pin = 2;
uint16_t ledDelay = 1000;

void ledTask(void *parametr){
  while(1){
    digitalWrite(led_pin, HIGH);
    vTaskDelay(ledDelay /portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(ledDelay /portTICK_PERIOD_MS);
  }
  
}

void serialTask(void *parameter){
  String serialRead = "";
  uint16_t userDelay = 0;
  
  while(1){
    if(Serial.available()){
      serialRead = Serial.readStringUntil('\n');
      
      if((serialRead == "xD") && eTaskGetState(task_1) != eSuspended){
        Serial.println("Led task suspended");
        vTaskSuspend(task_1);
      }else if(serialRead == "xD"){
        Serial.println("Led task resumed");
        vTaskResume(task_1);
      }else if(serialRead != ""){
        userDelay = serialRead.toInt(); 
        Serial.print("Update LED delay to: ");
        Serial.println(userDelay);
        ledDelay = userDelay;
      }
      
      serialRead = "";
    }
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(115200);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  xTaskCreatePinnedToCore(ledTask, "ledTask", 1024, NULL, 1, &task_1, app_cpu);
  xTaskCreatePinnedToCore(serialTask, "serialTask", 1024, NULL, 2, &task_2, app_cpu);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
