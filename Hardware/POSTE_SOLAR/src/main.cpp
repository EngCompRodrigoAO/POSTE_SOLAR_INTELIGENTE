
#include  <Arduino.h>
#include <Wire.h>
#include <BME280I2C.h>

//--------------------------------------------------------------------------------------------------------------------------------- DEFINIÇÕES
#define BTN_RESET_MANUAL  2               //PINO  D2  GPIO02
#define LED_VIDA 4                        //PINO  D4  GPIO04
#define LEDS_REFLETOR_PRIMARIO  5         //PINO  D5  GPIO05
#define SENSOR_PROXIMIDADE 18             //PINO  D18 GPIO18
#define SENSOR_LDR 13                     //PINO  D13 ADC2_CH4
#define SENSOR_UV 36                      //PINO  VP  ADC1_CH0
#define SENSOR_VOLT_PLACA_SOLAR 39        //PINO  VN  ADC1_CH3
#define SENSOR_VOLT_BATERIA 34            //PINO  D34 ADC1_CH6
#define SENSOR_CORRENTE_PLACA_SOLAR 35    //PINO  D35 ADC1_CH7
#define SENSOR_CORRENTE_BATERIA 32        //PINO  D32 ADC1_CH4

//--------------------------------------------------------------------------------------------------------------------------------- VARIÁVEIS
//--------------------------------------------------------------------------------------------------------------------------------- FUNÇOES
//--------------------------------------------------------------------------------------------------------------------------------- SETUP PADRÃO
//--------------------------------------------------------------------------------------------------------------------------------- LOOP PRINCIPAL
  
void Setup(){

}
//--------------------------------------------------------------------------------------------------------------------------------- LOOP PRINCIPAL
void loop() {
  
}