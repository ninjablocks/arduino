#include "OSv2ProtocolDecoder.h"

#include "../common/Ninja.h"

/*********************************************\

Manchester encoding (as used for the Oregon Scientific v2 devices
assume the very first bit is zero.
two short pulses -> current bit is the same as previous bit
a long pulse -> current bit is ~previous bit (flips the bit)
a long pulse cannot follow one short pulse
each bit writen to the wire twice, first time negated.
Packet starts with 16 bit premble of all ones 	- ie:31 long pulses 

OSv2 uses 17 nibbles in total for this device

/********************DINUKA*******************/


byte total_bits, flip, state, pos, data[25]={0};
enum { UNKNOWN, T0, T1, T2, T3, OK, DONE };

OSv2ProtocolDecoder::OSv2ProtocolDecoder()
{
	m_nPulseLength = 0;
	state = UNKNOWN;
}

void OSv2ProtocolDecoder::resetDecoder() {
	total_bits = pos = flip = 0;
	state = UNKNOWN;
}

void OSv2ProtocolDecoder::manchester(char value) {
	flip ^= value; // manchester code, long pulse flips the bit
	gotBit(flip);
}


void OSv2ProtocolDecoder::gotBit(char value) {
        if(!(total_bits & 0x01))			//only update using the second bit for each bit pair, because each bit is written twice to the wire.
        {
            data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
        }
        total_bits++;
        //digitalWrite(BLUE_LED_PIN, HIGH);
        pos = total_bits >> 4;
        if (pos >= sizeof data) {
            resetDecoder();
            return;
        }
        state = OK;
		//if(pos==8) Serial.println(data[pos]);
}

boolean OSv2ProtocolDecoder::decode(RFPacket* pPacket)
{
	word width = 0;

	// number of pulses depend on the data value, therefore cannot use pulse count to limit search
	while(total_bits <136)
	{
		width = pPacket->next();

        if (200 <= width && width < 1200) {
            //digitalWrite(BLUE_LED_PIN, HIGH);
            byte w = width >= 700;
            switch (state) {
                case UNKNOWN:
                    if (w != 0) {
                        // Long pulse
                        ++flip;
                    } else if (31 <= flip) {
                        // Short pulse, start bit
                        //digitalWrite(BLUE_LED_PIN, HIGH);
                        flip = 0;
                        state = T0;
                    } else {
                      // Reset decoder
						resetDecoder();
                        return false;
                    }
                    break;
                case OK:
                    if (w == 0) {
                        // Short pulse
                        state = T0;
                    } else {
                        // Long pulse
                        manchester(1);
                    }
                    break;
                case T0:
                    if (w == 0) {
                      // Second short pulse
                        manchester(0);
					  //Store width of the short pulse for RF transmission purposes
						m_nPulseLength = width;						
                    } else {
                        // Reset decoder because a long pulse cannot follow a single short pulse
						resetDecoder();
                        return false;
                    }
                    break;
            }
            //digitalWrite(BLUE_LED_PIN, HIGH);
        } else {
            return false;
        }
	}
	if(total_bits == 136) {
		//resetDecoder(); //finished decoding packet, prepare for next
		//pPacket->print();//***********************************************************DEBUG
		//Serial.println(data[8]);
		return 1;
	}
	else return false;

}

void OSv2ProtocolDecoder::reportSerial() {
    //byte pos;
    //const byte* data = decoder.getData(pos);
    //Serial.print(s);
    Serial.print("OSv2 ");
    for (byte i = 0; i < pos; ++i) {
        Serial.print(data[i] >> 4, HEX);
        Serial.print(data[i] & 0x0F, HEX);
    }
    
    // Serial.print(' ');
    // Serial.print(millis() / 1000);
    Serial.println();
	//resetDecoder();
}


void OSv2ProtocolDecoder::fillPacket(NinjaPacket* pPacket)
{
	pPacket->setEncoding(ENCODING_OSV2);
	pPacket->setTiming(m_nPulseLength);
	pPacket->setType(TYPE_DEVICE);
	pPacket->setGuid(0);
	pPacket->setDevice(ID_ONBOARD_RF);
	pPacket->setData(data, pos+1);
	pPacket->setDataInArray();
	resetDecoder();
}