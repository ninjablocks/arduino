#ifndef NinjaPi_h
#define NinjaPi_h
/*
const prog_char PROGMEM RETRIEVING_NAME[] ="Retrieving name\n";
const prog_char PROGMEM ERROR_RETRIEVING_NAME[] ="Error retrieving name\n";
const prog_char PROGMEM SUCCESSFULLY_RETRIEVED_NAME[] ="Successfully retrieved Name:";
const prog_char PROGMEM PARSING_OBJECT[] ="Parsing String\n";
const prog_char PROGMEM ERROR_PARSING_OBJECT[] ="Error parsing Object\n";
const prog_char PROGMEM SUCCESSFULLY_PARSED_OBJECT[] ="Successfully parsed Object\n";
const prog_char PROGMEM DELETING_OBJECT_STRING[] = "Deleting the object\n";
const prog_char PROGMEM FORMAT_FAILED_STRING[] = "Failed to create Format Object\n";
const prog_char PROGMEM OUTPUT_STRING_ERROR[] = "Error creating output String\n";
const prog_char PROGMEM RESULT_PRINTING_STRING[] = "Printing the result:\n";
const prog_char PROGMEM ADDING_FRAMERATE_STRING[] = "Adding frame rate to the format\n";
const prog_char PROGMEM ADDING_INTERLACE_STRING[] = "Adding interlace to the format\n";
const prog_char PROGMEM ADDING_HEIGHT_STRING[] = "Adding height to the format\n";
const prog_char PROGMEM ADDING_WIDTH_STRING[] = "Adding width to the format\n";
const prog_char PROGMEM ADDING_TYPE_STRING[] = "Adding type to the format\n";
const prog_char PROGMEM ADDING_FORMAT_STRING[] = "Adding format to the object\n";
const prog_char PROGMEM ADDING_LENGTH_STRING[] = "Adding length to the object\n";
const prog_char PROGMEM CREATING_FROMAT_STRING[] = "Creating format object\n";
const prog_char PROGMEM ADDING_NAME_STRING[] = "Adding name to the object\n";
const prog_char PROGMEM OBJECT_CREATION_FAILED_STRING[] = "Failed to create the object\n";
const prog_char PROGMEM OBJECT_CREATE_STRING[] = "Created a Object\n";
const prog_char PROGMEM HELLO_STRING[] = "********************\nTesting aJson\n*****************\n";
*/

#define recvLEN 	128			// this is really bad, will need to work out dynamic length
#define GUID_LEN	36
#define DATA_LEN	64
#define SLOW_DEVICE_HEARTBEAT	30000		// 30 seconds

typedef struct Device Device;
struct Device {
	char * strGUID;
	const int intVID;
	const int intDID;
	char * strDATA;
	const int strDATALenMax;
	double intDATA;
	const bool IsString;
	void (*didUpdate)(Device*);
};

class NinjaPi {
	private:
		unsigned long _lastHeartbeat;
		Device ** customDevices;
	public:
		NinjaPi ();
		void connectDevices(Device * customDevices[]);
		void sendObjects();
		void doReactors();
		boolean decodeJSON();
		void doOnBoardTemp();
		void doOnBoardAccelerometer();
		void doOnBoardRGB();
		void doJSONResponse();
		void doJSONError(int errorCode);
		void doJSONData(const char * strGUID, int intVID, int intDID
		 , const char * strDATA, double numDATA, bool IsString
		 , byte dataTYPE);
		boolean doPort1(byte* port);
		boolean doPort2(byte* port);
		boolean doPort3(byte* port);
		void doDHT22(byte port);
		void blinkLED(byte ledPin);
		void do433(void);
		void doLacrosseTX3(unsigned long long tx3value);
		void doLacrosseWS2355(unsigned long long ws2344value);
		void doWT450(unsigned long long value);
		//send device to cloud
		inline void doDevice(Device *d) {
			//doJSONData(char * strGUID, int intVID, int intDID
			//	, char * strDATA, double numDATA, bool IsString
			//  , byte dataTYPE)
			doJSONData(d->strGUID, d->intVID, d->intDID
				, d->strDATA, d->intDATA, d->IsString
				, DATATYPE_DEVICE	 //always DEVICE
			);
		}
		void doCustomDevices();
		inline bool checkDeviceForUpdate(Device *d);
		bool checkDevicesForUpdate();
};

extern NinjaPi nOBJECTS;
#endif
