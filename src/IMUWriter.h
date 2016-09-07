#ifndef IMUWRITER_H_
#define IMUWRITER_H_


#include "extEEPROM.h"
#include "IMUResult.h"

#define max_json_payload_size 1024

//Writes IMUResults to EEPROM
class IMUWriter : public extEEPROM
{

	public:
		IMUWriter(eeprom_size_t devCap, byte nDev, unsigned int pgSize, byte busAddr) : extEEPROM(devCap, nDev, pgSize, busAddr) { nResults = 0; position=0; readPosition=0; }
		void writeResult(IMUResult result);
		void readResults();
		void setPosition(int pos) { readPosition=pos; }
		int jsonifyNextResult(String& buf, unsigned long absTimeInSeconds, unsigned long relativeTimeInMillis);
	private:
		int nResults; //Number of results currently stored on eeprom
		int position; //Which byte we are at on eeprom.  This is used for writing to EEPROM.
		//The templated functions below are provided courtesy of http://playground.arduino.cc/Code/EEPROMWriteAnything
		template <class T> int writeAnything(int ee, const T& value);
		template <class T> int readAnything(int ee, T& value);
		int readPosition; //This is used to read through EEPROM when sending results to server.

};


#endif
