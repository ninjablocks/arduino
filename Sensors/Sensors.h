#ifndef Sensors_h
#define Sensors_h

class SENSORS
{
	public:
		SENSORS();
		int idTheType(int sensorValue, bool debug);
		int getSensorValue(byte port, int type);
		float getBoardTemperature();
  
};

extern SENSORS Sensors;

#endif