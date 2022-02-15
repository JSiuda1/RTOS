//uzycie tylko jednego core
//#include "FreeRTOS.h"
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

//Pins
static const int led_pin = 2;

//Task - świecenie diodą
void toggleLED(void *parameter){
  
  while(1){
    digitalWrite(led_pin, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // nie blokujący timer
    digitalWrite(led_pin, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // nie blokujący timer
  }
}

void setup() {
  //konfig pinów
  pinMode(led_pin, OUTPUT);

  //xTaskCreate() //run on random core
  xTaskCreatePinnedToCore( //run on choosen core 
    toggleLED, //Function to be called
    "Toggle LED", //Task name
    1024,         // stack size (bytes esp32, words rtos
    NULL,         //Paramter to pass to function
    1,            //Task priority (0 to configMAX_PRIORITIES - 1) higher number higer prioirty
    NULL,         //Task handle
    app_cpu);     //run on one core (ESP32 only)


  //vTaskStartScheduler(); in vanilla FreeRTOS
}

void loop() {
  // put your main code here, to run repeatedly:

}
