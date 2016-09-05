/*
  This Code is nearly 100% taken from Sparkfuns MPU9250 library, written by Kris Winer.  Some changes have been made to Kris' library in order to make certain actions more convenient and more opaque to the user.  Thanks Kris for your hard work.

  Hardware setup:
  MPU9250 Breakout --------- Adafruit Huzzah
  VDD ---------------------- 3.3V
  VDDI --------------------- 3.3V
  SDA ----------------------- Pin4
  SCL ----------------------- Pin5
  GND ---------------------- GND

  EEPROM
  Vcc to 3.3V
  GND to GND
  AO, A1, A2 to GND  (on 24LC256 this gives an i2c slave address as 1010000 which is 0x50)
  SDA/SCL to Pin4 and Pin5 of Adafruit Huzzah, respectively
*/

#include "src/quaternionFilters.h"
#include "src/MPU9250.h"
#include "src/IMUResult.h"
#include "src/IMUWriter.h"


#define serialDebug true  // Set to true to get Serial output for debugging

#define baudRate 115200
#define samplingRateInMillis 1000
#define declination 15.93  //http://www.ngdc.noaa.gov/geomag-web/#declination . 

#define calibrateMagnetometer true

uint32_t lastSample = 0;

MPU9250 myIMU;
IMUWriter writer(kbits_256, 1, 64, 0x50);  //These are the arguments needed for extEEPROM library.  See their documentation at https://github.com/JChristensen/extEEPROM
IMUResult magResult, accResult, gyroResult, orientResult;

void setup()
{

  Serial.begin(baudRate);


  magResult.setName("magfield");
  accResult.setName("acceleration");
  gyroResult.setName("gyro");
  orientResult.setName("orientation");

  //Start IMU.  Assumes default SDA and SCL pins 4,5 respectively.
  myIMU.begin();


  //This tests communication between the accelerometer and the ESP8266.  Dont continue until we get a successful reading.
  //It is expected that the WHO_AM_I_MPU9250 register should return a value of 0x71.
  //If it fails to do so try the following:
  //1) Go to src/MPU9250.h and change the value of ADO from 0 to 1
  //2) Ensure your i2c lines are 3.3V and that you haven't mixed up SDA and SCL
  //3) Run an i2c scanner program (google it) and see what i2c address the MPU9250 is on.  Verify your value of ADO in src/MPU9250.h is correct.
  //4) Some models apparently expect a hex value of 0x73 and not 0x71.  If that is the case, either remove the below check or change the value fro 0x71 to 0x73.
  byte c;
  do
  {
    c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
    if (c != 0x71)
    {
      Serial.println("Failed to communicate with MPU9250");
      Serial.print("WHO_AM_I returned ");
      Serial.println(c, HEX);
      delay(500);
    }
  } while (c != 0x71);

  Serial.println("Successfully communicated with MPU9250");


  //Test to ensure readings are within specified % of factory values.  Not necessary to do for most use cases.
  myIMU.selfTest();

  // Calibrate gyro and accelerometers, load biases in bias registers
  myIMU.calibrate();

  // Initialize device for active mode read of acclerometer, gyroscope, and temperature
  myIMU.init();

  //You will be prompted on the serial output to move your device in figure 8s for a few seconds.  If you don't want to calibrate the magnetometer at each device bootup then do the calibration once, then manually enter those values moving forward.
  //This can be looked up here:
  if (calibrateMagnetometer)
  {
    //myIMU.magCalibrate();
    myIMU.setMagCalibrationManually(-166, 16, 663);    //Set manually with the results of magCalibrate() if you don't want to calibrate at each device bootup.
    //Note that values will change as seasons change and as you move around globe.  These values are for zip code 98103 in the fall.
  }

  Serial.println("Initialized for active data mode....");
}
int counter = 0;
void loop()
{
  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    myIMU.readAccelData(&accResult);
    myIMU.readGyroData(&gyroResult);
    myIMU.readMagData(&magResult);
  }

  // Must be called before updating quaternions!
  myIMU.updateTime();
  MahonyQuaternionUpdate(&accResult, &gyroResult, &magResult, myIMU.deltat);
  readOrientation(&orientResult, declination);

  if (millis() - lastSample > samplingRateInMillis)
  {
    lastSample = millis();
    if (serialDebug)
    {
      accResult.printResult();
      //gyroResult.printResult();
      //magResult.printResult();
      //orientResult.printResult();
    }




    //Now write values to EEPROM
    writer.writeResult(accResult);
    counter++;
    if (counter % 10 == 0 & counter!=0)
    {
      writer.readResults();
    }
    

    myIMU.sumCount = 0;
    myIMU.sum = 0;

  }
}
