#include "IMUWriter.h"
#include <ArduinoJson.h>

IMUWriter::IMUWriter(eeprom_size_t devCap, byte nDev, unsigned int pgSize, byte busAddr) : extEEPROM(devCap, nDev, pgSize, busAddr) 
{ 
	nResults = 0; 
	position=0; 
	readPosition=0; 
	maxStoreableResults=devCap*1000/sizeof(IMUResult); 
}


void IMUWriter::writeResult(IMUResult result)
{
	this->writeAnything(this->position, result);
	this->nResults++;
	this->position+=sizeof(result);
}

void IMUWriter::next()
{
	this->nResults--;
	readPosition+=sizeof(IMUResult);
}

void IMUWriter::previous()
{
	this->nResults++;
	readPosition-=sizeof(IMUResult);
}

void IMUWriter::printStorage()
{
	Serial.print("Storage: ");
	Serial.print(this->nResults);
	Serial.print("/");
	Serial.println(this->maxStoreableResults);
}


int IMUWriter::jsonifyNextResult(String& buf, unsigned long absTimeInSeconds, unsigned long relativeTimeInMillis)
{
 
	if(readPosition>=position)
		return 0;
 
	StaticJsonBuffer<max_json_payload_size> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();


	unsigned long time;
	unsigned long offSet;
	int offSetFractionalPart = 0;
	float data[3];
	String timeBuf;
	String xName, yName, zName, name;
	IMUResult result;

	this->readAnything(readPosition, result);

	result.getResult(data);

	offSet = relativeTimeInMillis - result.getMillis();
	offSetFractionalPart = offSet % 1000; //This is because the last 3 digits of offSet are discared on the next line when we divide by 1000 to convert from milliseconds to seconds
	offSet = offSet / 1000.;

	time = absTimeInSeconds - offSet;
	timeBuf = String(time);
	timeBuf = timeBuf + String(offSetFractionalPart);

	result.getName(name);
	xName = "x-" + name;
	yName = "y-" + name;
	zName = "z-" + name;


	root.set<String>("timestamp", timeBuf);
	root.set<float>(xName, data[0]);
	root.set<float>(yName, data[1]);
	root.set<float>(zName, data[2]);

	root.printTo(buf);

	return this->nResults;

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
