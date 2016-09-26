#include "IMUWriter.h"
#include <ArduinoJson.h>

IMUWriter::IMUWriter(eeprom_size_t devCap, byte nDev, unsigned int pgSize, byte busAddr) : extEEPROM(devCap, nDev, pgSize, busAddr) 
{ 
	nResults = 0; 
	writePosition=0; 
	readPosition=0; 
	maxStoreableResults=devCap*1000/sizeof(IMUResult); 
}

void IMUWriter::rollBack(int numStepsBackwards)
{
	//If we fail to send then rollback the pointer to the current eeprom position
	for(int i = 0; i<numStepsBackwards; i++)
	{
		this->previous();
	}
}

void IMUWriter::writeResult(IMUResult result)
{
	this->writeAnything(this->writePosition, result);
	this->writePosition+=sizeof(result);

	//If we just reached end of EEPROM don't increment nResults.  Keep it at the max value.
	if( this->writePosition == this->maxStoreableResults * sizeof(IMUResult) )
		this->nResults = this->maxStoreableResults;
	else
		this->nResults++;


	//We loop to beginning of eeprom when we reach end (this should only happen if you go a long time without wifi)
	this->writePosition = this->writePosition % ( this->maxStoreableResults * sizeof(IMUResult) );

}

void IMUWriter::next()
{
	this->nResults--;
	readPosition+=sizeof(IMUResult);

	//We loop to beginning of eeprom when we reach end (this should only happen if you go a long time without wifi)
	this->readPosition = this->readPosition % ( this->maxStoreableResults * sizeof(IMUResult) );
}

void IMUWriter::previous()
{
	if( this->nResults >= 1 )
	{
		this->nResults++;
		readPosition-=sizeof(IMUResult);
	}
}

void IMUWriter::printStorage()
{
	Serial.print("Storage: ");
	Serial.print(this->nResults);
	Serial.print("/");
	Serial.println(this->maxStoreableResults);
}

void IMUWriter::readResults()
{
	Serial.println("----------------------START OF EEPROM----------------------------------");
	int tempPosition = 0;

	IMUResult result;

	while(tempPosition<writePosition)
	{
		Serial.print(tempPosition);
		this->readAnything(tempPosition, result);
		tempPosition+=sizeof(result);

		Serial.print(": (");
		Serial.print(result.getXComponent(), 2);
		Serial.print(", ");
		Serial.print(result.getYComponent(), 2);
		Serial.print(", ");
		Serial.print(result.getZComponent(), 2);
		Serial.println(")");
	}
	Serial.println("----------------------END OF EEPROM----------------------------------");
}
