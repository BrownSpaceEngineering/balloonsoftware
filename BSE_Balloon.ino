
/*
 * Final balloon sketch.
 * Handles OSD shield, SD shield, and temperature/altitude sensor
 * 
 * Created: 3/21/2017
 * Last modified: 10/19/2017
 * 
 * Author: Nishanth Kumar (NishanthJKumar)
 * Based on code by:
 * David Schurman (dschurman)
 */

// Included Libraries
  //#include <MemoryFree.h> // used to test for memory leaks
  #include <SPI.h> // Serial Peripheral Interface, used to communicate with shields
//  #include <MAX7456.h> // On-Screen-Display (OSD) shield library
  #include <SFE_BMP180.h> // Altitide/Temperature sensor library
  #include <SD.h> // SD card library

// Global variables 
// These define serial communication between Arduino and shields
 // const byte osdChipSelect = 10; // chip select pin for OSD
  const byte masterOutSlaveIn = MOSI;
  const byte masterInSlaveOut = MISO;
  const byte slaveClock = SCK;
  const byte osdReset = 0;
  const int chipSelect = 4; // chip select pin for SD card

  SFE_BMP180 pressure;
  double baseline; // baseline pressure

 // MAX7456 OSD(osdChipSelect);

// Hardware Setup
  void setup() 
  {
//    unsigned char system_video_in=NULL;  

    pinMode(8, OUTPUT);          // sets the digital pin 13 as output

    digitalWrite(8, LOW);

    Serial.begin(9600); // Begin serial communication (9600 baud)
    
    Serial.println("REBOOT");
    if (pressure.begin()) // initialize sensor
    Serial.println("BMP180 init success");
    else
    {
      Serial.println("BMP180 init fail (disconnected?)\n\n");
      while(1); // Pause forever.
    }
    
    Serial.print("Initializing SD card...");
    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
      // don't do anything more
    }
    Serial.println("card initialized.");
    // Initialize the SPI connection:
    SPI.begin();
    SPI.setClockDivider( SPI_CLOCK_DIV2 );      // Must be less than 10MHz.
    baseline = getPressure();
    // Initialize the MAX7456 OSD:
  //  OSD.begin();
  //  OSD.setSwitchingTime(5);     // Use NTSC with default area.
    
  //  system_video_in = OSD.videoSystem(); 
  //  if(NULL != system_video_in)
  //  {
  //      OSD.setDefaultSystem(system_video_in) ;
  //  }
  //  else
  //  {
  //      OSD.setDefaultSystem(MAX7456_NTSC) ;
  //  }
    
  //  OSD.display();  // Activate the text display.
    
  }

// Main Code
  void loop() 
  {
    // Serial.println(freeMemory()); //use to check for memory leaks
    String dataString = ""; // for writing to SD
    double a,P,T;
    P = getPressure();
    T = getTemp();
    baseline = 1013.25; // baseline pressure for sea level, in millibars
    a = pressure.altitude(P,baseline); // altitude in meters
    
    dataString += a;
    dataString += " ";
    dataString += T;
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, display error message:
    else {
      Serial.println(dataString);
      Serial.println("txt err");
    }  

    // OSD formatting
//    OSD.setCursor(0,-1);
//    OSD.print("Alt: ");
//    OSD.setCursor(10,-1);
//    OSD.print(a);
//    OSD.setCursor(0,-2);
//    OSD.print("Temp: ");
//    OSD.setCursor(10,-2);
//    OSD.print(T);
//    delay(1000); // delay for 1 second
  } 

// Helper functions
double getTemp()
{
  char status;
  double T; 
 
  status = pressure.startTemperature();
    // Wait for the measurement to complete:
  delay(status);
  status = pressure.getTemperature(T);
  T = T*9/5 + 32; // convert to Fahrenheit
  return(T);
}

double getPressure()
{
  char status;
  double T, P, p0, a;

  // Must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}
