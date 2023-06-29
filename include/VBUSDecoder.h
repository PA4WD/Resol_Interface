/** Copyright (c) 2017 - 'FatBeard' @ www.domoticz.com/forum
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#ifndef vbusdecoder_h
#define vbusdecoder_h

#include <Arduino.h>

// Settings for the VBus decoding
#define FLength 6				// Framelength
#define FOffset 10				// Offset start of Frames
#define FSeptet 4				// Septet byte in Frame
#define SENSORNOTCONNECTED 8888 // Sometimes this might be 888 instead.

class VBUSDecoder
{
public:
	bool messageDecode(unsigned char *buffer, size_t length);
	float getS1Temp();
	float getS2Temp();
	float getS3Temp();
	float getS4Temp();
	// bool getP1Status();
	// bool getP2Status();
	uint8_t getP1Speed();
	uint8_t getP2Speed();
	uint16_t getP1OperatingHours();
	uint16_t getP2OperatingHours();

	bool getAlertStatus();
	int getScheme();
	String getSystemTime();
	uint32_t getHeatQuantity();

	// float Sensor1Temp;
	// float Sensor2Temp;
	// float Sensor3Temp;
	// float Sensor4Temp;

protected:
private:
	float Sensor1Temp;
	float Sensor2Temp;
	float Sensor3Temp;
	float Sensor4Temp;

	// Conergy DT5 specific
	uint8_t PumpSpeed1; // in  %
	uint8_t PumpSpeed2; //  in %
	uint16_t OperatingHoursRelay1;
	uint16_t OperatingHoursRelay2;
	char RelaisMask;
	char ErrorMask;
	uint32_t StatusMask;
	char Scheme;
	char OptionPostPulse;
	char OptionThermostat;
	char OptionHQM;
	uint32_t HeatQuantity;
	uint16_t Version;
	uint16_t SystemTime;

	char Relay1;	  // in  %
	char Relay2;	  //  in %
	char MixerOpen;	  // in  %
	char MixerClosed; // in  %
	char SystemNotification;

	bool resolConergyDT5(unsigned char *buffer);
	bool deltaSolCSPlus(unsigned char *buffer);
	bool resolDeltathermFK(unsigned char *buffer);
	bool resolDeltaSolC(unsigned char *buffer);
	bool resolDeltaSolM(unsigned char *buffer);
	bool SKSC1_2(unsigned char *buffer);
	bool resolDefault(unsigned char *buffer);

	void ExtractSeptett(unsigned char *buffer, int offset, int length);
	void InjectSeptett(unsigned char *buffer, int offset, int length);
	unsigned char CalcCrc(const unsigned char *buffer, int offset, int length);

	uint16_t ReadUInt16LE(unsigned char *buffer);
	uint32_t ReadUInt32LE(unsigned char *buffer);
	float ReadTemp(unsigned char *buffer);

	void PrintHex8(unsigned char *data, uint8_t length); // prints 8-bit data in hex with leading zeroes
};

#endif
