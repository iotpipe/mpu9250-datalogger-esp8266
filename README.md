# mpu9250-datalogger-esp8266
A data logger example using the MPU9250 accelerometer and a ESP8266 Wi-Fi Radio.  This example works best with IoT Pipe (www.iotpipe.io).

# What does it do
Our data logger records values from an MPU9250 Accelerometer onto an EEPROM.  Periodically it checks for an internet connection and if one is found the data contained on the EEPROM is uploaded to the IoT Pipe server.  Consider the following three cases for a better understanding:

## Case 1: Constant internet connection
With an always-on internet connection results are quickly sent to the IoT Pipe server.  The EEPROM is almost entirely ignored.

## Case 2: Intermittent internet connection
Results are stored on the EEPROM.  As an internet connection is found, the data on the EEPROM will be uploaded to the IoT Pipe server.  Error handling is performed for cases where connections are abruptly cut short so that no data is lost.

## Case 3: Almost never an internet connection
A typically EEPROM is 512kb and a single result is 32 bytes.  This allows for the storing of 16000 values on the EEPROM.  In the case where there is no internet for an extended period of time older sensor values will be overwritten for more recent ones in a first in first out principle.

# What could be improved
We most likely won't improve this library further but if we did we would do the following:
1. Make it more generic.  It is currently tied closely to the MPU9250.  Making it more generic for other sensors would be useful and not too hard to do.
2. Handle situations where device power is turned off or the device is reset.  Currently, if the device is powered off we lose all data stored on the EEPROM.  This is because we don't persist our state between power-ups.

# Special thanks
We want to especially thank Kris Winter for his awesome MPU9250 library.  You can find it here: https://github.com/kriswiner/MPU-9250

We'd additionally like to thank the authors of the extEEPROM, ArduinoJSON, and PubSubClient libraries for all that they do.

# A final caveat
This code is not perfect and there are surely errors.  Report them to us @iot_pipe on twitter.  
