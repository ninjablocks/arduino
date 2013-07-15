#ifndef NinjaDeviceIds_h
#define NinjaDeviceIds_h

#define kNBDIDOnBoardStatusLED      999

#ifdef V11
#define kNBDIDOnBoardRGBLED         1000
#endif
#if defined(V12) || defined(VRPI10)
#define kNBDIDOnBoardRGBLED         1007
#endif

#endif
