#ifndef Sensors_h
#define Sensors_h

class SENSORS
{
	private:
    unsigned long _lastReadTime;
    float _lastTemperature;
    		
	public:
		SENSORS();
		int idTheType(int sensorValue, bool debug);
		int getSensorValue(byte port, int type);
		float getBoardTemperature();
  
};

extern SENSORS Sensors;

#endif