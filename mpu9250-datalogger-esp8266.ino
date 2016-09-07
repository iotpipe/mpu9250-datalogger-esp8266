/*
  This Code is nearly 100% taken from Sparkfuns MPU9250 library, written by Kris Winer.  Some changes have been made to Kris' library in order to make certain actions more convenient and more opaque to the user.  Thanks Kris for your hard work.

  Required Libraries:
  1) IoT Pipe
  2) extEEPROM

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
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "src/quaternionFilters.h"
#include "src/MPU9250.h"
#include "src/IMUResult.h"
#include "src/IMUWriter.h"
#include "iotpipe.h"

// IoT Pipe Setup.  Go to www.iotpipe.io to create an account!  You can find a tutorial for the ESP8266 at https://iotpipe.io/esp8266
const char ssid[] = "CenturyLink0638";
const char password[] = "5nesxjf5kym5nd";
const char deviceId[] = "e1a211a6ab30c49";
const char mqtt_user[] = "58d654b5309fff7baca44eb667b5e80";
const char mqtt_pass[] = "d12d09d1a47dd8b6853da06189e0ebc";
WiFiClient espClient;
PubSubClient client(espClient);
IotPipe iotpipe(deviceId);

//Debug information
#define serialDebug true  // Set to true to get Serial output for debugging
#define baudRate 115200

#define samplingRateInMillis 1000


//Setup for the Accelerometer
#define declination 15.93  //http://www.ngdc.noaa.gov/geomag-web/#declination . This is the declinarion in the easterly direction in degrees.  
#define calibrateMagnetometer false  //Setting requires requires you to move device in figure 8 pattern when prompted over serial port.  Typically, you do this once, then manually provide the calibration values moving forward.
MPU9250 myIMU;
IMUWriter writer(kbits_256, 1, 64, 0x50);  //These are the arguments needed for extEEPROM library.  See their documentation at https://github.com/JChristensen/extEEPROM
IMUResult magResult, accResult, gyroResult, orientResult;
uint32_t lastSample = 0;



//Wifi status LED (Turns on when connected to Wi-Fi)
int wifiStatusPin = 13;

//Data Sent LED (Turns on when all data has been sent from EEPROM to Server)
int dataStatusPin = 14;

//Send Data Interrupt Pin (When brought low, device attempts to connect to Wi-Fi and send data.  Status is determined from our two status LEDs)
int sendDataPin = 12;


void setup()
{

  Serial.begin(baudRate);
  Serial.setDebugOutput(true);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA); 

 
  //We treat each measurable quantity of the accelerometer as a individual sensor with the name provided tothe setName() function
  magResult.setName("magfield");
  accResult.setName("acceleration");
  gyroResult.setName("gyro");
  orientResult.setName("orientation");

  //Start IMU.  Assumes default SDA and SCL pins 4,5 respectively.
  myIMU.begin();


  //This tests communication between the accelerometer and the ESP8266.  Dont continue until we get a successful reading.
  //It is expected that the WHO_AM_I_MPU9250 register should return a value of 0x71.
  //If it fails to do so try the following:
  //1) Turn power off to the ESP8266 and restart.  Try this a few times first.  It seems to resolve the issue most of the time.  If this fails, then proceed to the followingn steps.
  //2) Go to src/MPU9250.h and change the value of ADO from 0 to 1
  //3) Ensure your i2c lines are 3.3V and that you haven't mixed up SDA and SCL
  //4) Run an i2c scanner program (google it) and see what i2c address the MPU9250 is on.  Verify your value of ADO in src/MPU9250.h is correct.
  //5) Some models apparently expect a hex value of 0x73 and not 0x71.  If that is the case, either remove the below check or change the value fro 0x71 to 0x73.
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


  // Calibrate gyro and accelerometers, load biases in bias registers, then initialize MPU.
  myIMU.calibrate();  
  myIMU.init();
  if (calibrateMagnetometer)
    myIMU.magCalibrate();
  else
    myIMU.setMagCalibrationManually(-166, 16, 663);    //Set manually with the results of magCalibrate() if you don't want to calibrate at each device bootup.
                                                       //Note that values will change as seasons change and as you move around globe.  These values are for zip code 98103 in the fall.

  Serial.println("Initialized for active data mode....");
  
  //Setup LEDs and buttons
  pinMode(sendDataPin, INPUT_PULLUP);
  pinMode(wifiStatusPin, OUTPUT);
  digitalWrite(wifiStatusPin,LOW);
  pinMode(dataStatusPin, OUTPUT);
  digitalWrite(dataStatusPin,LOW);
  attachInterrupt(digitalPinToInterrupt(sendDataPin), readyToSendData, FALLING);
}

//This function flips a boolean when it is time to connect to wifi
bool readyToConnect = false;
void readyToSendData()
{
  detachInterrupt(digitalPinToInterrupt(sendDataPin));
  readyToConnect = true;
}

//This connects to wifi and iotpipe server
void connectToWifiAndSendData()
{
  
  bool success = false;
  if(WiFi.status()!=WL_CONNECTED)
  {
    success = connect_wifi();
  }
  else
  {
    success=true;
  }
  
  
  if(success)
  {
    digitalWrite(wifiStatusPin,HIGH);
    readyToConnect = false;
    client.setServer("broker.iotpipe.io",1883);      
    delay(10);
        

    //Wait until we make connection with NTP Server.  If we take more than 20 seconds to make connection, we stop and continue collecting data.
    IotPipe_SNTP timeGetter;    
    unsigned long absTimeInSeconds=0, timeOffsetInMillis=0;
    int counter = 0;
    while(counter<40)
    {
      Serial.println("Acquiring time from SNTP server.");
      Serial.print(".");
      absTimeInSeconds = (unsigned long)timeGetter.getEpochTimeInSeconds();
      if(absTimeInSeconds!=0)
      {
        timeOffsetInMillis=millis();        
        Serial.println("");
        delay(10);
        break;
      }
      delay(500);
      counter++;
    }

    if(counter==40)
    {
      Serial.println("Failed to connect to NTP Server.  Will continue collecting data.");
      return;
    }

    String buf="[\n\t"; 
    
    int nResults = 0, gp;
    do
    {
      nResults = writer.jsonifyNextResult(buf, absTimeInSeconds, timeOffsetInMillis);        
      
      
      if(buf.length()>2048 | nResults==0)
      {
        buf=buf+"\n]";
        
        if (!client.connected()) 
        {
          reconnect();    
        }
        //Publish
        Serial.println("publishing data to server.");                
        Serial.println(buf);
        String topic = iotpipe.get_sampling_topic();
        client.publish(topic.c_str(),buf.c_str(), buf.length());
        //Start buf over again.      
        buf="[\n\t";
        
      }           
      else
      {
        buf=buf+",\n\t";        
      }
    }while(nResults!=0);
        
  } 
}

void loop()
{
  if(readyToConnect==true)
  {
    connectToWifiAndSendData();
  }


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
    writer.writeResult(gyroResult);  
    writer.writeResult(magResult);  
    writer.writeResult(orientResult);  

    myIMU.sumCount = 0;
    myIMU.sum = 0;

  }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//              Beyond this point is functionality for Wi-Fi connectivity and MQTT messaging.  Don't touch unless you know what you are doing. :)
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long lastMsg = 0;
char msg[50];
int value = 0;

bool connect_wifi() 
{  
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin("CenturyLink0638", "5nesxjf5kym5nd");
  int counter=0;
  while (WiFi.status() != WL_CONNECTED & counter<20) {
    delay(500);
    Serial.print(".");
    counter++;
  }

  if(counter==20)
  {
    Serial.println("Couldn't connect to wifi.  Will continue collecting data.");
    return false;
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client",mqtt_user,mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

