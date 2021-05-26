
/*
  WriteSingleField
  
  Description: Writes a value to a channel on ThingSpeak every 20 seconds.
  
  Hardware: Arduino MKR GSM 1400
  
  !!! IMPORTANT - Modify the secrets.h file for this project with your network connection and ThingSpeak channel details. !!!
  
  Note:
  - Requires MKRGSM library.
  - Reqires GSM access (SIM card or credentials).
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize, and 
  analyze live data streams in the cloud. Visit https://www.thingspeak.com to sign up for a free account and create a channel.  
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the README.md folder where the library was installed.
  See https://www.mathworks.com/help/thingspeak/index.html for the full ThingSpeak documentation.
  
  For licensing information, see the accompanying license file.
  
  Copyright 2018, The MathWorks, Inc.
*/


#include "ThingSpeak.h"
#include <MKRGSM.h>
#include "arduino_secrets.h"

#include "Adafruit_SI1145.h"
#include <Wire.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#include "DHT.h"

#include <RTCZero.h>

#define DHTPIN 1     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino 
#define ONE_WIRE_BUS 2 

/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/ 
DHT dht(DHTPIN, DHTTYPE);
RTCZero rtc;

// PIN Number
const char PINNUMBER[]     = SECRET_PIN;
// APN data
const char GPRS_APN[]      = SECRET_GPRS_APN;
const char GPRS_LOGIN[]    = SECRET_GPRS_LOGIN;
const char GPRS_PASSWORD[] = SECRET_GPRS_PASS;
//const bool donepin = 1;      // the number of the data sent pin
//int val = 0;


Adafruit_SI1145 uv = Adafruit_SI1145();   //Instantiate the Adafruit si1145 Sensor

GSMClient client;
GPRS gprs;
GSM gsmAccess;



unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

int number = 0;

void setup() {
  
  rtc.begin();

  dht.begin();
  
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  
  Serial.begin(115200);  //Initialize serial
  delay(5000);
  //Serial.println("donepin set to value = " + donepin);
  
  //digitalWrite(donepin, HIGH);
//  val = donepin;
//  Serial.println(val);
//  Serial.println("Starting Arduino web client.");

  if (! uv.begin()) {
    Serial.println("Didn't find Si1145");
    while (1);
  }

  Serial.println("Starting Arduino web client.");
  boolean connected = false;

  // wait 10 seconds for connection:
  delay(5000);
  
  while (!connected) {
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
      (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(200);
    }
  }

  Serial.println("connected");
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak 
}

void loop() {
  
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.println(F("%"));
  Serial.println(F("Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  Serial.print(f);
  Serial.println(F("°F"));


  sensors.requestTemperatures(); // Send the command to get temperature readings 
  delay(2000);
  float temp1 = sensors.getTempCByIndex(0);
  Serial.println(temp1);

//  float temp2 = sensors.getTempCByIndex(1);
//  Serial.println(temp2);


  
  //Get IR, UV and Vis readings from SI1145 module
  Serial.print("Vis: "); Serial.println(uv.readVisible());
  Serial.print("IR: "); Serial.println(uv.readIR());
  float Vis = uv.readVisible();           //Vis IR and UVindex come from the SI1145 sensor
  float IR = uv.readIR();
  float UVindex = uv.readUV();
  UVindex /= 100.0;  
  Serial.print("UV: ");  
  Serial.println(UVindex);
  Serial.print("VIS: ");  
  Serial.println(Vis);
  Serial.print("IR: ");  
  Serial.println(IR);

//  // set the fields with the values
  ThingSpeak.setField(1, temp1);
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, t);
  ThingSpeak.setField(4, IR);
  ThingSpeak.setField(5, Vis);
  ThingSpeak.setField(6, UVindex);

  
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  //int x = ThingSpeak.writeField(myChannelNumber, 1, number, myWriteAPIKey);
  
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
    Serial.println("attempting to write donepin to low and waiting 1 second");
    delay(1000); //just to check, remove for final code
    digitalWrite(1, LOW);
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  // change the value
  number++;
  if(number > 99){
    number = 0;
  }
  rtc.standbyMode();
  //delay(20000); // Wait 20 seconds to update the channel again
}
