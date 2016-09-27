#ifndef IMUWRITER_H_
#define IMUWRITER_H_


#include "extEEPROM.h"
#include "IMUResult.h"


//Writes IMUResults to EEPROM
class IMUWriter : public extEEPROM
{

	public:
		IMUWriter(eeprom_size_t devCap, byte nDev, unsigned int pgSize, byte busAddr); 
		void writeResult(IMUResult result);
		void readResults();  //Reads all values currently in EEPROM
		void printStorage(); //prints how much space is left in eeprom
		int getNumResults() { return nResults; }
		template <typename T> void getNextResult(T& value);
		template <typename T> void peekNextResult(T& value);
		void rollBack(int numStepsBackwards);
		void printDump();
		int getMaxStoreableResults() { return this->maxStoreableResults; }
	protected:
		void previous();  //moves read position back
		void next(); //advances read position
		int nResults; //Number of results currently stored on eeprom
		//The templated functions below are provided courtesy of http://playground.arduino.cc/Code/EEPROMWriteAnything
		template <class T> int writeAnything(int ee, const T& value);
		template <class T> int readAnything(int ee, T& value);
		int readPosition; //This is used to read through EEPROM when sending results to server.
		int writePosition; //Which byte we are at on eeprom.  This is used for writing to EEPROM.
		int maxStoreableResults;
};

template <typename T> void IMUWriter::peekNextResult(T& value)
{
	this->readAnything( this->readPosition, value);
}

template <typename T> void IMUWriter::getNextResult(T& value)
{
	this->readAnything( this->readPosition, value);
	this->next();
}


template <class T> int IMUWriter::writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;

    for (i = 0; i < sizeof(value); i++)
          this->write(ee++, *p++);
    return i;
}

template <class T> int IMUWriter::readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;

    for (i = 0; i < sizeof(value); i++)
          *p++ = this->read(ee++);

    return i;
}


#endif
