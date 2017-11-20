#include "MagnetogamaData.h"

/*Orientation::Orientation()
{
	yaw = 0;
	roll = 0;
	pitch = 0;



void Orientation::update(float y, float p, float r)
{
	yaw = y;
	pitch = p;
	roll = r;
}*/

// BComponent Class Function
BComponent::BComponent()
{
	X = 0;
	Y = 0;
	Z = 0;
}
void BComponent::update(float bx, float by, float bz)
{
	X = bx;
	Y = by;
	Z = bz;
}

void BComponent::newAverage(float bx, float by, float bz, int ncount)
{
	X += (bx - X) / ncount;
	Y += (by - Y) / ncount;
	Z += (bz - Z) / ncount;
}

// End of BComponent class function


// PolarComponent Class Function
PolarComponent::PolarComponent()
{
	R = 0;
	H = 0;
	D = 0;
	I = 0;
}
void PolarComponent::calculate(BComponent bcomp)
{
	R = sqrt(square(bcomp.X) + square(bcomp.Y) + square(bcomp.Z));
	H = sqrt(square(bcomp.X) + square(bcomp.Z));
	D = atan(bcomp.Z / bcomp.X) * 4068 / 71;
	I = atan(bcomp.Y / H) * 4068 / 71;
}

// End of PolarComponent class function
*/
Location::Location()
{
	latitude = 0;
	longitude = 0;
	elevation = 0;
}
void Location::update(float lat, float lon, float elev)
{
	latitude = lat;
	longitude = lon;
	elevation = elev;
}

DateTime::DateTime()
{
	day 	= 0;
	month 	= 0;
	year 	= 0;
	hour 	= 0;
	minute 	= 0;
	second = 0;
}

void DateTime::update(byte _day, byte _month, int _year, byte _hour, byte _min, byte _sec)
{
	day = _day;
	month = _month;
	year = _year;
	hour = _hour;
	minute = _min;
	second = _sec;
}

MagnetogamaData::MagnetogamaData()
{

}

void MagnetogamaData::sendBinOverSerial(HardwareSerial& serial)
{
	BinaryPacket bp = pack2BinaryStruct();

	serial.print("$SBPT");
	serial.write((const uint8_t *) &bp, sizeof(bp));
	serial.print("$EBPT");
}

/*void MagnetogamaData::sendASCIIOverSerial(HardwareSerial& serial)
{
	ArduinoOutStream cout(serial);

	cout 	<< setw(10) << setprecision(3) 	<< Bcomp.X << ','
	        << setw(10) << setprecision(3) 	<< Bcomp.Y << ','
	        << setw(10) << setprecision(3) 	<< Bcomp.Z << ','
	        << setw(10) << setprecision(5) 	<< location.latitude << ','
	        << setw(10) << setprecision(5) 	<< location.longitude << ','
	        << setw(7) 	<< setprecision(2) 	<< location.elevation << ','
	        << setw(2) 	<< setfill('0') 	<< (int) datetime.day
	        << setw(2) 	<< setfill('0')		<< (int) datetime.month
	        << setw(4) 	<< setfill('0')		<< (int) datetime.year
	        << setw(2) 	<< setfill('0')		<< (int) datetime.hour
	        << setw(2) 	<< setfill('0')		<< (int) datetime.minute
	        << setw(2) 	<< setfill('0')		<< (int) datetime.second
	        << endl;
}*/

BinaryPacket MagnetogamaData::pack2BinaryStruct()
{
	BinaryPacket packet;
	packet.Xmag = Bcomp.X;
	packet.Ymag = Bcomp.Y;
	packet.Zmag = Bcomp.Z;
	packet.Xloc = location.longitude;
	packet.Yloc = location.latitude;
	packet.Zloc = location.elevation;
	packet.day = datetime.day;
	packet.month = datetime.month;
	packet.year = datetime.year;
	packet.hour = datetime.minute;
	packet.minute = datetime.minute;
	packet.second = datetime.second;

	return packet;

}
