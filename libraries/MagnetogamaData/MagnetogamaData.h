#ifndef MagnetogamaData_h
#define MagnetogamaData_h

#include "Arduino.h"
//#include "SdFat.h"

/*class Orientation
{
public:
	Orientation();
	void update(float y, float p, float r);
	float yaw;
	float pitch;
	float roll;

};*/
class BComponent
{
public:
	BComponent();
	void update(float bx, float by, float bz);
	void newAverage(float bx, float by, float bz, int ncount);
	float X;
	float Y;
	float Z;
};

/*class PolarComponent
{
public:
	PolarComponent();
	void calculate(BComponent bcomp);
	float R;
	float D;
	float I;
	float H;

};*/

class Location
{
public:
	Location();
	void update(float lat, float lon, float elev);
	float latitude;
	float longitude;
	float elevation;
};

class DateTime
{
public:
	DateTime();
	void update(byte _day, byte _month, int _year, byte _hour, byte _min, byte _sec);
	byte day;
	byte month;
	int year;
	byte hour;
	byte minute;
	byte second;
};

struct BinaryPacket
{
	float Xmag;
	float Ymag;
	float Zmag;
	float Xloc;
	float Yloc;
	float Zloc;
	byte day;
	byte month;
	int year;
	byte hour;
	byte minute;
	byte second;
};

class MagnetogamaData
{
public:
	MagnetogamaData();
	void sendBinOverSerial(HardwareSerial& serial);
	void sendASCIIOverSerial(HardwareSerial& serial);
	//Orientation orientation;
	BComponent Bcomp;
	//PolarComponent Pcomp;
	Location location;
	DateTime datetime;
private:
	BinaryPacket pack2BinaryStruct();
};

#endif