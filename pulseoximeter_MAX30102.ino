#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "algorithm_by_RF.h"
#include "max30102.h"
#include "MAX30105.h"                                     // MAX3010x library
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>                                 // OLED Library
#include <Adafruit_SSD1306.h>                             // OLED Library

//#define DEBUG                                           // Uncomment for debug output to the Serial stream

#define SCREEN_WIDTH 128                                  // OLED display width, in pixels
#define SCREEN_HEIGHT 64                                  // OLED display height, in pixels
#define OLED_RESET    -1                                  // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)

// Interrupt pin
const byte oxiInt = D8;                                   // pin connected to MAX30102 INT
byte readLED = D4;                                        //Blinks with each data read
MAX30105 particleSensor;

uint32_t aun_ir_buffer[BUFFER_SIZE];                      //infrared LED sensor data
uint32_t aun_red_buffer[BUFFER_SIZE];                     //red LED sensor data
float old_n_spo2;                                         // Previous SPO2 value
uint8_t uch_dummy,k;
bool isFingerPlaced = false;

void setup() {
  Wire.setClock(400000);                                  // Set I2C speed to 400kHz
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);              // Start the OLED display
  display.setTextColor(WHITE);
  
  pinMode(oxiInt, INPUT);                                 //pin D10 connects to the interrupt output pin of the MAX30102
  pinMode(readLED, OUTPUT);
  Wire.begin();

  particleSensor.begin(Wire, I2C_SPEED_STANDARD);         // Use default I2C port, 400kHz speed
  particleSensor.setup();  
  
  Serial.begin(115200);
  maxim_max30102_reset();                                 //resets the MAX30102
  delay(1000);
  maxim_max30102_read_reg(REG_INTR_STATUS_1,&uch_dummy);  //Reads/clears the interrupt status register
  maxim_max30102_init();                                  //initialize the MAX30102
  old_n_spo2=0.0;

  uint8_t revID;
  uint8_t partID;

  maxim_max30102_read_reg(0xFE, &revID);
  maxim_max30102_read_reg(0xFF, &partID);

  #ifdef DEBUG
    Serial.print("Rev ");
    Serial.print(revID, HEX);
    Serial.print(" Part ");
    Serial.println(partID, HEX);
    Serial.print("-------------------------------------");
  #endif

}

//Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every ST seconds
void loop() {
   processHRandSPO2();
}

void setDisplay(int32_t hr, float spo2, String msg){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("BPM:");
  if(hr == -999)
    display.print("-");
  else display.print(hr);
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("SpO2:");
  if(hr == -999)
    display.print("-");
  else display.print(spo2);
  display.setCursor(0, 45);
  display.setTextSize(1);
  display.print(msg);
  display.display();
}

void processHRandSPO2(){
  long irValue = particleSensor.getIR();                  // Reading the IR value it will permit us to know if there's a finger on the sensor or not
  if (irValue > 50000){
    if(isFingerPlaced == false){
      isFingerPlaced = true;
      setDisplay(-999, -999, "measuring vitals...");
    }
    float n_spo2,ratio,correl;                            //SPO2 value
    int8_t ch_spo2_valid;                                 //indicator to show if the SPO2 calculation is valid
    int32_t n_heart_rate;                                 //heart rate value
    int8_t  ch_hr_valid;                                  //indicator to show if the heart rate calculation is valid
    int32_t i;
    char hr_str[10];
       
    //buffer length of BUFFER_SIZE stores ST seconds of samples running at FS sps
    //read BUFFER_SIZE samples, and determine the signal range
    for(i=0;i<BUFFER_SIZE;i++)
    {
      while(digitalRead(oxiInt)==1){                      //wait until the interrupt pin asserts
         yield();
      }
      digitalWrite(readLED, !digitalRead(readLED));
      
      //IMPORTANT:
      //IR and LED are swapped here for MH-ET MAX30102. Check your vendor for MAX30102
      //and use this or the commented-out function call.
      maxim_max30102_read_fifo((aun_ir_buffer+i), (aun_red_buffer+i));
      //maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));
    }
  
    //calculate heart rate and SpO2 after BUFFER_SIZE samples (ST seconds of samples) using Robert's method
    rf_heart_rate_and_oxygen_saturation(aun_ir_buffer, BUFFER_SIZE, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid, &ratio, &correl); 

    setDisplay(n_heart_rate, n_spo2, "measuring vitals...");
    #ifdef DEBUG
      Serial.println("--RF--");
      Serial.print("\tSpO2: ");
      Serial.print(n_spo2);
      Serial.print("\tHR: ");
      Serial.print(n_heart_rate, DEC);
      Serial.print("\t");
      Serial.println(hr_str);
      Serial.println("------");
    #endif
  }else{
    digitalWrite(readLED, HIGH);
    isFingerPlaced = false;
    Serial.println("No finger");
    setDisplay(-999, -999, "Place your finger on sensor and wait..");
  }
}


