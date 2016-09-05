#include "IMUResult.h"

IMUResult::IMUResult()
{
	result[0]=0; 
	result[1] = 0; 
	result[2] = 0; 

	for(int i = 0; i < 16; i++)
		resultName[i]='\0';

}

//pass in null padded string
void IMUResult::setName(char *name)
{
	int len = strlen(name);
	if(len>15)
	{
		Serial.println("Couldn't assign name.  Too long.  Must not exceed 15 characters. Must also be null-terminated.");
		return;
	}

	for(int i = 0; i < len; i++)
		resultName[i] = name[i];

}

void IMUResult::printResult()
{
  Serial.print(this->resultName);
  Serial.print(": ");
  Serial.print(this->result[0], 2);
  Serial.print(", ");
  Serial.print(this->result[1], 2);
  Serial.print(", ");
  Serial.println(this->result[2], 2);
}
