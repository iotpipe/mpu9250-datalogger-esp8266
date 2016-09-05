#include "IMUWriter.h"



void IMUWriter::writeResult(IMUResult result)
{
	Serial.print("To position ");
	Serial.println(this->position);
	this->writeAnything(this->position, result);
	this->nResults++;
	this->position+=sizeof(result);	
}

void IMUWriter::readResults()
{
	Serial.println("----------------------START OF EEPROM----------------------------------");
	int tempPosition = 0;

	IMUResult result;

	while(tempPosition<position)
	{
		Serial.print("From position ");
		Serial.println(tempPosition);
		this->readAnything(tempPosition, result);
		tempPosition+=sizeof(result);
		result.printResult();
	}
	Serial.println("----------------------END OF EEPROM----------------------------------");
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
