#ifndef NinjaLightObjects_h
#define NinjaLightObjects_h

#define recvLEN 	128			// this is really bad, will need to work out dynamic length
#define GUID_LEN	36
#define DATA_LEN	64


class NinjaLightObjects {
	private:

		
	public:
		char serInStr[recvLEN];  // array to hold the incoming serial string bytes
		char strGUID[GUID_LEN];
		int intVID;
		int intDID;
		int intDATA;
		int userVID;
		char strDATA[DATA_LEN];
		boolean IsDATAString;

		NinjaLightObjects ();
		//void sendObjects();
		boolean doReactors();
		boolean decodeJSON();
		void doJSONResponse();
		void doJSONError(int errorCode);
		void doJSONData(char * strGUID, int intVID, int intDID, char * strDATA, double numDATA, bool IsString, byte dataTYPE);
		int readSerialString ();
};

extern NinjaLightObjects lOBJECTS;
#endif
