#ifndef NinjaDeviceIds_h
#define NinjaDeviceIds_h

#define kNBDIDOnBoardStatusLED      999
#define V13

#ifdef V11
#define kNBDIDOnBoardRGBLED         1000
#elif V12
#define kNBDIDOnBoardRGBLED         1007
#endif

#endif
