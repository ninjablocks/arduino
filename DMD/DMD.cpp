/*--------------------------------------------------------------------------------------

 DMD.cpp - Function and support library for the Freetronics DMD, a 512 LED matrix display
           panel arranged in a 32 x 16 layout.

 Copyright (C) 2011 Marc Alexander (info <at> freetronics <dot> com)

 Note that the DMD library uses the SPI port for the fastest, low overhead writing to the
 display. Keep an eye on conflicts if there are any other devices running from the same
 SPI port, and that the chip select on those devices is correctly set to be inactive
 when the DMD is being written to.

 ---

 This program is free software: you can redistribute it and/or modify it under the terms
 of the version 3 GNU General Public License as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program.
 If not, see <http://www.gnu.org/licenses/>.

--------------------------------------------------------------------------------------*/
#include "DMD.h"

/*--------------------------------------------------------------------------------------
 Setup and instantiation of DMD library
 Note this currently uses the SPI port for the fastest performance to the DMD, be
 careful of possible conflicts with other SPI port devices
--------------------------------------------------------------------------------------*/
DMD::DMD(byte panelsWide, byte panelsHigh, byte panelsBPP)
{
    DisplaysWide=panelsWide;
    DisplaysHigh=panelsHigh;
    DisplayMaxX=DisplaysWide*DMD_PIXELS_ACROSS;
    DisplayMaxY=DisplaysHigh*DMD_PIXELS_DOWN;
    DisplaysTotal=DisplaysWide*DisplaysHigh;
    DisplaysBPP=panelsBPP;
    row1 = DisplaysTotal<<4;
    row2 = DisplaysTotal<<5;
    row3 = ((DisplaysTotal<<2)*3)<<2;
    setupBuffer(1);

    // initialize the SPI port
    SPI.begin();		// probably don't need this since it inits the port pins only, which we do just below with the appropriate DMD interface setup
    SPI.setBitOrder(MSBFIRST);	//
    SPI.setDataMode(SPI_MODE0);	// CPOL=0, CPHA=0
    SPI.setClockDivider(SPI_CLOCK_DIV2);	// system clock / 2 = 8MHz SPI CLK to shift registers

    digitalWrite(PIN_DMD_A, LOW);	// 
    digitalWrite(PIN_DMD_B, LOW);	// 
    digitalWrite(PIN_DMD_CLK, LOW);	// 
    digitalWrite(PIN_DMD_SCLK, LOW);	// 
    digitalWrite(PIN_DMD_R_DATA, HIGH);	// 
    digitalWrite(PIN_DMD_nOE, LOW);	//

    pinMode(PIN_DMD_A, OUTPUT);	//
    pinMode(PIN_DMD_B, OUTPUT);	//
    pinMode(PIN_DMD_CLK, OUTPUT);	//
    pinMode(PIN_DMD_SCLK, OUTPUT);	//
    pinMode(PIN_DMD_R_DATA, OUTPUT);	//
    pinMode(PIN_DMD_nOE, OUTPUT);	//

    clearScreen(0);

    // init the scan line/ram pointer to the required start point
    bDMDByte = 0;
}

/*--------------------------------------------------------------------------------------
 Setup for buffers
--------------------------------------------------------------------------------------*/
void
 DMD::setupBuffer( byte buffers)
{
    if (bDMDScreenRAM != NULL) {
        for(byte i=0;i<DisplaysBPP;i++) {
            if (bDMDScreenRAM[i] != NULL) free(bDMDScreenRAM[i]);
        }
        free(bDMDScreenRAM);
    }
    DisplayBuf=0;
    EditBuf=0;
    bDMDScreenRAM = (byte **) malloc (buffers * DisplaysBPP * sizeof(byte *));
    for(byte i=0;i<(DisplaysBPP*buffers);i++) {
        bDMDScreenRAM[i] = (byte *) malloc(DisplaysTotal*DMD_RAM_SIZE_BYTES);
        memset(bDMDScreenRAM[i],0xFF,DMD_RAM_SIZE_BYTES*DisplaysTotal);
    }
}

/*--------------------------------------------------------------------------------------
 Set which buffer will be editted
--------------------------------------------------------------------------------------*/
void
 DMD::setBufferEdit(byte buffer)
{
    EditBuf=buffer;
}

/*--------------------------------------------------------------------------------------
 Set which buffer will be displayed
--------------------------------------------------------------------------------------*/
void
 DMD::setBufferDisplay (byte buffer)
{
    DisplayBuf=buffer;
}

/*--------------------------------------------------------------------------------------
 Set or clear a pixel at the x and y location (0,0 is the top left corner)
--------------------------------------------------------------------------------------*/
void
 DMD::writePixel(unsigned int bX, unsigned int bY, byte colour)
{
    unsigned int uiDMDRAMPointer;

    if (bX >= DisplayMaxX || bY >= DisplayMaxY) {
	    return;
    }
    byte panel=(bX/DMD_PIXELS_ACROSS) + (DisplaysWide*(bY/DMD_PIXELS_DOWN));
    bX=(bX % DMD_PIXELS_ACROSS) + (panel<<5);
    bY=bY % DMD_PIXELS_DOWN;
    //set pointer to DMD RAM byte to be modified
    uiDMDRAMPointer = bX/8 + bY*(DisplaysTotal<<2);

    byte lookup = bPixelLookupTable[bX & 0x07];

    if (colour > 0) {
        bDMDScreenRAM[0+(DisplaysBPP*EditBuf)][uiDMDRAMPointer] &= ~lookup;
    } else {
        bDMDScreenRAM[0+(DisplaysBPP*EditBuf)][uiDMDRAMPointer] |= lookup;
    }
    if (DisplaysBPP==1) return;
    if (colour > 1) {
        bDMDScreenRAM[1+(DisplaysBPP*EditBuf)][uiDMDRAMPointer] &= ~lookup;
    } else {
        bDMDScreenRAM[1+(DisplaysBPP*EditBuf)][uiDMDRAMPointer] |= lookup;
    }
    if (DisplaysBPP==2) return;
    if (colour > 2) {
        bDMDScreenRAM[2+(DisplaysBPP*EditBuf)][uiDMDRAMPointer] &= ~lookup;
    } else {
        bDMDScreenRAM[2+(DisplaysBPP*EditBuf)][uiDMDRAMPointer] |= lookup;
    }
}

void DMD::drawString(int bX, int bY, const char *bChars, byte length,
		     byte fgcolour, byte bgcolour)
{
    if (bX >= DisplayMaxX || bY >= DisplayMaxY) return;
    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);
    if (bY+height<0) return;

    int strWidth = 0;
	this->drawLine(bX -1 , bY, bX -1 , bY + height, bgcolour);

    for (byte i = 0; i < length; i++) {
        int charWide = this->drawChar(bX+strWidth, bY, bChars[i], fgcolour, bgcolour);
	    if (charWide > 0) {
	        strWidth += charWide ;
	        this->drawLine(bX + strWidth , bY, bX + strWidth , bY + height, bgcolour);
            strWidth++;
        } else if (charWide < 0) {
            return;
        }
        if ((bX + strWidth) >= DisplayMaxX || bY >= DisplayMaxY) return;
    }
}

void DMD::drawMarquee(const char *bChars, byte length, int left, int top, byte fgcolour, byte bgcolour)
{
    marqueeWidth = 0;
    for (byte i = 0; i < length; i++) {
	    marqueeText[i] = bChars[i];
	    marqueeWidth += charWidth(bChars[i]) + 1;
    }
    marqueeHeight=pgm_read_byte(this->Font + FONT_HEIGHT);
    marqueeText[length] = '\0';
    marqueeOffsetY = top;
    marqueeOffsetX = left;
    marqueeLength = length;
    marqueeColour = fgcolour;
    marqueeBG = bgcolour;
    drawString(marqueeOffsetX, marqueeOffsetY, marqueeText, marqueeLength,
	   marqueeColour, marqueeBG);
}

boolean DMD::stepMarquee(int amountX, int amountY)
{
    boolean ret=false;
    marqueeOffsetX += amountX;
    marqueeOffsetY += amountY;
    if (marqueeOffsetX < -marqueeWidth) {
	    marqueeOffsetX = DisplayMaxX;
	    clearScreen(marqueeBG);
        ret=true;
    } else if (marqueeOffsetX > DisplayMaxX) {
	    marqueeOffsetX = -marqueeWidth;
	    clearScreen(marqueeBG);
        ret=true;
    }
    
        
    if (marqueeOffsetY < -marqueeHeight) {
	    marqueeOffsetY = DMD_PIXELS_DOWN * DisplaysHigh;
	    clearScreen(marqueeBG);
        ret=true;
    } else if (marqueeOffsetY > DisplayMaxY) {
	    marqueeOffsetY = -marqueeHeight;
	    clearScreen(marqueeBG);
        ret=true;
    }

    // Special case horizontal scrolling to improve speed
    if (amountY==0 && amountX==-1) {
        this->scrollHorz(amountX,false);

        // Redraw last char on screen
        int strWidth=marqueeOffsetX;
        for (byte i=0; i < marqueeLength; i++) {
            byte wide = charWidth(marqueeText[i]);
            if (strWidth+wide >= DisplayMaxX) {
                drawChar(strWidth, marqueeOffsetY,marqueeText[i],marqueeColour, marqueeBG);
                return ret;
            }
            strWidth += wide+1;
        }
    } else if (amountY==0 && amountX==1) {
        this->scrollHorz(amountX,false);

        // Redraw first char on screen
        int strWidth=marqueeOffsetX;
        for (byte i=0; i < marqueeLength; i++) {
            byte wide = charWidth(marqueeText[i]);
            if (strWidth+wide >= 0) {
                drawChar(strWidth, marqueeOffsetY,marqueeText[i],marqueeColour, marqueeBG);
                return ret;
            }
            strWidth += wide+1;
        }
    } else {
        drawString(marqueeOffsetX, marqueeOffsetY, marqueeText, marqueeLength,
	       marqueeColour, marqueeBG);
    }

    return ret;
}


/*--------------------------------------------------------------------------------------
 Clear the screen in DMD RAM
--------------------------------------------------------------------------------------*/
void DMD::clearScreen(byte colour)
{
    for (byte col=0; col<DisplaysBPP;col++) {
        if (colour&(1<<col)) // clear all pixels
            memset(bDMDScreenRAM[col+(DisplaysBPP*EditBuf)],0x00,DMD_RAM_SIZE_BYTES*DisplaysTotal);
        else // set all pixels
            memset(bDMDScreenRAM[col+(DisplaysBPP*EditBuf)],0xFF,DMD_RAM_SIZE_BYTES*DisplaysTotal);
    }
}

/*--------------------------------------------------------------------------------------
 Draw or clear a line from x1,y1 to x2,y2
--------------------------------------------------------------------------------------*/
void DMD::drawLine(int x1, int y1, int x2, int y2, byte colour)
{
    int dy = y2 - y1;
    int dx = x2 - x1;
    int stepx, stepy;

    if (dy < 0) {
	    dy = -dy;
	    stepy = -1;
    } else {
	    stepy = 1;
    }
    if (dx < 0) {
	    dx = -dx;
	    stepx = -1;
    } else {
	    stepx = 1;
    }
    dy <<= 1;			// dy is now 2*dy
    dx <<= 1;			// dx is now 2*dx

    writePixel(x1, y1, colour);
    if (dx > dy) {
	    int fraction = dy - (dx >> 1);	// same as 2*dy - dx
	    while (x1 != x2) {
	        if (fraction >= 0) {
		        y1 += stepy;
		        fraction -= dx;	// same as fraction -= 2*dx
	        }
	        x1 += stepx;
	        fraction += dy;	// same as fraction -= 2*dy
	        writePixel(x1, y1, colour);
	    }
    } else {
	    int fraction = dx - (dy >> 1);
	    while (y1 != y2) {
	        if (fraction >= 0) {
		        x1 += stepx;
		        fraction -= dy;
	        }
	        y1 += stepy;
	        fraction += dx;
	        writePixel(x1, y1, colour);
	    }
    }
}

/*--------------------------------------------------------------------------------------
 Draw or clear a circle of radius r at x,y centre
--------------------------------------------------------------------------------------*/
void DMD::drawCircle(int xCenter, int yCenter, int radius,
		     byte colour)
{
    int x = 0;
    int y = radius;
    int p = (5 - radius * 4) / 4;

    drawCircleSub(xCenter, yCenter, x, y, colour);
    while (x < y) {
	    x++;
	    if (p < 0) {
	        p += 2 * x + 1;
	    } else {
	        y--;
	        p += 2 * (x - y) + 1;
	    }
	    drawCircleSub(xCenter, yCenter, x, y, colour);
    }
}

void DMD::drawCircleSub(int cx, int cy, int x, int y, byte colour)
{

    if (x == 0) {
	    writePixel(cx, cy + y, colour);
	    writePixel(cx, cy - y, colour);
	    writePixel(cx + y, cy, colour);
	    writePixel(cx - y, cy, colour);
    } else if (x == y) {
	    writePixel(cx + x, cy + y, colour);
	    writePixel(cx - x, cy + y, colour);
	    writePixel(cx + x, cy - y, colour);
	    writePixel(cx - x, cy - y, colour);
    } else if (x < y) {
	    writePixel(cx + x, cy + y, colour);
	    writePixel(cx - x, cy + y, colour);
	    writePixel(cx + x, cy - y, colour);
	    writePixel(cx - x, cy - y, colour);
	    writePixel(cx + y, cy + x, colour);
	    writePixel(cx - y, cy + x, colour);
	    writePixel(cx + y, cy - x, colour);
	    writePixel(cx - y, cy - x, colour);
    }
}

/*--------------------------------------------------------------------------------------
 Draw or clear a box(rectangle) with a single pixel border
--------------------------------------------------------------------------------------*/
void DMD::drawBox(int x1, int y1, int x2, int y2, byte colour)
{
    drawLine(x1, y1, x2, y1, colour);
    drawLine(x2, y1, x2, y2, colour);
    drawLine(x2, y2, x1, y2, colour);
    drawLine(x1, y2, x1, y1, colour);
}

/*--------------------------------------------------------------------------------------
 Draw or clear a filled box(rectangle) with a single pixel border
--------------------------------------------------------------------------------------*/
void DMD::drawFilledBox(int x1, int y1, int x2, int y2,
			byte colour)
{
    for (int b = x1; b <= x2; b++) {
	    drawLine(b, y1, b, y2, colour);
    }
}

/*--------------------------------------------------------------------------------------
 Draw the selected test pattern
--------------------------------------------------------------------------------------*/
void DMD::drawTestPattern(byte bPattern)
{
    unsigned int numPixels=DisplaysTotal * DMD_PIXELS_ACROSS * DMD_PIXELS_DOWN;
    int pixelsWide=DisplayMaxX;
    for (unsigned int ui = 0; ui < numPixels; ui++) {
	    switch (bPattern) {
	    case PATTERN_ALT_0:	// every alternate pixel, first pixel on
		    if ((ui & pixelsWide) == 0)
		        //even row
		        writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), ui & 1);
		    else
		        //odd row
		        writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), (ui+1) & 1);
		    break;
	    case PATTERN_ALT_1:	// every alternate pixel, first pixel off
		    if ((ui & pixelsWide) == 0)
		        //even row
		        writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), (ui+1) & 1);
		    else
		        //odd row
		        writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), ui & 1);
		    break;
	    case PATTERN_STRIPE_0:	// vertical stripes, first stripe on
		    writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), (ui & 1)*DisplaysBPP);
		    break;
	    case PATTERN_STRIPE_1:	// vertical stripes, first stripe off
		    writePixel((ui & (pixelsWide-1)), ((ui & ~(pixelsWide-1)) / pixelsWide), ((ui+1) & 1)*DisplaysBPP);
		    break;
        }
    }
}

/*--------------------------------------------------------------------------------------
 Scan the dot matrix LED panel display, from the RAM mirror out to the display hardware.
 Call 4 times to scan the whole display which is made up of 4 interleaved rows within the 16 total rows.
 Insert the calls to this function into the main loop for the highest call rate, or from a timer interrupt
--------------------------------------------------------------------------------------*/
void DMD::scanDisplayBySPI()
{
    //if PIN_OTHER_SPI_nCS is in use during a DMD scan request then scanDisplayBySPI() will exit without conflict! (and skip that scan)
    if( digitalRead( PIN_OTHER_SPI_nCS ) == HIGH )
    {
        //SPI transfer pixels to the display hardware shift registers
        int rowsize=DisplaysTotal<<2;
        int offset=rowsize * (bDMDByte & 3);
        byte col=bDMDByte>>2;
        for (int i=0;i<rowsize;i++) {
            SPI.transfer(bDMDScreenRAM[col+(DisplaysBPP*DisplayBuf)][offset+i+row3]);
            SPI.transfer(bDMDScreenRAM[col+(DisplaysBPP*DisplayBuf)][offset+i+row2]);
            SPI.transfer(bDMDScreenRAM[col+(DisplaysBPP*DisplayBuf)][offset+i+row1]);
            SPI.transfer(bDMDScreenRAM[col+(DisplaysBPP*DisplayBuf)][offset+i]);
        }

        OE_DMD_ROWS_OFF();
        LATCH_DMD_SHIFT_REG_TO_OUTPUT();
        switch (bDMDByte & 3) {
        case 0:			// row 1, 5, 9, 13 were clocked out
            LIGHT_DMD_ROW_01_05_09_13();
            break;
        case 1:			// row 2, 6, 10, 14 were clocked out
            LIGHT_DMD_ROW_02_06_10_14();
            break;
        case 2:			// row 3, 7, 11, 15 were clocked out
            LIGHT_DMD_ROW_03_07_11_15();
            break;
        case 3:			// row 4, 8, 12, 16 were clocked out
            LIGHT_DMD_ROW_04_08_12_16();
            break;
        }
        bDMDByte++;
        if (bDMDByte==4*DisplaysBPP) bDMDByte=0;
        OE_DMD_ROWS_ON();
    }
}

void DMD::selectFont(const uint8_t * font)
{
    this->Font = font;
}


byte DMD::drawChar(const int bX, const int bY, const char letter, byte fgcolour, byte bgcolour)
{
    if (bX >= DisplayMaxX || bY >= DisplayMaxY ) return -1;
    char c = letter;
    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);
    if (c == ' ') {
	    byte charWide = charWidth(' ');
	    this->drawFilledBox(bX, bY, bX + charWide, bY + height, bgcolour);
	    return charWide;
    }
    uint8_t width = 0;
    uint8_t bytes = (height + 7) / 8;

    uint8_t firstChar = pgm_read_byte(this->Font + FONT_FIRST_CHAR);
    uint8_t charCount = pgm_read_byte(this->Font + FONT_CHAR_COUNT);

    uint16_t index = 0;

    if (c < firstChar || c >= (firstChar + charCount)) return 0;
    c -= firstChar;

    if (pgm_read_byte(this->Font + FONT_LENGTH) == 0
	    && pgm_read_byte(this->Font + FONT_LENGTH + 1) == 0) {
	    // zero length is flag indicating fixed width font (array does not contain width data entries)
	    width = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
	    index = c * bytes * width + FONT_WIDTH_TABLE;
    } else {
	    // variable width font, read width data, to get the index
	    for (uint8_t i = 0; i < c; i++) {
	        index += pgm_read_byte(this->Font + FONT_WIDTH_TABLE + i);
	    }
	    index = index * bytes + charCount + FONT_WIDTH_TABLE;
	    width = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c);
    }
    if (bX < -width || bY < -height) return width;

    // last but not least, draw the character
    for (uint8_t j = 0; j < width; j++) { // Width
	    for (uint8_t i = bytes - 1; i < 254; i--) { // Vertical Bytes
	        uint8_t data = pgm_read_byte(this->Font + index + j + (i * width));
		    int offset = (i * 8);
		    if ((i == bytes - 1) && bytes > 1) {
		        offset = height - 8;
            } else if (height<8) {
                offset = height - 7;
            }
	        for (uint8_t k = 0; k < 8; k++) { // Vertical bits
		        if ((offset+k >= i*8) && (offset+k <= height)) {
		            if (data & (1 << k)) {
			            writePixel(bX + j, bY + offset + k, fgcolour);
		            } else {
			            writePixel(bX + j, bY + offset + k, bgcolour);
		            }
		        }
	        }
	    }
    }
    return width;
}

byte DMD::charWidth(const char letter)
{
    char c = letter;
    // Space is often not included in font so use width of 'n'
    if (c == ' ') c = 'n';
    uint8_t width = 0;

    uint8_t firstChar = pgm_read_byte(this->Font + FONT_FIRST_CHAR);
    uint8_t charCount = pgm_read_byte(this->Font + FONT_CHAR_COUNT);

    uint16_t index = 0;

    if (c < firstChar || c >= (firstChar + charCount)) {
	    return 0;
    }
    c -= firstChar;

    if (pgm_read_byte(this->Font + FONT_LENGTH) == 0
	&& pgm_read_byte(this->Font + FONT_LENGTH + 1) == 0) {
	    // zero length is flag indicating fixed width font (array does not contain width data entries)
	    width = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
    } else {
	    // variable width font, read width data
	    width = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c);
    }
    return width;
}

void DMD::dumpPixels()
{
/*
    for (int y=0;y<16;y++) {
        for (int x=0;x<32;x++) {
            if (bDMDScreenRAM[y][x/4] & bPixelLookupTable[x & 0x07]) {
                Serial.print(".");
            } else {
                Serial.print("#");
            }
        }
        Serial.println();
    }
*/
}

void DMD::scrollVert(int direction, boolean wrap) 
{
    int offset,newoffset;
    int rowsize=DisplaysTotal<<2;
    for (byte col=0;col<DisplaysBPP;col++) {
        if (direction<0) {
            for (int panelY=DisplaysHigh-1;panelY>=0;panelY--) {
                for (int y=15;y>=0;y--) {
                    offset=y*rowsize + (panelY*DisplaysWide*4);
                    newoffset=offset-rowsize;
                    for (byte x=0;x<(DisplaysWide<<2);x++) {
                        if (y==0) {
                            if (panelY>0) {
                                bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x] = bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][15*rowsize + ((panelY-1)*DisplaysWide*4)+x];
                            } else {
                                bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]=0xFF;
                            }
                        } else {
                            bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]=bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][newoffset+x];
                        }
                    }
                }
            }
        } else if (direction > 0) {
            for (byte panelY=0;panelY<DisplaysHigh;panelY++) {
                for (byte y=0;y<16;y++) {
                    offset=y*rowsize + (panelY*DisplaysWide*4);
                    newoffset=offset+rowsize;
                    for (byte x=0;x<(DisplaysWide<<2);x++) {
                        if (y==15) {
                            bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]=0xFF;
                        } else {
                            bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]=bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][newoffset+x];
                        }
                    }
                }
            }
        }
    }
}

void DMD::scrollHorz(int direction, boolean wrap)
{
    int rowsize=DisplaysTotal<<2;
    for (byte col=0; col<DisplaysBPP;col++) {
        if (direction<0) {
            for (byte panelY=0;panelY<DisplaysHigh;panelY++) {
                for (byte y=0;y<16;y++) {
                    int offset=panelY*(DisplaysWide*4) + y*rowsize;
                    for (byte x=0;x<(DisplaysWide<<2)-1;x++) {
                        bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]=(bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]<<1) + ((bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x+1] & 0x80) >>7);
                    }
                    byte x=(DisplaysWide<<2)-1;
                    bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]=(bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]<<1) + 1;
                }
            }
        } else if (direction>0) {
            for (byte panelY=0;panelY<DisplaysHigh;panelY++) {
                for (byte y=0;y<16;y++) {
                    int offset=panelY*(DisplaysWide*4) + y*rowsize;
                    for (int x=(DisplaysWide<<2)-1;x>0;x--) {
                        bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]=(bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x]>>1) + ((bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset+x-1] & 1) <<7);
                    }
                    bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset]=(bDMDScreenRAM[col+(DisplaysBPP*EditBuf)][offset]>>1)+128;
                }
            }
        }
    }
}

byte DMD::getPixel(unsigned int bX, unsigned int bY)
{
    unsigned int uiDMDRAMPointer;

    if (bX >= (DMD_PIXELS_ACROSS*DisplaysWide) || bY >= (DMD_PIXELS_DOWN * DisplaysHigh)) {
            return 0;
    }
    byte panel=(bX/DMD_PIXELS_ACROSS) + (DisplaysWide*(bY/DMD_PIXELS_DOWN));
    bX=(bX % DMD_PIXELS_ACROSS) + (panel<<5);
    bY=bY % DMD_PIXELS_DOWN;
    //set pointer to DMD RAM byte to be modified
    uiDMDRAMPointer = bX/8 + bY*(DisplaysTotal<<2);

    byte lookup = bPixelLookupTable[bX & 0x07];
    byte col=0;
    for (byte i=0;i<DisplaysBPP;i++) {
        if ((bDMDScreenRAM[i+(DisplaysBPP*EditBuf)][uiDMDRAMPointer] & lookup) == 0) {
            col=col+(1<<i);
        }
    }
    return col;
}

/* Transition between two buffers with output to third */
boolean DMD::transition(byte frombuffer1, byte frombuffer2, byte outbuffer, byte transType, int step)
{
    unsigned int uiDMDRAMPointer;
    byte oldEditBuf=EditBuf;
    int centreX=DisplayMaxX/2;
    int centreY=DisplayMaxY/2;

    for (int x=0;x<(DisplayMaxX/8);x++) {
        for (int y=0;y<DisplayMaxY;y++) {
            int panel = ((x*8)/DMD_PIXELS_ACROSS) +(DisplaysWide*(y/DMD_PIXELS_DOWN));
            int bX=((x*8) % DMD_PIXELS_ACROSS) + (panel<<5);
            int bY=y % DMD_PIXELS_DOWN;
            //set pointer to DMD RAM byte to be modified
            uiDMDRAMPointer = bX/8 + bY*(DisplaysTotal<<2);
            for (byte col=0;col<DisplaysBPP;col++) {
                byte outByte=0;
                byte b1=bDMDScreenRAM[col+(DisplaysBPP*frombuffer1)][uiDMDRAMPointer];
                byte b2=bDMDScreenRAM[col+(DisplaysBPP*frombuffer2)][uiDMDRAMPointer];
	            switch (transType) {
	            case TRANS_WIPE_DOWN:
                    if (y<step) {
                        outByte=b2;
                    } else {
                        outByte=b1;
                    }
                    break;
	            case TRANS_WIPE_UP:
                    if ((DisplayMaxY-y)<step) {
                        outByte=b2;
                    } else {
                        outByte=b1;
                    }
                    break;
	            case TRANS_WIPE_LEFT:
                    if (((DisplayMaxX/8)-x)<((step/8)+1)) {
                        outByte=b2;
                    } else if (((DisplayMaxX/8)-x)>((step/8)+1)) {
                        outByte=b1;
                    } else {
                        int off=step%8;
                        int mask=(1<<(off+1)) -1;
                        outByte=(b1|mask)&(b2|(~mask));
                    }
                    break;
	            case TRANS_WIPE_RIGHT:
                    if (x< (step/8) ) {
                        outByte=b2;
                    } else if (x> (step/8) ) {
                        outByte=b1;
                    } else {
                        int off=7-(step%8);
                        int mask=(1<<(off+1)) -1;
                        outByte=(b2|mask)&(b1|(~mask));
                    }
                    break;
	            case TRANS_BOX_IN:
                    if (y<step || (y-DisplayMaxY>-step)) { // top/bottom
                        outByte=b2;
                    } else {
                        if (x< (step/8) ) { // left
                            outByte=b2;
                        } else if (x> (step/8) ) { 
                            if (((DisplayMaxX/8)-x)<((step/8)+1)) { // right
                                outByte=b2;
                            } else if (((DisplayMaxX/8)-x)>((step/8)+1)) {
                                outByte=b1;
                            } else { //right crossover
                                int off=step%8;
                                int mask=(1<<(off+1)) -1;
                                outByte=(b1|mask)&(b2|(~mask));
                            }
                        } else { // left crossover
                            int off=7-(step%8);
                            int mask=(1<<(off+1)) -1;
                            outByte=(b2|mask)&(b1|(~mask));
                        }
                    } 
                    break;
	            case TRANS_BOX_OUT:
                    if (y>(centreY-step) && (y-DisplayMaxY<-(centreY-step))) {
                        if (((centreX/8)-x)< ((step/8)+1)) { 
                            outByte=b2;
                            if ((x-(centreX/8)) > (step/8)) {
                                outByte=b1;
                            } else if ((x-(centreX/8))<(step/8)) {
                                outByte=b2;
                            } else {
                                int off=7-(step%8);
                                int mask=(1<<(off+1)) -1;
                                outByte=(b2|mask)&(b1|(~mask));
                            }
                        } else if (((centreX/8)-x)> ((step/8)+1) ) {
                            outByte=b1;
                        } else { 
                            int off=step%8;
                            int mask=(1<<(off+1)) -1;
                            outByte=(b1|mask)&(b2|(~mask));
                        }
                    } else { 
                        outByte=b1;
                    }
                    break;
	            case TRANS_CROSS_IN:
                    if (y>step && (y-DisplayMaxY<-step)) { 
                        outByte=b1;
                    } else {
                        if ((x < (step/8)) || ((DisplayMaxX/8)-x)<((step/8)+1) ) { 
                            outByte=b2;
                        } else if (x > (step/8) ) { 
                            if (((DisplayMaxX/8)-x)>((step/8)+1)) {
                                outByte=b1;
                            } else { 
                                int off=step%8;
                                int mask=(1<<(off+1)) -1;
                                outByte=(b1|mask)&(b2|(~mask));
                            }
                        } else {
                            int off=7-(step%8);
                            int mask=(1<<(off+1)) -1;
                            outByte=(b2|mask)&(b1|(~mask));
                        }
                    } 
                    break;
	            case TRANS_CROSS_OUT:
                    if (y<(centreY-step) || (y-DisplayMaxY>-(centreY-step))) {
                        if (((centreX/8)-x)< ((step/8)+1)) { 
                            outByte=b2;
                            if ((x-(centreX/8)) > (step/8)) {
                                outByte=b1;
                            } else if ((x-(centreX/8))<(step/8)) {
                                outByte=b2;
                            } else {
                                int off=7-(step%8);
                                int mask=(1<<(off+1)) -1;
                                outByte=(b2|mask)&(b1|(~mask));
                            }
                        } else if (((centreX/8)-x)> ((step/8)+1) ) {
                            outByte=b1;
                        } else { 
                            int off=step%8;
                            int mask=(1<<(off+1)) -1;
                            outByte=(b1|mask)&(b2|(~mask));
                        }
                    } else { 
                        outByte=b2;
                    }
                    break;
                }
                bDMDScreenRAM[0+(DisplaysBPP*outbuffer)][uiDMDRAMPointer] = outByte;
            }
        }
    }
    EditBuf=oldEditBuf;
	switch (transType) {
	case TRANS_WIPE_DOWN:
	case TRANS_WIPE_UP:
        if (step>=DisplayMaxY) return false;
        break;
	case TRANS_WIPE_LEFT:
	case TRANS_WIPE_RIGHT:
        if (step>=DisplayMaxX) return false;
        break;
	case TRANS_BOX_OUT:
	case TRANS_CROSS_OUT:
	case TRANS_CROSS_IN:
        if (step>=(max(centreX,centreY))) return false;
        break;
	case TRANS_BOX_IN:
        if (step>(min(centreX,centreY))) return false;
        break;
    }
    return true;
}

/* Copy one buffer to another */
void DMD::copyBuffer(byte frombuffer, byte tobuffer)
{
    for (byte col=0; col<DisplaysBPP;col++) {
        memcpy(bDMDScreenRAM[col+(DisplaysBPP*tobuffer)],bDMDScreenRAM[col+(DisplaysBPP*frombuffer)],DMD_RAM_SIZE_BYTES*DisplaysTotal);
    }
}
