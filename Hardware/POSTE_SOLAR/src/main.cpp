
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <WebServer.h>

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

#define SEALEVELPRESSURE_HPA (1013.25)    //PRESSÃO DO NIVEL DO MAR 

//--------------------------------------------------------------------------------------------------------------------------------- CONSTANTES


/*Put your SSID & Password*/
const char* ssid = "FALANGE_SUPREMA";  // Enter SSID here
const char* password = "#kinecs#";  //Enter Password here


//--------------------------------------------------------------------------------------------------------------------------------- VARIÁVEIS
float MVPORAMP = 43.56;                                 //PARA USAR O SENSOR DE 5A ALTERE O VALOR PARA 185, O DE 20A ALTERE O VALOR PARA 100 E O DE 30A ALTERE O VALOR PARA 66
double ZERO_AMPER = 1.723777;                               //ZERO DA TENSÃO DO MICROCONTROLADOR FAZ A LEITURA DO ADC INTERNO NESTE CASO O ADC LE ATE 3,3V OU SEJA 3,3*0,5=1,65V
double OFFSET_V = 0.000805664;                          //OFFSET PARA VOLTAGEM QUE NO CASO SERIA PONTOS 1023/3,3 = 3,3/x FAZENDO REGRA DE TRÊS  TEMOS x=0,00371337890625
double AMPER  = 0;                                      //GUARDA A CORRENTE APÓS CALCULOS E AQUISIÇÃO
double POTENCIA = 0;                                    //POTENCIA GERADA DEPENDENTE DA TENSÃO COLHIDA
float BME_TEMPERATURA = 0;                              //GUARDA OS DADOS DO SENSOR BME280 DADO DE: TEMPERATURA
float BME_UMIDADE = 0;                                  //GUARDA OS DADOS DO SENSOR BME280 DADO DE: UMIDADE RELATIVA
float BME_PRESSAO = 0;                                  //GUARDA OS DADOS DO SENSOR BME280 DADO DE: PRESSÃO BAROMETRICA
float BME_ALTITUDE_RELATIVA = 0;                        //GUARDA OS DADOS DO SENSOR BME280 DADO DE: ALTITUDE RELATIVA
double TENSAO_BATERIA = 0,  TENSAO_PLACA_SOLAR = 0;     //GUARDA AS TENSÕES LIDAS
double CORRENTE_BATERIA = 0,  CORRENTE_PLACA_SOLAR = 0; //GUARDA A CORRENTE LIDAS
double POTENCIA_BATERIA = 0, POTENCIA_PLACA_SOLAR = 0;  //GUARDA A POTÊNCIA LIDAS
float R1 = 10000.0;                                     //VALOR DO RESISTOR R1 PARA CALCULO DE TENSÃO
float R2 = 3000.0;                                      //VALOR DO RESISTOR R1 PARA CALCULO DE TENSÃO
float V_CC[50];                                         //DECLARA V_CC COMO UM VETOR DE 50 POSIÇÕES
int DEBOUCE = 20;                                       //VALOR DO TEMPO DE DEBOUCE(PAUSA) PARA EVITAR RUIDOS INICIAIS.
int DELAY_INICIAL = 100;                                //VALOR DE TEMPO EM ms PARA INICIAR A ROTINA DE ZERO DOS CIRECUITOS
int BARRA_PROGRESSO = 10;                               //VALOR PARA A BARRA DE PROGRESSO
int SENSOR_UV_VALOR = 0, SENSOR_UV_TENSAO = 0;          //GUARDA OS DADOS DO SENSOR UV GYML8511
bool SENSOR_PRESENCA_STATUS = 0;                        //GUARDA STATUS DO SENSOR DE PRESENÇA

//--------------------------------------------------------------------------------------------------------------------------------- PARAMETRIZAÇÃO DE HARDWARE
Adafruit_BME280 bme; //SENSOR BME280 I2C TEMPERATURA, PRESSÃO, UMIDADE.
WebServer server(80);
//--------------------------------------------------------------------------------------------------------------------------------- FUNÇOES
String SendHTML(double TB, double CB, double PB, double TPS, double CPS, double PPS, float BMET, float BMEU, float BMEP, float BMEAR, int SUVV, int SUVT, bool SPS){
  String ptr = "<!DOCTYPE html><html><head><title>POSTE SOLAR INTELIGENTE</title><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta http-equiv=\"refresh\" content=\"60\"><style>html{font-family:Arial,Helvetica,sans-serif;display:block;margin:0px auto;text-align:center;color:#fff;text-shadow:2px 2px 1px rgb(0,0,0)}body{margin:0px;background-color:rgb(255,255,255)}h1{margin:10px auto 7px}.side-by-side{display:table-cell;vertical-align:middle;position:relative}.text{font-weight:800;font-size:18px;width:200px;font-family:Arial,Helvetica,sans-serif}.reading{font-weight:800;font-size:18px;padding-right:25px;font-family:Arial,Helvetica,sans-serif}.temperature .reading{color:#F29C1F}.humidity .reading{color:#3B97D3}.pressure .reading{color:#26B99A}.altitude .reading{color:#955BA5}.PAINEL_SOLAR .reading{color:greenyellow}.BATERIA_SISTEMA .reading{color:#3B97D3}.INDICE_UV .reading{color:blueviolet}.ILUMINACAO_LED .reading{color:rgb(0,238,255)}.superscript{font-size:17px;font-weight:600;position:absolute;top:10px}.data{padding:10px}.container{display:table;margin:0 left}.icon{width:65px}.efeito_1{background-color:rgba(0, 0, 0, 0.7);padding:5px 10px 5px 10px}</style></head><body><div class='container'><div class=\"efeito_1\"><h2>POSTE SOLAR INTELIGENTE</h2></div><div class='efeito_1'><div class='data temperature'></div><div class='side-by-side icon'> <svg enable-background='new 0 0 19.438 54.003' height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003' width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px> <g> <path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982 C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718 c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833 c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22 s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z' fill=#F29C21 /> </g> </svg></div><div class='side-by-side text'>TEMPERATURA</div><div class='side-by-side reading'> ";
  ptr += BMET;
  ptr += " <span class='superscript'>&deg;C</span></div><div class='data humidity'><div class='side-by-side icon'> <svg enable-background='new 0 0 29.235 40.64' height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64' width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px> <path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617 C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426 c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425 C15.093,36.497,14.455,37.135,13.667,37.135z' fill=#3C97D3 /></svg></div><div class='side-by-side text'>UMIDADE</div><div class='side-by-side reading'> ";
  ptr += BMEU;
  ptr += " <span class='superscript'>%</span></div></div><div class='data INDICE_UV'><div class='side-by-side icon'> <svg version=\"1.1\" id=\"Capa_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\" viewBox=\"0 0 325.446 325.446\" style=\"enable-background:new 0 0 325.446 325.446;\" xml:space=\"preserve\" whidth=60px height=60px> <g> <path style=\"fill:#fffb00;\" d=\"M263.585,0.005c0.294,3.138,0.484,6.304,0.484,9.513c0,7.468-0.87,14.734-2.41,21.745 c5.483,2.23,11.873,3.535,18.721,3.535c0,0,36.627-11.172,36.627-24.96V9.192c0-3.247-2.078-6.342-5.189-9.192h-48.234V0.005z\" /> <path style=\"fill:#fffb00;\" d=\"M184.469,108.455c-7.016,1.539-14.278,2.41-21.745,2.41c-7.468,0-14.734-0.87-21.745-2.41 c-2.23,5.483-3.535,11.873-3.535,18.721c0,0,11.172,36.627,24.96,36.627h0.647c13.783,0,24.96-36.627,24.96-36.627 C188.004,120.328,186.699,113.938,184.469,108.455z\" /> <path style=\"fill:#fffb00;\" d=\"M112.064,97.163c-12.646-7.337-23.557-17.307-31.987-29.186 c-5.379,2.545-10.688,6.331-15.338,11.422c0,0-16.431,34.587-6.249,43.877l0.479,0.435c10.182,9.29,43.121-10.236,43.121-10.236 C106.739,108.384,110.019,102.749,112.064,97.163z\" /> <path style=\"fill:#fffb00;\" d=\"M266.468,123.712l0.479-0.435c10.182-9.29-6.249-43.877-6.249-43.877 c-4.645-5.091-9.959-8.871-15.338-11.422c-8.431,11.884-19.341,21.849-31.987,29.186c2.045,5.586,5.325,11.221,9.97,16.312 C223.347,113.475,256.286,133.001,266.468,123.712z\" /> <path style=\"fill:#fffb00;\" d=\"M63.787,31.264c-1.539-7.016-2.41-14.278-2.41-21.745c0-3.214,0.19-6.38,0.484-9.513H13.622 C10.511,2.85,8.439,5.945,8.439,9.192v0.647c0,13.783,36.627,24.96,36.627,24.96C51.913,34.799,58.299,33.494,63.787,31.264z\" /> <path style=\"fill:#fffb00;\" d=\"M162.718,98.327c4.95,0,9.769-0.506,14.495-1.284c12.303-2.023,23.758-6.527,33.809-13.059 c8.175-5.314,15.398-11.928,21.414-19.559c9.078-11.509,15.322-25.313,17.807-40.407c0.778-4.727,1.284-9.546,1.284-14.495 c0-3.214-0.19-6.385-0.528-9.513h-4.656H79.092h-4.656c-0.332,3.127-0.528,6.298-0.528,9.513c0,4.95,0.506,9.769,1.284,14.495 c2.486,15.088,8.73,28.892,17.813,40.401c6.021,7.631,13.244,14.245,21.414,19.559c10.051,6.532,21.501,11.036,33.809,13.059 C152.955,97.822,157.774,98.327,162.718,98.327z\" /> <path style=\"fill:#ffbb00;\" d=\"M77.983,153.817c-5.205-3.013-11.857-1.246-14.865,3.965l-34.331,59.275 c-3.008,5.194-1.235,11.852,3.965,14.86c1.719,0.995,3.59,1.469,5.439,1.469c3.753,0,7.408-1.942,9.426-5.428l34.331-59.275 C84.955,163.487,83.177,156.83,77.983,153.817z\" /> <path style=\"fill:#ffbb00;\" d=\"M269.9,157.782c-3.008-5.2-9.665-6.973-14.865-3.965c-5.2,3.013-6.973,9.665-3.96,14.86 l34.331,59.275c2.018,3.486,5.668,5.428,9.42,5.428c1.849,0,3.726-0.473,5.439-1.469c5.2-3.013,6.973-9.665,3.96-14.86 L269.9,157.782z\" /> <path style=\"fill:#ffbb00;\" d=\"M105.689,246.564c1.012,0.294,2.029,0.435,3.03,0.435c4.721,0,9.067-3.095,10.443-7.854 l17.895-61.788c1.675-5.776-1.653-11.808-7.424-13.478c-5.765-1.67-11.803,1.653-13.473,7.419l-17.895,61.788 C96.59,238.862,99.918,244.894,105.689,246.564z\" /> <path style=\"fill:#ffbb00;\" d=\"M215.83,239.884c1.675,4.302,5.782,6.929,10.138,6.929c1.316,0,2.654-0.245,3.954-0.751 c5.597-2.181,8.365-8.496,6.179-14.087l-23.959-61.418c-2.181-5.602-8.496-8.36-14.087-6.184 c-5.597,2.181-8.365,8.496-6.179,14.087L215.83,239.884z\" /> <path style=\"fill:#7a27ce;\" d=\"M154.336,259.373h-4.922c-4.498,0-8.159,4.09-8.159,9.116v28.919 c0,13.483-14.528,16.312-26.722,16.312c-9.513,0-25.466-2.121-25.466-16.312v-28.919c0-5.026-3.66-9.116-8.159-9.116h-4.917 c-4.498,0-8.159,4.09-8.159,9.116v28.74c0,18.194,16.393,28.218,46.161,28.218c31.275,0,48.5-10.144,48.5-28.561v-28.397 C162.495,263.457,158.834,259.373,154.336,259.373z\" /> <path style=\"fill:#7a27ce;\" d=\"M268.502,259.373h-5.814c-3.911,0-8.692,2.633-11.112,6.124l-17.03,24.525 c-4.449,6.44-8.028,11.797-11.03,17.003c-0.228,0.392-0.446,0.778-0.658,1.148l-0.62-1.186 c-3.03-5.646-6.728-11.498-10.269-16.981l-15.577-24.269c-2.328-3.628-7.054-6.364-11.003-6.364h-5.999 c-2.361,0-4.248,0.995-5.178,2.725c-1.001,1.871-0.707,4.253,0.81,6.543l32.814,49.561c2.366,3.573,7.114,6.266,11.047,6.266 h6.353c3.889,0,8.692-2.578,11.172-5.994l36.202-49.92c1.615-2.23,2.002-4.585,1.044-6.451 C272.772,260.368,270.895,259.373,268.502,259.373z\" /> </g> </svg></div><div class='side-by-side text'>INDICE UV</div><div class='side-by-side reading'> ";
  ptr += SUVV;
  ptr += " <span class='superscript'></span></div><div class='side-by-side text'>UV TES&Atilde;O</div><div class='side-by-side reading'> ";
  ptr += SUVT;
  ptr += " <span class='superscript'></span></div></div><div class='data pressure'><div class='side-by-side icon'> <svg enable-background='new 0 0 40.542 40.541' height=40.541px id=Layer_1 version=1.1 viewBox='0 0 40.542 40.541' width=40.542px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px> <g> <path d='M34.313,20.271c0-0.552,0.447-1,1-1h5.178c-0.236-4.841-2.163-9.228-5.214-12.593l-3.425,3.424 c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293c-0.391-0.391-0.391-1.023,0-1.414l3.425-3.424 c-3.375-3.059-7.776-4.987-12.634-5.215c0.015,0.067,0.041,0.13,0.041,0.202v4.687c0,0.552-0.447,1-1,1s-1-0.448-1-1V0.25 c0-0.071,0.026-0.134,0.041-0.202C14.39,0.279,9.936,2.256,6.544,5.385l3.576,3.577c0.391,0.391,0.391,1.024,0,1.414 c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293L5.142,6.812c-2.98,3.348-4.858,7.682-5.092,12.459h4.804 c0.552,0,1,0.448,1,1s-0.448,1-1,1H0.05c0.525,10.728,9.362,19.271,20.22,19.271c10.857,0,19.696-8.543,20.22-19.271h-5.178 C34.76,21.271,34.313,20.823,34.313,20.271z M23.084,22.037c-0.559,1.561-2.274,2.372-3.833,1.814 c-1.561-0.557-2.373-2.272-1.815-3.833c0.372-1.041,1.263-1.737,2.277-1.928L25.2,7.202L22.497,19.05 C23.196,19.843,23.464,20.973,23.084,22.037z' fill=#26B999 /> </g> </svg></div><div class='side-by-side text'>PRESS&Atilde;O</div><div class='side-by-side reading'> ";
  ptr += BMEP;
  ptr += " <span class='superscript'>hPa</span></div></div><div class='data altitude'><div class='side-by-side icon'> <svg enable-background='new 0 0 58.422 40.639' height=40.639px id=Layer_1 version=1.1 viewBox='0 0 58.422 40.639' width=58.422px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px> <g> <path d='M58.203,37.754l0.007-0.004L42.09,9.935l-0.001,0.001c-0.356-0.543-0.969-0.902-1.667-0.902 c-0.655,0-1.231,0.32-1.595,0.808l-0.011-0.007l-0.039,0.067c-0.021,0.03-0.035,0.063-0.054,0.094L22.78,37.692l0.008,0.004 c-0.149,0.28-0.242,0.594-0.242,0.934c0,1.102,0.894,1.995,1.994,1.995v0.015h31.888c1.101,0,1.994-0.893,1.994-1.994 C58.422,38.323,58.339,38.024,58.203,37.754z' fill=#955BA5 /> <path d='M19.704,38.674l-0.013-0.004l13.544-23.522L25.13,1.156l-0.002,0.001C24.671,0.459,23.885,0,22.985,0 c-0.84,0-1.582,0.41-2.051,1.038l-0.016-0.01L20.87,1.114c-0.025,0.039-0.046,0.082-0.068,0.124L0.299,36.851l0.013,0.004 C0.117,37.215,0,37.62,0,38.059c0,1.412,1.147,2.565,2.565,2.565v0.015h16.989c-0.091-0.256-0.149-0.526-0.149-0.813 C19.405,39.407,19.518,39.019,19.704,38.674z' fill=#955BA5 /> </g> </svg></div><div class='side-by-side text'>ALTITUDE</div><div class='side-by-side reading'> ";
  ptr += BMEAR;
  ptr += " <span class='superscript'>m</span></div></div><div class='data PAINEL_SOLAR'><div class='side-by-side icon'> <svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" id=\"Capa_1\" x=\"0px\" y=\"0px\" viewBox=\"0 0 512 512\" style=\"enable-background:new 0 0 512 512;\" xml:space=\"preserve\" width=\"60\" height=\"60\"> <g> <path style=\"fill:#697C86;\" d=\"M301,436v60.099h-90V436c0-8.401,6.599-15,15-15h60C294.401,421,301,427.599,301,436z\" /> <path style=\"fill:#596C76;\" d=\"M301,436v60.099h-45V421h30C294.401,421,301,427.599,301,436z\" /> <g> <g> <g> <polygon style=\"fill:#9DC1FF;\" points=\"497,436 15,436 76,166 436,166 \" /> </g> </g> </g> <path style=\"fill:#D7E9FF;\" d=\"M450.7,162.7c-1.5-6.901-7.8-11.7-14.7-11.7H76c-6.899,0-13.2,4.799-14.7,11.7l-61,270 c-0.901,4.499,0.3,9,2.999,12.599c3.001,3.6,7.2,5.7,11.7,5.7h482c4.501,0,8.699-2.1,11.7-5.7c2.699-3.6,3.9-8.101,2.999-12.599 L450.7,162.7z M122.5,421H33.6l14.5-60h81L122.5,421z M132.7,331H54.699l13.2-60h71.4L132.7,331z M142.599,241H74.801l13.2-60 H149.2L142.599,241z M241,421h-88.2l6.601-60H241V421z M241,331h-78.3l6.599-60H241V331z M241,241h-68.101l6.601-60H241V241z M271,181h61.5l6.601,60H271V181z M271,271h71.7l6.599,60H271V271z M271,421v-60h81.599l6.601,60H271z M362.8,181h61.199l13.2,60 h-67.798L362.8,181z M372.7,271h71.4l13.2,60H379.3L372.7,271z M389.5,421l-6.601-60h81l14.5,60H389.5z\" /> <polygon style=\"fill:#6396ED;\" points=\"497,436 256,436 256,166 436,166 \" /> <path style=\"fill:#9ABADB;\" d=\"M450.7,162.7c-1.5-6.901-7.8-11.7-14.7-11.7H256v300h241c4.501,0,8.699-2.1,11.7-5.7 c2.699-3.6,3.9-8.101,2.999-12.599L450.7,162.7z M271,181h61.5l6.601,60H271V181z M271,271h71.7l6.599,60H271V271z M271,421v-60 h81.599l6.601,60H271z M362.8,181h61.199l13.2,60h-67.798L362.8,181z M372.7,271h71.4l13.2,60H379.3L372.7,271z M389.5,421 l-6.601-60h81l14.5,60H389.5z\" /> <g> <path style=\"fill:#8EDB5A;\" d=\"M372.7,69.6c-6,5.999-15.3,5.999-21.301,0C325.901,44.099,292,30,256,30s-69.901,14.099-95.4,39.6 c-6,5.999-15.3,5.999-21.301,0c-5.7-6-5.7-15.3,0-21.301C170.501,17.1,211.901,0,256,0s85.499,17.1,116.7,48.3 C378.401,54.3,378.401,63.6,372.7,69.6z\" /> </g> <g> <path style=\"fill:#8EDB5A;\" d=\"M330.099,112.899c-5.698,6-15.298,6-20.999,0C295,98.8,275.501,91.599,256,91.599 s-39,7.202-53.101,21.301c-5.7,6-15.3,6-20.999,0c-6-5.7-6-15.3,0-21c19.799-20.099,47.098-30,74.099-30s54.3,9.901,74.099,30 C336.099,97.599,336.099,107.199,330.099,112.899z\" /> </g> <g> <path style=\"fill:#8EDB5A;\" d=\"M256,121c-24.901,0-45,20.099-45,45c0,24.899,20.099,45,45,45s45-20.101,45-45 C301,141.099,280.901,121,256,121z\" /> </g> <path style=\"fill:#5CC178;\" d=\"M301,166c0,24.899-20.099,45-45,45v-90C280.901,121,301,141.099,301,166z\" /> <path style=\"fill:#5CC178;\" d=\"M330.099,91.899c6,5.7,6,15.3,0,21c-5.698,6-15.298,6-20.999,0C295,98.8,275.501,91.599,256,91.599 v-29.7C283.001,61.899,310.3,71.8,330.099,91.899z\" /> <path style=\"fill:#5CC178;\" d=\"M372.7,69.6c-6,5.999-15.3,5.999-21.301,0C325.901,44.099,292,30,256,30V0 c44.099,0,85.499,17.1,116.7,48.3C378.401,54.3,378.401,63.6,372.7,69.6z\" /> <g> <path style=\"fill:#596C76;\" d=\"M361,497c0,8.399-6.599,15-15,15H166c-8.401,0-15-6.601-15-15c0-8.401,6.599-15,15-15h180 C354.401,482,361,488.599,361,497z\" /> </g> <path style=\"fill:#3B4A51;\" d=\"M361,497c0,8.399-6.599,15-15,15h-90v-30h90C354.401,482,361,488.599,361,497z\" /> </g> </svg></div><div class='side-by-side text'>TENS&Atilde;O PAINEL SOLAR</div><div class='side-by-side reading'> ";
  ptr += TPS;
  ptr += " <span class='superscript'>V</span></div><div class='side-by-side text'>CORRENTE PAINEL SOLAR</div><div class='side-by-side reading'> ";
  ptr += CPS;
  ptr += " <span class='superscript'>A</span></div><div class='side-by-side text'>POT&Ecirc;NCIA PAINEL SOLAR </div><div class='side-by-side reading'> ";
  ptr += PPS;
  ptr += " <span class='superscript'>W</span></div></div><div class='data BATERIA_SISTEMA'><div class='side-by-side icon'> <svg height=\"60pt\" viewBox=\"0 0 480 480.00012\" width=\"60pt\" xmlns=\"http://www.w3.org/2000/svg\"> <path d=\"m400 312h72v40h-72zm0 0\" fill=\"#57a4ff\" /> <path d=\"m400 272h72v40h-72zm0 0\" fill=\"#57a4ff\" /> <path d=\"m400 232h72v40h-72zm0 0\" fill=\"#57a4ff\" /> <path d=\"m400 192h72v40h-72zm0 0\" fill=\"#57a4ff\" /> <path d=\"m432 56-88 104 24-72h-40l24-32h-32l24-48h80l-48 48zm0 0\" fill=\"#ffda44\" /> <path d=\"m336 312h64v40h-64zm0 0\" fill=\"#57a4ff\" /> <path d=\"m336 272h64v40h-64zm0 0\" fill=\"#57a4ff\" /> <path d=\"m336 232h64v40h-64zm0 0\" fill=\"#57a4ff\" /> <path d=\"m336 192h64v40h-64zm0 0\" fill=\"#57a4ff\" /> <path d=\"m368 392 8 32h-80l8-32zm0 0\" fill=\"#3e3d42\" /> <g fill=\"#57a4ff\"> <path d=\"m264 312h72v40h-72zm0 0\" /> <path d=\"m264 272h72v40h-72zm0 0\" /> <path d=\"m264 232h72v40h-72zm0 0\" /> <path d=\"m264 192h72v40h-72zm0 0\" /> <path d=\"m200 312h64v40h-64zm0 0\" /> <path d=\"m200 272h64v40h-64zm0 0\" /> <path d=\"m200 232h64v40h-64zm0 0\" /> <path d=\"m200 192h64v40h-64zm0 0\" /> </g> <path d=\"m160 112c0 35.347656-28.652344 64-64 64s-64-28.652344-64-64 28.652344-64 64-64 64 28.652344 64 64zm0 0\" fill=\"#ffda44\" /> <path d=\"m136 400v72h-128v-80h128zm0 0\" fill=\"#c6c5ca\" /> <path d=\"m96 184c39.765625 0 72-32.234375 72-72s-32.234375-72-72-72-72 32.234375-72 72c.042969 39.746094 32.253906 71.957031 72 72zm0-128c30.929688 0 56 25.070312 56 56s-25.070312 56-56 56-56-25.070312-56-56c.035156-30.914062 25.085938-55.964844 56-56zm0 0\" /> <path d=\"m91.511719 30.640625c.449219.300781.9375.542969 1.449219.71875.453124.230469.9375.394531 1.4375.480469.53125.09375 1.066406.148437 1.601562.160156 2.117188-.03125 4.144531-.859375 5.679688-2.320312 1.472656-1.523438 2.304687-3.558594 2.320312-5.679688-.011719-1.046875-.230469-2.078125-.640625-3.039062-.386719-.980469-.957031-1.875-1.679687-2.640626-.398438-.320312-.800782-.640624-1.28125-.960937-.40625-.316406-.867188-.558594-1.359376-.71875-.476562-.242187-.996093-.40625-1.527343-.480469-1.023438-.238281-2.089844-.238281-3.113281 0-.5.085938-.984376.25-1.4375.480469-.511719.175781-1 .417969-1.449219.71875l-1.191407.960937c-.707031.777344-1.273437 1.671876-1.679687 2.640626-.410156.960937-.628906 1.992187-.640625 3.039062.027344 2.117188.855469 4.148438 2.320312 5.679688zm0 0\" /> <path d=\"m28.121094 55.433594c3.136718 3.03125 8.128906 2.988281 11.214844-.097656 3.085937-3.085938 3.128906-8.078126.097656-11.214844l-11.3125-11.320313c-3.128906-3.125-8.195313-3.125-11.320313 0s-3.125 8.195313 0 11.320313zm0 0\" /> <path d=\"m.640625 115.039062c.175781.511719.417969.996094.71875 1.441407.296875.417969.617187.820312.960937 1.199219 1.53125 1.460937 3.5625 2.289062 5.679688 2.320312.535156-.015625 1.070312-.066406 1.601562-.160156.5-.089844.984376-.25 1.4375-.480469.511719-.175781.996094-.417969 1.441407-.71875l1.199219-.960937c3.09375-3.152344 3.09375-8.203126 0-11.359376l-1.199219-.960937c-.445313-.300781-.929688-.542969-1.441407-.71875-.453124-.230469-.9375-.390625-1.4375-.480469-2.636718-.539062-5.367187.273438-7.28125 2.160156-.34375.378907-.664062.78125-.960937 1.199219-.300781.445313-.542969.929688-.71875 1.441407-.230469.453124-.394531.9375-.480469 1.4375-.0898435.53125-.144531 1.066406-.160156 1.601562.015625.535156.0664062 1.070312.160156 1.601562.085938.5.25.984376.480469 1.4375zm0 0\" /> <path d=\"m28.121094 168.566406-11.320313 11.3125c-3.125 3.128906-3.125 8.195313 0 11.320313s8.191407 3.125 11.320313 0l11.3125-11.3125c2.078125-2.007813 2.914062-4.980469 2.179687-7.777344-.730469-2.796875-2.917969-4.984375-5.714843-5.714844-2.796876-.730469-5.769532.101563-7.777344 2.179688zm0 0\" /> <path d=\"m90.320312 194.320312c-.707031.777344-1.273437 1.671876-1.679687 2.640626-.410156.960937-.628906 1.992187-.640625 3.039062.027344 2.117188.855469 4.148438 2.320312 5.679688l1.191407.960937c.449219.300781.9375.542969 1.449219.71875.453124.230469.9375.394531 1.4375.480469.53125.09375 1.066406.148437 1.601562.160156 2.117188-.03125 4.144531-.859375 5.679688-2.320312 1.472656-1.523438 2.304687-3.558594 2.320312-5.679688-.011719-1.046875-.230469-2.078125-.640625-3.039062-.386719-.980469-.957031-1.875-1.679687-2.640626-3.207032-2.960937-8.152344-2.960937-11.359376 0zm0 0\" /> <path d=\"m163.878906 168.566406c-2.007812-2.078125-4.980468-2.910156-7.777344-2.179687-2.796874.730469-4.984374 2.917969-5.714843 5.714843-.730469 2.796876.101562 5.769532 2.179687 7.777344l11.3125 11.320313c3.128906 3.125 8.195313 3.125 11.320313 0s3.125-8.191407 0-11.320313zm0 0\" /> <path d=\"m176 112c0 4.417969 3.582031 8 8 8s8-3.582031 8-8c-.011719-1.046875-.230469-2.078125-.640625-3.039062-.40625-.96875-.972656-1.863282-1.679687-2.640626-3.207032-2.960937-8.152344-2.960937-11.359376 0-.707031.777344-1.273437 1.671876-1.679687 2.640626-.410156.960937-.628906 1.992187-.640625 3.039062zm0 0\" /> <path d=\"m158.222656 57.777344c2.121094 0 4.15625-.84375 5.65625-2.34375l11.320313-11.3125c3.125-3.125 3.125-8.195313 0-11.320313s-8.191407-3.125-11.320313 0l-11.3125 11.320313c-2.285156 2.285156-2.96875 5.726562-1.730468 8.714844 1.238281 2.992187 4.152343 4.941406 7.386718 4.941406zm0 0\" /> <path d=\"m472 184h-272c-4.417969 0-8 3.582031-8 8v160c0 4.417969 3.582031 8 8 8h128v24h-24c-3.667969 0-6.867188 2.496094-7.761719 6.054688l-8 32c-.597656 2.390624-.0625 4.925781 1.457031 6.867187 1.515626 1.941406 3.839844 3.078125 6.304688 3.078125h32v24c0 4.417969-3.582031 8-8 8h-120c-4.417969 0-8-3.582031-8-8v-8c0-4.417969 3.582031-8 8-8h24c13.253906 0 24-10.746094 24-24s-10.746094-24-24-24h-80c0-4.417969-3.582031-8-8-8h-24v-16h-16v16h-48v-16h-16v16h-24c-4.417969 0-8 3.582031-8 8v80c0 4.417969 3.582031 8 8 8h128c4.417969 0 8-3.582031 8-8v-64h80c4.417969 0 8 3.582031 8 8s-3.582031 8-8 8h-24c-13.253906 0-24 10.746094-24 24v8c0 13.253906 10.746094 24 24 24h120c13.253906 0 24-10.746094 24-24v-24h32c2.464844 0 4.789062-1.136719 6.308594-3.078125 1.515625-1.941406 2.050781-4.476563 1.453125-6.867187l-8-32c-.894531-3.558594-4.09375-6.054688-7.761719-6.054688h-24v-24h128c4.417969 0 8-3.582031 8-8v-160c0-4.417969-3.582031-8-8-8zm-344 280h-112v-64h112zm237.753906-48h-59.503906l4-16h51.503906zm98.246094-192h-56v-24h56zm-120 0v-24h48v24zm48 16v24h-48v-24zm-64-16h-56v-24h56zm0 16v24h-56v-24zm-72 24h-48v-24h48zm0 16v24h-48v-24zm16 0h56v24h-56zm56 40v24h-56v-24zm16 0h48v24h-48zm0-16v-24h48v24zm64-24h56v24h-56zm0-16v-24h56v24zm-152-64v24h-48v-24zm-48 120h48v24h-48zm200 24v-24h56v24zm0 0\" /> <path d=\"m320 64h16l-14.398438 19.199219c-1.820312 2.425781-2.113281 5.667969-.757812 8.378906s4.125 4.421875 7.15625 4.421875h28.894531l-20.476562 61.472656c-1.210938 3.636719.328125 7.621094 3.667969 9.5 3.34375 1.878906 7.546874 1.121094 10.027343-1.804687l88-104c2.011719-2.378907 2.460938-5.710938 1.148438-8.535157-1.3125-2.828124-4.144531-4.632812-7.261719-4.632812h-36.6875l34.34375-34.34375c2.285156-2.289062 2.972656-5.726562 1.734375-8.71875-1.238281-2.988281-4.15625-4.9375-7.390625-4.9375h-80c-3.046875-.015625-5.839844 1.699219-7.199219 4.425781l-24 48c-1.242187 2.484375-1.105469 5.4375.363281 7.800781 1.46875 2.359376 4.054688 3.789063 6.835938 3.773438zm28.945312-48h55.742188l-34.34375 34.34375c-2.285156 2.289062-2.972656 5.730469-1.734375 8.71875s4.15625 4.9375 7.390625 4.9375h38.753906l-50.027344 59.121094 10.859376-32.59375c.808593-2.4375.402343-5.117188-1.101563-7.203125-1.5-2.085938-3.914063-3.320313-6.484375-3.324219h-24l14.398438-19.199219c1.820312-2.425781 2.113281-5.667969.757812-8.378906s-4.125-4.421875-7.15625-4.421875h-19.054688zm0 0\" /> <path d=\"m32 440h16v-8h8v-16h-8v-8h-16v8h-8v16h8zm0 0\" /> <path d=\"m88 416h32v16h-32zm0 0\" /> </svg></div><div class='side-by-side text'>TENS&Atilde DA BATERIA;O</div><div class='side-by-side reading'> ";
  ptr += TB;
  ptr += " <span class='superscript'>V</span></div><div class='side-by-side text'>CORRENTE DA BATERIA</div><div class='side-by-side reading'> ";
  ptr += CB;
  ptr += " <span class='superscript'>A</span></div><div class='side-by-side text'>POT&Ecirc;NCIA DA BATERIA</div><div class='side-by-side reading'> ";
  ptr += PB;
  ptr += " <span class='superscript'>W</span></div></div><div class='data ILUMINACAO_LED'><div class='side-by-side icon'> <svg height=\"60pt\" viewBox=\"-3 0 471 472\" width=\"60pt\" xmlns=\"http://www.w3.org/2000/svg\"> <path d=\"m184.5 400h96v32h-96zm0 0\" fill=\"#cfcfcd\" /> <path d=\"m256.5 400h24v32h-24zm0 0\" fill=\"#c2c2c0\" /> <path d=\"m216.5 432h32v32h-32zm0 0\" fill=\"#c2c2c0\" /> <path d=\"m184.5 456h96v16h-96zm0 0\" fill=\"#cfcfcd\" /> <path d=\"m342.613281 400h-220.226562c-6.058594 0-11.601563-3.425781-14.3125-8.847656l-11.574219-23.152344h272l-11.574219 23.152344c-2.710937 5.421875-8.253906 8.847656-14.3125 8.847656zm0 0\" fill=\"#5c5c5b\" /> <path d=\"m48.5 0h368c26.507812 0 48 21.492188 48 48v272c0 26.507812-21.492188 48-48 48h-368c-26.507812 0-48-21.492188-48-48v-272c0-26.507812 21.492188-48 48-48zm0 0\" fill=\"#5c5c5b\" /> <path d=\"m5.15625 340.511719c7.914062 16.773437 24.796875 27.480469 43.34375 27.488281h368c26.507812 0 48-21.492188 48-48v-231.753906c-128.671875 142.210937-324.800781 215.273437-459.34375 252.265625zm0 0\" fill=\"#4f4f4e\" /> <path d=\"m56.5 40h352c8.835938 0 16 7.164062 16 16v256c0 8.835938-7.164062 16-16 16h-352c-8.835938 0-16-7.164062-16-16v-256c0-8.835938 7.164062-16 16-16zm0 0\" fill=\"#a3a3a1\" /> <path d=\"m408.5 64v240c0 4.417969-3.582031 8-8 8h-336c-4.417969 0-8-3.582031-8-8v-240c0-4.417969 3.582031-8 8-8h336c4.417969 0 8 3.582031 8 8zm0 0\" fill=\"#b0b0ae\" /> <g fill=\"#e3e3e1\"> <path d=\"m248.5 88c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m184.5 88c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m120.5 88c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m376.5 88c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m312.5 88c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m248.5 152c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m184.5 152c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m120.5 152c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m376.5 152c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m312.5 152c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m248.5 216c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m184.5 216c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m120.5 216c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m376.5 216c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m312.5 216c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m248.5 280c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m184.5 280c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m120.5 280c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m376.5 280c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> <path d=\"m312.5 280c0 8.835938-7.164062 16-16 16s-16-7.164062-16-16 7.164062-16 16-16 16 7.164062 16 16zm0 0\" /> </g> <path d=\"m256.5 456h24v16h-24zm0 0\" fill=\"#c2c2c0\" /> </svg></div><div class='side-by-side text'>ILUMINA&Ccedil;&Atilde;O</div><div class='side-by-side reading'> ";
  ptr += SPS;
  ptr += " <span class='superscript'></span></div></div></div></div></div></div></body></html>";

  return ptr;
}

void CONFIGURA_PORTA(){
  pinMode(SENSOR_CORRENTE_PLACA_SOLAR, INPUT);
  pinMode(SENSOR_VOLT_PLACA_SOLAR, INPUT);
  pinMode(SENSOR_CORRENTE_BATERIA, INPUT);
  pinMode(SENSOR_VOLT_BATERIA, INPUT);
  pinMode(BTN_RESET_MANUAL, INPUT);
  pinMode(SENSOR_UV, INPUT);
  pinMode(SENSOR_LDR, INPUT);
  pinMode(SENSOR_PROXIMIDADE, INPUT);
  pinMode(LED_VIDA, OUTPUT);
  pinMode(LEDS_REFLETOR_PRIMARIO, OUTPUT);

}

void BUSCA_SENSOR_BME280(){
  
      bool status;
    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin(0x76);  
    if (!status) {
        Serial.println("!!!!! SENSOR BME280 NÃO ENCONTRADO, VERIFIQUE LIGAÇÃO !!!!!");
        while (1);
}
}

void FAZER_ZERO_SENSORES(uint8_t LER_PORTA_OFFSET){
  delay(DEBOUCE);
  for (int i = 0; i < 5; i++) {
    ZERO_AMPER += analogRead(LER_PORTA_OFFSET);
    delay(10);
  }
  ZERO_AMPER /= 5;
  ZERO_AMPER = (ZERO_AMPER / 1024);
}

float CALCULA_CORRENTE(uint8_t LER_PORTA_CORRENTE, double TENSAO_EXTERNA){ //---------------------------------------------------------CALCULA A CORRENTE GERADA PELO CIRCUITO

  double TENSAO = 0;
  double LEITURA_PORTA_CORRENTE  = 0;

  for (int i = 0; i < 500; i++) {
    LEITURA_PORTA_CORRENTE += analogRead(LER_PORTA_CORRENTE);
    delay(1);
  }
  LEITURA_PORTA_CORRENTE /= 500;
  TENSAO = (LEITURA_PORTA_CORRENTE / 1024) - ZERO_AMPER;
  AMPER = (TENSAO * 1000) / MVPORAMP;
  POTENCIA = AMPER * TENSAO_EXTERNA;

  return AMPER;
}

float CALCULA_TENSAO(uint8_t LER_PORTA_VOLT){
  float V_LIDO = 0;                                       // Declara uma variável para leitura dos valores analógicos
  float V_MEDIDO = 0;                                     // Variável que armazenará a média das amostras da tensão cc

  for (int i = 0; i < 50; i++)                            // Armaneza no vetor v_cc a tensão de entrada, em 50 espaços de memória
  {
    V_LIDO = analogRead(LER_PORTA_VOLT);                  // Lê o valor analógico e armazena na variável v_lido
    V_CC[i] = (3.3 * V_LIDO * (R1 + R2)) / (R2 * 4096);   // Faz o cálculo da tensão medida correspondente a leitura analógica
  }                                                       // fim laço for
  for (int i = 0; i < 50; i++)                            // Calcula a soma de todos os valores de tensão cc armazenados em v_cc
  {
    V_MEDIDO = V_MEDIDO + V_CC[i];                        // Somas dos valores de tensão medidos
  }                                                       // fim laço for

  return V_MEDIDO = (V_MEDIDO / 50);                      // Calcula a média de todos os valores de tensão

}

void VERIFICA_MOVIMENTO(){
  
  if(digitalRead(SENSOR_PROXIMIDADE)==0){
    digitalWrite(LEDS_REFLETOR_PRIMARIO,LOW);
    SENSOR_PRESENCA_STATUS=0;
   }else{
    digitalWrite(LEDS_REFLETOR_PRIMARIO,HIGH);
    SENSOR_PRESENCA_STATUS=1;
   }
}

void CALCULA_NIVEL_UV(){
  SENSOR_UV_VALOR = analogRead(SENSOR_UV);
  //Calcula tensao em milivolts
  SENSOR_UV_TENSAO = (SENSOR_UV_VALOR * (3.3 / 4092.0)) * 1000;
  //Compara com valores tabela UV_Index
  if (SENSOR_UV_TENSAO > 0 && SENSOR_UV_TENSAO < 50)
  {
    SENSOR_UV_VALOR = 0;
  }
  else if (SENSOR_UV_TENSAO > 50 && SENSOR_UV_TENSAO <= 227)
  {
    SENSOR_UV_VALOR = 0;
  }
  else if (SENSOR_UV_TENSAO > 227 && SENSOR_UV_TENSAO <= 318)
  {
    SENSOR_UV_VALOR = 1;
  }
  else if (SENSOR_UV_TENSAO > 318 && SENSOR_UV_TENSAO <= 408)
  {
    SENSOR_UV_VALOR = 2;
  }
  else if (SENSOR_UV_TENSAO > 408 && SENSOR_UV_TENSAO <= 503)
  {
    SENSOR_UV_VALOR = 3;
  }
  else if (SENSOR_UV_TENSAO > 503 && SENSOR_UV_TENSAO <= 606)
  {
    SENSOR_UV_VALOR = 4;
  }
  else if (SENSOR_UV_TENSAO > 606 && SENSOR_UV_TENSAO <= 696)
  {
    SENSOR_UV_VALOR = 5;
  }
  else if (SENSOR_UV_TENSAO > 696 && SENSOR_UV_TENSAO <= 795)
  {
    SENSOR_UV_VALOR = 6;
  }
  else if (SENSOR_UV_TENSAO > 795 && SENSOR_UV_TENSAO <= 881)
  {
    SENSOR_UV_VALOR = 7;
  }
  else if (SENSOR_UV_TENSAO > 881 && SENSOR_UV_TENSAO <= 976)
  {
    SENSOR_UV_VALOR = 8;
  }
  else if (SENSOR_UV_TENSAO > 976 && SENSOR_UV_TENSAO <= 1079)
  {
    SENSOR_UV_VALOR = 9;
  }
  else if (SENSOR_UV_TENSAO > 1079 && SENSOR_UV_TENSAO <= 1170)
  {
    SENSOR_UV_VALOR = 10;
  }
  else if (SENSOR_UV_TENSAO > 1170)
  {
    SENSOR_UV_VALOR = 11;
  }
}

void ESCREVE_SERIAL(){
  printf ("TENSÃO BATERIA: %4.2fV  | CORRENTE BATERIA: %4.1fA | POTÊNCIA: %4.2fW  | TENSÃO PLACA SOLAR: %4.2fV  | CORRENTE PLACA SOLAR: %4.1fA | POTÊNCIA: %4.2fW | ",TENSAO_BATERIA, CORRENTE_BATERIA, POTENCIA_BATERIA, TENSAO_PLACA_SOLAR, CORRENTE_PLACA_SOLAR, POTENCIA_PLACA_SOLAR);
  printf ("TEMPERATURA: %4.2f°C | UMIDADE: %4.1f%c  | PRESSÃO: %4.2fPa  | ALTITUDE: %4.0fm  | UV: %i - %i | PRESENÇA: %d\n", BME_TEMPERATURA, BME_UMIDADE,'%', BME_PRESSAO, BME_ALTITUDE_RELATIVA, SENSOR_UV_VALOR, SENSOR_UV_TENSAO,SENSOR_PRESENCA_STATUS);
  
}

void handle_OnConnect() {

   server.send(200, "text/html",  SendHTML(TENSAO_BATERIA, CORRENTE_BATERIA, POTENCIA_BATERIA, TENSAO_PLACA_SOLAR, CORRENTE_PLACA_SOLAR, POTENCIA_PLACA_SOLAR, BME_TEMPERATURA, BME_UMIDADE, BME_PRESSAO, BME_ALTITUDE_RELATIVA, SENSOR_UV_VALOR, SENSOR_UV_TENSAO, SENSOR_PRESENCA_STATUS)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

//--------------------------------------------------------------------------------------------------------------------------------- SETUP PADRÃO
void setup()
{
  Serial.begin(9600);

  WiFi.begin(ssid, password); //connect to your local wi-fi network
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  server.begin();
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  Serial.println("HTTP server started");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  Serial.print("CARREGANDO PROGRAMA AGUARDE: ");
  delay(DELAY_INICIAL);

  for (int i = 0; i < BARRA_PROGRESSO; i++) {
    Serial.print("|");
    delay(DELAY_INICIAL);
  }
  Serial.println();
  Serial.println("....PROGRAMA CARREGADO....");
  delay(DELAY_INICIAL);
//---------------------------------------------------------------------------------  INICIALIZAÇÕES A SEREM CARREGADAS PRIMEIRO
  
  CONFIGURA_PORTA();      //CONFIGURAS AS PORTAS DO MICROCONTROLADOR
  BUSCA_SENSOR_BME280();  //BUSCA E INICIA O SENSOR BME280

   Serial.print("CARREGANDO CONFIGURAÇÕES: ");
     delay(DELAY_INICIAL);

  for (int i = 0; i < BARRA_PROGRESSO; i++) {
    Serial.print("|");
    delay(DELAY_INICIAL);
  }
//--------------------------------------------------------------------------------- LEITURADE DADOS GUARDADOS


  Serial.println("");
  Serial.println("....CONFIGURAÇÕES CARREGADAS....");
  delay(DELAY_INICIAL);
  
//--------------------------------------------------------------------------------- FAZENDO ZERO NOS SENSORES QUE NECESSITAM
  //FAZER_ZERO_SENSORES(SENSOR_CORRENTE_PLACA_SOLAR);

  Serial.print("CALIBRANDO SENSORES: ");
  delay(DELAY_INICIAL);
  for (int i = 0; i < BARRA_PROGRESSO; i++) {
    Serial.print("|");
    delay(DELAY_INICIAL);
  }
  Serial.println("");
  Serial.println("....SENSORES FUNCIONANDO E PRÉ-CALIBRADO....");
  //------------------------------------------------------------------------------- EXIBIR PAREMETROS CARREGADOS E VALORES DE ZERO DOS SENSORES
  printf ("OFFSET ACS712: %F mV \n", ZERO_AMPER);
  delay(DELAY_INICIAL);
  }
//--------------------------------------------------------------------------------------------------------------------------------- LOOP PADRÃO
void loop()
{
  digitalWrite(LED_VIDA, LOW);
  server.handleClient();
  TENSAO_BATERIA = CALCULA_TENSAO(SENSOR_VOLT_BATERIA);
  CORRENTE_BATERIA = CALCULA_CORRENTE(SENSOR_CORRENTE_BATERIA, TENSAO_BATERIA);
  POTENCIA_BATERIA  = TENSAO_BATERIA  * CORRENTE_BATERIA;
  TENSAO_PLACA_SOLAR = CALCULA_TENSAO(SENSOR_VOLT_PLACA_SOLAR);
  CORRENTE_PLACA_SOLAR = CALCULA_CORRENTE(SENSOR_CORRENTE_PLACA_SOLAR, TENSAO_PLACA_SOLAR);
  POTENCIA_PLACA_SOLAR  = TENSAO_PLACA_SOLAR  * CORRENTE_PLACA_SOLAR;
  BME_TEMPERATURA = bme.readTemperature();
  BME_UMIDADE =  bme.readHumidity();
  BME_PRESSAO =  (bme.readPressure()/100);
  BME_ALTITUDE_RELATIVA =  bme.readAltitude(SEALEVELPRESSURE_HPA);
  VERIFICA_MOVIMENTO();
  digitalRead(SENSOR_PROXIMIDADE);
  CALCULA_NIVEL_UV();
  ESCREVE_SERIAL();
  delay(500);
  digitalWrite(LED_VIDA, HIGH);
  delay(500);
}