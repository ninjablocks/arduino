/*
  MMA8453Q.cpp - MMA8453Q Accelerometer library
  
  Based on the MMA8453Q example by Kerry D Wong from http://www.kerrywong.com/2012/01/09/interfacing-mma8453q-with-arduino/ 
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


	Version 0.2: 5th May 2012 by JP Liew 
		- moved to MMA8453Q library
		
	Version 0.1: by Marcus Schappi www.ninjablocks.com

*/

#include <Arduino.h>
#include "MMA8453Q.h"
#include <I2C.h>

MMA8453Q::MMA8453Q ()
{
	mmaInit();
}
	

/*
  Read register content into buffer.
  The default count is 1 byte.
 
  The buffer needs to be pre-allocated
  if count > 1
*/
void MMA8453Q::regRead(byte reg, byte *buf, byte count )
{
  I2c.write(I2C_ADDR, reg);
  I2c.read(I2C_ADDR, reg, count);
 
  for (int i = 0; i < count; i++)
    *(buf+i) = I2c.receive();
}
 
/*
  Write a byte value into a register
*/
void MMA8453Q::regWrite(byte reg, byte val)
{
  I2c.write(I2C_ADDR, reg, val);
}
 
/*
  Put MMA8453Q into standby mode
*/
void MMA8453Q::standbyMode()
{
  byte reg;
  byte activeMask = 0x01;
 
  regRead(REG_CTRL_REG1, &reg);
  regWrite(REG_CTRL_REG1, reg & ~activeMask);
}
 
/*
  Put MMA8453Q into active mode
*/
void MMA8453Q::activeMode()
{
  byte reg;
  byte activeMask = 0x01;
 
  regRead(REG_CTRL_REG1, &reg);
  regWrite(REG_CTRL_REG1, reg | activeMask);
}
 
/*
  Use fast mode (low resolution mode)
  The acceleration readings will be
  limited to 8 bits in this mode.
*/
void MMA8453Q::lowResMode()
{
  byte reg;
  byte fastModeMask = 0x02;
 
  regRead(REG_CTRL_REG1, &reg);
  regWrite(REG_CTRL_REG1, reg | fastModeMask);
}
 
/*
  Use default mode (high resolution mode)
  The acceleration readings will be
  10 bits in this mode.
*/
void MMA8453Q::hiResMode()
{
  byte reg;
  byte fastModeMask = 0x02;
 
  regRead(REG_CTRL_REG1, &reg);
  regWrite(REG_CTRL_REG1,  reg & ~fastModeMask);
}
 
/*
  Get accelerometer readings (x, y, z)
  by default, standard 10 bits mode is used.
 
  This function also convers 2's complement number to
  signed integer result.
 
  If accelerometer is initialized to use low res mode,
  isHighRes must be passed in as false.
*/
void MMA8453Q::getAccXYZ(int *x, int *y, int *z, bool isHighRes)
{
  byte buf[6];
 
  if (isHighRes) {
    regRead(REG_OUT_X_MSB, buf, 6);
    *x = buf[0] << 2 | buf[1] >> 6 & 0x3;
    *y = buf[2] << 2 | buf[3] >> 6 & 0x3;
    *z = buf[4] << 2 | buf[5] >> 6 & 0x3;
  }
  else {
    regRead(REG_OUT_X_MSB, buf, 3);
    *x = buf[0] << 2;
    *y = buf[1] << 2;
    *z = buf[2] << 2;
  }
 
  if (*x > 511) *x = *x - 1024;
  if (*y > 511) *y = *y - 1024 ;
  if (*z > 511) *z = *z - 1024;
}

void MMA8453Q::mmaInit()
{
	byte b;
	I2c.begin();
	standbyMode(); //register settings must be made in standby mode
  regWrite(REG_XYZ_DATA_CFG, FULL_SCALE_RANGE_2g);
  hiResMode(); //this is the default setting and can be omitted.
  activeMode();
  regRead(REG_WHO_AM_I, &b,1);
}

MMA8453Q MMA;