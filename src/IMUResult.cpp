#include "IMUResult.h"

void AccResult::printResult()
{
  Serial.print("Acceleration (mg): ");
  Serial.print(this->result[0]*1000, 2);
  Serial.print(", ");
  Serial.print(this->result[1]*1000, 2);
  Serial.print(", ");
  Serial.println(this->result[2]*1000, 2);
}
void MagResult::printResult()
{
  Serial.print("Magnetic Field (milli-gauss): ");
  Serial.print(this->result[0], 2);
  Serial.print(", ");
  Serial.print(this->result[1], 2);
  Serial.print(", ");
  Serial.println(this->result[2], 2);
}
void GyroResult::printResult()
{
  Serial.print("Gyro: ");
  Serial.print(this->result[0], 2);
  Serial.print(", ");
  Serial.print(this->result[1], 2);
  Serial.print(", ");
  Serial.println(this->result[2], 2);
}
void OrientResult::printResult()
{
  Serial.print("Yaw, Pitch, Roll: ");
  Serial.print(this->result[0], 2);
  Serial.print(", ");
  Serial.print(this->result[1], 2);
  Serial.print(", ");
  Serial.println(this->result[2], 2);
}
