#ifndef IMURESULT_H_
#define IMURESULT_H_

#include <SPI.h>

class IMUResult
{

	public:
		IMUResult() { result[0]=0; result[1] = 0; result[2] = 0; }
		void setResult(float x, float y, float z) { result[0] = x; result[1] = y; result[2] = z; }
		float * getResult() { return &this->result[0]; }
		virtual void printResult() = 0;

	protected:
		float result[3];

};





class MagResult : public IMUResult
{
	public:
		void printResult();

};


class AccResult : public IMUResult
{
	public:
		void printResult();

};

class GyroResult : public IMUResult
{
	public:
		void printResult();

};

class OrientResult : public IMUResult
{
	public:
		void printResult();

};

#endif
