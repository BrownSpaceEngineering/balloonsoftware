/*
 * Final balloon sketch.
 * Handles OSD shield, SD shield, and temperature/altitude sensor
 * 
 * Created: 3/21/2017
 * Last modified: 10/14/2018
 * 
 * Author: Nishanth J. Kumar (NishanthJKumar)
 * Based on code by:
 * David Schurman (dschurman)
 */

// Included Libraries
  #include <SPI.h> // Serial Peripheral Interface, used to communicate with shields
  #include <SD.h> // SD card library
  #include <Adafruit_Sensor.h>
  #include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C


// Global variables 
// These define serial communication between Arduino and shields
 // const byte osdChipSelect = 10; // chip select pin for OSD
  const byte masterOutSlaveIn = MOSI;
  const byte masterInSlaveOut = MISO;
  const byte slaveClock = SCK;
  const int chipSelect = 4; // chip select pin for SD card

//  SFE_BMP180 pressure;
//  double baseline; // baseline pressure
  unsigned long curr_time;

  boolean deployed = false; // Indicating the panel isn't deployed yet
  boolean printed = false; // Indicating whether it's been printed that the deploy has happened


// Hardware Setup
  void setup() 
  {

    Serial.begin(9600); // Begin serial communication (9600 baud)

    bool status;
    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin();  

    Serial.println("Program started");

    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);

    // sets the digital pin 10 as output. This will be used to trigger the MOSFET
    pinMode(9, OUTPUT);
    
    digitalWrite(9, LOW); //set pin originally to low so flaps are originally not deployed
    analogWrite(9, 0);
    
    Serial.println("REBOOT");
    if (status) // initialize sensor
    Serial.println("BME280 init success");
    else
    {
      Serial.println("BME280 init fail (disconnected?)\n\n");
      while(1); // Pause forever.
    }
    
    Serial.print("Initializing SD card...");
    // see if the card is present and can be initialized:
    if (!SD.begin(4)) {
      Serial.println("Card failed, or not present");
      // don't do anything more
    }
    Serial.println("card initialized.");
    // Initialize the SPI connection:
    SPI.begin();
    SPI.setClockDivider( SPI_CLOCK_DIV2 );      // Must be less than 10MHz.  
  }

// Main Code
  void loop() 
  {
    curr_time = millis(); // tine value in millis
    // Serial.println(freeMemory()); //use to check for memory leaks
    String dataString = ""; // for writing to SD
    double a,P,T;
    P = bme.readPressure() / 100.0F;
    T = bme.readTemperature();
    int solarMilliVolts1 = analogRead(A0) * (5000 / 1023);
    int solarMilliVolts2 = analogRead(A1) * (5000 / 1023);
    a = bme.readAltitude(SEALEVELPRESSURE_HPA); // altitude in meters
   
    dataString += a;
    dataString += ",";
    dataString += T;
    dataString += ",";
    dataString += solarMilliVolts1;
    dataString += ",";
    dataString += solarMilliVolts2;

      if(((a > 10000) || (curr_time > 3333000)) && !deployed) {
      int i = 100;
      while(i < 255) {
        analogWrite(9, i);
        delay(50); 
        i += 50;         
      }
      delay(1000);
      i = 0;
      analogWrite(9, i); 
      deployed = true;
      Serial.println("!!!!!! DEPLOY JUST HAPPENED !!!!!!!!!!!!!!!");
      }

    File dataFile;
    char filename[] = "datalog.csv";
    dataFile = SD.open(filename, FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      if(deployed && !printed) {
      dataFile.println("SOLAR DEPLOYED APPROX 1 SECOND EARLIER");
      Serial.println("SOLAR DEPLOYED APPROX 1 SECOND EARLIER");
      printed = true;
      }
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, display error message:
    else {
      if(deployed && !printed) {
      Serial.println("SOLAR DEPLOYED APPROX 1 SECOND EARLIER");
      printed = true;
      }
      Serial.println(dataString);
      Serial.println("txt err");
     }
     }
