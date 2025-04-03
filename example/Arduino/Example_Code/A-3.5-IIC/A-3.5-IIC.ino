#include <Arduino.h>

//#include <TFT_eSPI.h>

#include <Wire.h>
#include "DHT20.h"



DHT20 DHT(&Wire);

#define I2C0_SDA 20
#define I2C0_SCL 21


void setup()
{
 
  Serial.begin( 115200 );

  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire.begin();
  DHT.begin(); 

  

 

  

  delay(100);

}

void loop()
{

  char DHT_buffer[6];

  int status = DHT.read();
  
  int a = (int)DHT.getTemperature();
  int b = (int)DHT.getHumidity();
  Serial.print("Temperature:");
  Serial.println(a);
  Serial.print("getHumidity:");
  Serial.println(b);

  delay( 500 );

}


