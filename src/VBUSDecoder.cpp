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

#include "VBUSDecoder.h"
//#define DEBUG 1

float VBUSDecoder::getS1Temp()
{
  return Sensor1Temp;
}

float VBUSDecoder::getS2Temp()
{
  return Sensor2Temp;
}

float VBUSDecoder::getS3Temp()
{
  return Sensor3Temp;
}

float VBUSDecoder::getS4Temp()
{
  return Sensor4Temp;
}

// bool VBUSDecoder::getP1Status()
// {
//   if (Relay1 != 0x00)
//   {
//     return true;
//   }
//   else
//   {
//     return false;
//   }
// }

// bool VBUSDecoder::getP2Status()
// {
//   if (Relay2 == 0x64)
//   {
//     return true;
//   }
//   else if (Relay2 == 0x00)
//   {
//     return false;
//   }
//   else
//   {
//     return false;
//   }
// }

uint8_t VBUSDecoder::getP1Speed()
{
  return PumpSpeed1;
}

uint8_t VBUSDecoder::getP2Speed()
{
  return PumpSpeed2;
}

uint16_t VBUSDecoder::getP1OperatingHours()
{
  return OperatingHoursRelay1;
}

uint16_t VBUSDecoder::getP2OperatingHours()
{
  return OperatingHoursRelay2;
}

int VBUSDecoder::getScheme()
{
  return Scheme;
}

bool VBUSDecoder::getAlertStatus()
{
  if (SystemNotification != 0x00 || ErrorMask != 0x00) // Not really sure what ErrorMask is, treating as system alert.
  {
    return true;
  }
  else
  {
    return false;
  }
}

String VBUSDecoder::getSystemTime()
{
  int hours = SystemTime / 60;
  int minutes = SystemTime % 60;
  String toReturn = String(String(hours) + ":" + String(minutes));
  if (hours < 10)
  {
    toReturn = "0" + toReturn;
  }
  return toReturn;
}

uint32_t VBUSDecoder::getHeatQuantity()
{
  return HeatQuantity;
}

// #if DEBUG
//     Serial.println("------Decoded VBus data------");
//     Serial.print("Destination: ");
//     Serial.println(Destination_address, HEX);
//     Serial.print("Source: ");
//     Serial.println(Source_address, HEX);
//     Serial.print("Protocol Version: ");
//     Serial.println(ProtocolVersion);
//     Serial.print("Command: ");
//     Serial.println(Command, HEX);
//     Serial.print("Framecount: ");
//     Serial.println(Framecnt);
//     Serial.print("Checksum: ");
//     Serial.println(Checksum);
//     Serial.println("------Values------");
//     Serial.print("Sensor 1: ");
//     Serial.println(sensor1Temp);
//     Serial.print("Sensor 2: ");
//     Serial.println(sensor2Temp);
//     Serial.print("Sensor 3: ");
//     Serial.println(sensor3Temp);
//     Serial.print("Sensor 4: ");
//     Serial.println(sensor4Temp);
//     Serial.print("Relay 1: ");
//     Serial.println(Relay1, DEC);
//     Serial.print("Relay 2: ");
//     Serial.println(Relay2, DEC);
//     Serial.print("Minute of Day: ");
//     Serial.println(SystemTime);
//     Serial.print("Notifications: ");
//     Serial.println(SystemNotification, DEC);
//     Serial.println("------END------");
// #endif

bool VBUSDecoder::resolConergyDT5(unsigned char *buffer)
{
#if DEBUG
  Serial.println(F("Now decoding for 0x3271"));
#endif

  // Frame info for the Resol ConergyDT5
  // check VBusprotocol specification for other products

  // Offset  Size    Mask    Name                    Factor  Unit
  // 0       2               Temperature sensor 1    0.1     &#65533;C
  // 2       2               Temperature sensor 2    0.1     &#65533;C
  // 4       2               Temperature sensor 3    0.1     &#65533;C
  // 6       2               Temperature sensor 4    0.1     &#65533;C
  // 8       1               Pump speed pump         1       1
  // 9       1               Pump speed pump 2       1
  // 10      1               Relay mask              1
  // 11      1               Error mask              1
  // 12      2               System time             1
  // 14      1               Scheme                  1
  // 15      1       1       Option PostPulse        1
  // 15      1       2       Option thermostat       1
  // 15      1       4       Option HQM              1
  // 16      2               Operating hours relay 1 1
  // 18      2               Operating hours relay 2 1
  // 20      2               Heat quantity           1       Wh
  // 22      2               Heat quantity           1000    Wh
  // 24      2               Heat quantity           1000000 Wh
  // 26      2               Version 0.01
  //
  //  Each frame has 6 bytes
  //  byte 1 to 4 are data bytes -> MSB of each bytes
  //  byte 5 is a septet and contains MSB of bytes 1 to 4
  //  byte 6 is a checksum
  //
  //*******************  Frame 1  *******************
  int offset = FOffset;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor1Temp = ReadTemp(&buffer[offset]);
    Sensor2Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 2  *******************
  offset = FOffset + FLength;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor3Temp = ReadTemp(&buffer[offset]);
    Sensor4Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 3  *******************
  offset = FOffset + FLength * 2;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    PumpSpeed1 = (buffer[offset]);
    PumpSpeed2 = (buffer[offset + 1]);
    RelaisMask = buffer[offset + 2];
    ErrorMask = buffer[offset + 3];
  }
  else
    return false;
  //*******************  Frame 4  *******************
  offset = FOffset + FLength * 3;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    // SystemTime = buffer[offset + 1] << 8 | buffer[offset];
    SystemTime = ReadUInt16LE(&buffer[offset]);
    Scheme = buffer[offset + 2];
    OptionPostPulse = (buffer[offset + 3] & 0x01);
    OptionThermostat = ((buffer[offset + 3] & 0x02) >> 1);
    OptionHQM = ((buffer[offset + 3] & 0x04) >> 2);
  }
  else
    return false;
  //*******************  Frame 5  *******************
  offset = FOffset + FLength * 4;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    OperatingHoursRelay1 = buffer[offset + 1] << 8 | buffer[offset];
    OperatingHoursRelay2 = buffer[offset + 3] << 8 | buffer[offset + 2];
  }
  else
    return false;
  //*******************  Frame 6  *******************
  offset = FOffset + FLength * 5;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    HeatQuantity = (buffer[offset + 1] << 8 | buffer[offset]) + (buffer[offset + 3] << 8 | buffer[offset + 2]) * 1000;
  }
  else
    return false;
  //*******************  Frame 7  *******************
  offset = FOffset + FLength * 6;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    HeatQuantity = HeatQuantity + (buffer[offset + 1] << 8 | buffer[offset]) * 1000000;
    Version = buffer[offset + 3] << 8 | buffer[offset + 2];
  }
  else
    return false;
  ///******************* End of frames ****************
  return true;
}

bool VBUSDecoder::deltaSolCSPlus(unsigned char *buffer)
{
#if DEBUG
  Serial.println(F("Now decoding for DeltaSol CS Plus 0x2211"));
#endif

  // Frame info for the Resol DeltaSol CS Plus (Joule)
  // check VBusprotocol specification for other products
  // Offset  Mask        Name                Factor      Unit
  // 0                   Temperature S1      1.0         °C
  // 1                   Temperature S1      256.0       °C
  // 2                   Temperature S2      1.0         °C
  // 3                   Temperature S2      256.0       °C

  // 4                   Temperature S3      1.0         °C
  // 5                   Temperature S3      256.0       °C
  // 6                   Temperature S4      1.0         °C
  // 7                   Temperature S4      256.0       °C

  // 8                   Pump Speed R1       1           %
  // 10                  Operating Hours R1  1           h
  // 11                  Operating Hours R1  256         h

  // 12                  Pump Speed R2       1           %
  // 14                  Operating Hours R2  1           h
  // 15                  Operating Hours R2  256         h

  // 16                  UnitType            1
  // 17                  System              1

  // 20          1       Sensor 1 defekt     1
  // 20          2       Sensor 2 defekt     1
  // 20          4       Sensor 3 defekt     1
  // 20          8       Sensor 4 defekt     1
  // 20                  Error Mask          1
  // 20                  Error Mask          256
  // 22                  Time                1
  // 23                  Time                256

  // 24                  Statusmask          1
  // 25                  Statusmask          256
  // 26                  Statusmask          65536
  // 27                  Statusmask          16777216

  // 28                  Heat Quantity       1           Wh
  // 29                  Heat Quantity       256         Wh
  // 30                  Heat Quantity       65536       Wh
  // 31                  Heat Quantity       16777216    Wh

  // 32                  SW-Version          0.01
  // 33                  SW-Version          2.56

  // 36 		        Temperature sensor 5 	   0.1 	    °C
  // 37 		        Temperature sensor 5 	   25.6 	  °C
  // 38 		            Flow rate 	         1 	      l/h
  // 39 		            Flow rate 	         256 	    l/h

  // Each frame has 6 bytes, FLength
  // byte 1 to 4 are data bytes -> MSB of each bytes
  // byte 5 is a septet and contains MSB of bytes 1 to 4, FSeptet
  // byte 6 is a checksum
  // FOffset 10, Offset start of Frames

  //*******************  Frame 1  *******************
  int offset = FOffset;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  {
    InjectSeptett(buffer, offset, 4);
    Sensor1Temp = ReadTemp(&buffer[offset]);
    Sensor2Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;

  //*******************  Frame 2  *******************
  offset = FOffset + FLength;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  {
    InjectSeptett(buffer, offset, 4);
    Sensor3Temp = ReadTemp(&buffer[offset]);
    Sensor4Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;

  //*******************  Frame 3  *******************
  offset = FOffset + FLength * 2;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  {
    InjectSeptett(buffer, offset, 4);
    PumpSpeed1 = buffer[offset];
    OperatingHoursRelay1 = ReadUInt16LE(&buffer[offset + 2]);
  }
  else
    return false;

  //*******************  Frame 4  *******************
  offset = FOffset + FLength * 3;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  {
    InjectSeptett(buffer, offset, 4);
    PumpSpeed2 = buffer[offset];
    OperatingHoursRelay2 = ReadUInt16LE(&buffer[offset + 2]);
  }
  else
    return false;

  //*******************  Frame 5  *******************
  offset = FOffset + FLength * 4;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  {
    InjectSeptett(buffer, offset, 4);
    // 16                  UnitType            1

    Scheme = buffer[offset + 1];
    // 17                  System              1
  }
  else
    return false;

  //*******************  Frame 6  *******************
  offset = FOffset + FLength * 5;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  {
    InjectSeptett(buffer, offset, 4);    
    ErrorMask = ReadUInt16LE(&buffer[offset]);
    SystemTime = ReadUInt16LE(&buffer[offset + 2]);
  }
  else
    return false;

  //*******************  Frame 7  *******************
  offset = FOffset + FLength * 6;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  {
    InjectSeptett(buffer, offset, 4);
    StatusMask = ReadUInt32LE(&buffer[offset]);
    // 24                  Statusmask          1
    // 25                  Statusmask          256
    // 26                  Statusmask          65536
    // 27                  Statusmask          16777216
  }
  else
    return false;

  //*******************  Frame 8  *******************
  offset = FOffset + FLength * 7;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  {
    InjectSeptett(buffer, offset, 4);
    HeatQuantity = ReadUInt32LE(&buffer[offset]);
  }
  else
    return false;

  ///******************* End of frames ****************
  return true;
}

bool VBUSDecoder::resolDeltathermFK(unsigned char *buffer)
{
#if DEBUG
  Serial.println("Now decoding for 0x5611");
#endif
  // Frame info for the Resol Deltatherm FK and Oranier Aquacontrol III
  // check VBusprotocol specification for other products

  // Offset  Size    Mask    Name                    Factor  Unit
  //  Frame 1
  // 0       2               Temperature sensor 1    0.1     &#65533;C
  // 2       2               Temperature sensor 2    0.1     &#65533;C
  //  Frame 2
  // 4       2               Temperature sensor 3    0.1     &#65533;C
  // 6       2               Temperature sensor 4    0.1     &#65533;C
  //  Frame 3
  // 8       1               Relay 1                 1       %
  // 9       1               Relay 2                 1       %
  // 10      1               Mixer open              1       %
  // 11      1               Mixer closed            1       %
  //  Frame 4
  // 12      4               System date             1
  //  Frame 5
  // 16      2               System time             1
  // 18      1               System notification     1
  //
  //  Each frame has 6 bytes
  //  byte 1 to 4 are data bytes -> MSB of each bytes
  //  byte 5 is a septet and contains MSB of bytes 1 to 4
  //  byte 6 is a checksum
  //
  //*******************  Frame 1  *******************

  int offset = FOffset;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor1Temp = ReadTemp(&buffer[offset]);
    Sensor2Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 2  *******************
  offset = FOffset + FLength;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor3Temp = ReadTemp(&buffer[offset]);
    Sensor4Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 3  *******************
  offset = FOffset + FLength * 2;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    // Some of the values are 7 bit instead of 8.
    // Adding '& 0x7F' means you are only interested in the first 7 bits.
    // 0x7F = 0b1111111.
    // See: http://stackoverflow.com/questions/9552063/c-language-bitwise-trick
    Relay1 = (buffer[offset] & 0X7F);
    Relay2 = (buffer[offset + 1] & 0X7F);
    MixerOpen = (buffer[offset + 2] & 0X7F);
    MixerClosed = (buffer[offset + 3] & 0X7F);
  }
  else
    return false;
  //*******************  Frame 4  *******************
  offset = FOffset + FLength * 3;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    SystemTime = ReadUInt16LE(&buffer[offset]);
  }
  else
    return false;
  //*******************  Frame 5  *******************
  offset = FOffset + FLength * 4;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    // System time is not needed for Domoticz

    // Status codes System Notification according to Resol:
    // 0: no error / warning
    // 1: S1 defect
    // 2: S2 defect
    // 3: S3 defect
    // 4: VFD defect
    // 5: Flow rate?
    // 6: ΔT too high
    // 7: Low water level

    SystemNotification = buffer[offset + 2];
  }
  else
    return false;
  ///******************* End of frames ****************
  return true;
}

bool VBUSDecoder::resolDeltaSolC(unsigned char *buffer)
{
#if DEBUG
  Serial.println("Now decoding for DeltaSol C 0x4212");
#endif
  //*******************  Frame 1  *******************
  int offset = FOffset;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor1Temp = ReadTemp(&buffer[offset]);
    Sensor2Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 2  *******************
  offset = FOffset + FLength;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor3Temp = ReadTemp(&buffer[offset]);
    Sensor4Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 3  *******************
  offset = FOffset + FLength * 2;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Relay1 = (buffer[offset]);
    Relay2 = (buffer[offset + 1]);
    ErrorMask = buffer[offset + 2];
    Scheme = buffer[offset + 3];
  }
  else
    return false;
  //*******************  Frame 4  *******************
  offset = FOffset + FLength * 3;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    OperatingHoursRelay1 = buffer[offset + 1] << 8 | buffer[offset];
    OperatingHoursRelay2 = buffer[offset + 3] << 8 | buffer[offset + 2];
  }
  else
    return false;
  //*******************  Frame 5  *******************
  offset = FOffset + FLength * 4;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    HeatQuantity = (buffer[offset + 1] << 8 | buffer[offset]) + (buffer[offset + 3] << 8 | buffer[offset + 2]) * 1000;
  }
  else
    return false;
  //*******************  Frame 6  *******************
  offset = FOffset + FLength * 5;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    HeatQuantity = HeatQuantity + (buffer[offset + 1] << 8 | buffer[offset]) * 1000000;
    // SystemTime = buffer[offset + 3] << 8 | buffer[offset + 2];
    SystemTime = ReadUInt16LE(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 7  *******************
  offset = FOffset + FLength * 6;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
  }
  else
    return false;
  ///******************* End of frames ****************
  return true;
}

bool VBUSDecoder::resolDeltaSolM(unsigned char *buffer)
{
  // Deltasol M, alias Roth B/W Komfort
  // 6 temp frames, 12 sensors
  // Only decoding the first four due to library limitations
  unsigned int frame = 0;
  // Frame 1:
  int offset = FOffset + frame * FLength;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor1Temp = ReadTemp(&buffer[offset]);
    Sensor2Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  // Frame 2:
  frame++;
  offset = FOffset + frame * FLength;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor3Temp = ReadTemp(&buffer[offset]);
    Sensor4Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  // Frame 7: Irradiation and unused
  /*
  frame = 7
  offset = FOffset + 6 * FLength;
  irradiation = CalcTemp(buffer[offset + 1], buffer[offset]);
  */
  // Frame 8: Pulse counter 1
  // Frame 9: Pulse counter 2
  // Frame 10: Sensor errors: no sensor / shorted
  // Frame 11: Sensors
  // Frame 12: Relays 1-4
  // Frame 13: Relays 5-8
  // Frame 14: Relays 9-12
  frame = 11;
  offset = FOffset + frame * FLength;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    Relay1 = buffer[offset];
    Relay2 = buffer[offset + 1];
  }
  else
    return false;
    // Frame 15: Not used / relays
    // Frame 16: Errors / warnings
    // Frame 17: Version, revision / time
#ifdef DEBUG
  Serial.println("Got values");
  Serial.print("Temperature: ");
  Serial.print(Sensor1Temp);
  Serial.print(", ");
  Serial.print(Sensor2Temp);
  Serial.print(". Relays: ");
  Serial.print(Relay1);
  Serial.print(", ");
  Serial.println(Relay2);
#endif
  return true;
}

bool VBUSDecoder::SKSC1_2(unsigned char *buffer)
{
#if DEBUG
  Serial.println("Now decoding for 0x4211  SKSC1/2");
#endif

  //*******************  Frame 1  *******************
  int offset = FOffset;
  // unsigned char septet;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor1Temp = ReadTemp(&buffer[offset]);
    Sensor2Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 2  *******************
  offset = FOffset + FLength;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor3Temp = ReadTemp(&buffer[offset]);
    Sensor4Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 3  *******************
  // store Pump data in relay as it gets data from there.. dont know why?
  // Relay1 is then the pump speed in %
  offset = FOffset + FLength * 2;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Relay1 = (buffer[offset]);
    Relay2 = (buffer[offset + 1]);
    RelaisMask = buffer[offset + 2];
    ErrorMask = buffer[offset + 3];
  }
  else
    return false;

  return true;
}

bool VBUSDecoder::resolDefault(unsigned char *buffer)
{
  // Default temp 1-4 extraction
  // For most Resol controllers temp 1-4 are always available, so
  // even if you do not know the datagram format you can still see
  // these temps 1 to 4.

  // Offset  Size    Mask    Name                    Factor  Unit
  //  Frame 1
  // 0       2               Temperature sensor 1    0.1     &#65533;C
  // 2       2               Temperature sensor 2    0.1     &#65533;C
  //  Frame 2
  // 4       2               Temperature sensor 3    0.1     &#65533;C
  // 6       2               Temperature sensor 4    0.1     &#65533;C
  //
  //  Each frame has 6 bytes
  //  byte 1 to 4 are data bytes -> MSB of each bytes
  //  byte 5 is a septet and contains MSB of bytes 1 to 4
  //  byte 6 is a checksum
  //
  //*******************  Frame 1  *******************
  int offset = FOffset;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor1Temp = ReadTemp(&buffer[offset]);
    Sensor2Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  //*******************  Frame 2  *******************
  offset = FOffset + FLength;
  if (buffer[offset + 5] == CalcCrc(buffer, offset, 5))
  { // CRC ok
    InjectSeptett(buffer, offset, 4);
    Sensor3Temp = ReadTemp(&buffer[offset]);
    Sensor4Temp = ReadTemp(&buffer[offset + 2]);
  }
  else
    return false;
  ///******************* End of frames ****************
  return true;
}

// The following function reads the data from the bus and converts it all
// depending on the used VBus controller.
bool VBUSDecoder::messageDecode(unsigned char *buffer, size_t length)
{
  unsigned int destinationAddress = buffer[2] << 8;
  destinationAddress |= buffer[1];
  unsigned int sourceAddress = buffer[4] << 8;
  sourceAddress |= buffer[3];
  unsigned char protocolVersion = (buffer[5] >> 4) + (buffer[5] & (1 << 15));

  unsigned int command = buffer[7] << 8;
  command |= buffer[6];

#if DEBUG
  Serial.print("Destination: ");
  Serial.println(destinationAddress, HEX);
  Serial.print("Source: ");
  Serial.println(sourceAddress, HEX);
  Serial.print("Protocol Version: ");
  Serial.println(protocolVersion);
  Serial.print("Command: ");
  Serial.println(command, HEX);
#endif

  if (protocolVersion == 1)
  {
    unsigned char framecnt = buffer[8];
    unsigned char checksum = buffer[9];

#if DEBUG
    Serial.print("Framecount: ");
    Serial.println(framecnt);
    Serial.print("Checksum: ");
    Serial.println(checksum);
    Serial.print("Length: ");
    Serial.println(length);
#endif

    if (length != 9 + framecnt * 6)
    {
#ifdef DEBUG
      Serial.println("Framecount error");
#endif
      return false;
    }

    if (checksum != CalcCrc(buffer, 1, 8))
    {
#ifdef DEBUG
      Serial.println("Checksum error");
#endif
      return false;
    }

    if (command == 0x0100)
    {
      // Only decode the data from the correct source address
      //(There might be other VBus devices on the same bus).
      if (sourceAddress == 0x2211)
      {
        if (destinationAddress == 0x10)
        {
          return deltaSolCSPlus(buffer);
        }
        // if (destinationAddress == 0x15)
        // {
        //   //Serial.println("dest 15");
        //   // return deltaSolCSPlus(buffer);
        // }
      }
      else if (sourceAddress == 0x3271)
      {
        return resolConergyDT5(buffer);
      }
      else if (sourceAddress == 0x5611)
      {
        return resolDeltathermFK(buffer);
      }
      else if (sourceAddress == 0x4212)
      {
        return resolDeltaSolC(buffer);
      }
      else if (sourceAddress == 0x7311)
      {
        return resolDeltaSolM(buffer);
      }
      else if (sourceAddress == 0x4211)
      {
        return SKSC1_2(buffer);
      }
      else
      {
        return resolDefault(buffer);
      }
    }
  }
  else if (protocolVersion == 2)
  {
    if (buffer[15] == CalcCrc(buffer, 1, 14))
    {
      InjectSeptett(buffer, 8, 6);
      // 8 	ID of data point (low-byte)
      // 9 	ID of data point (high-byte)
      uint16_t dataPointId = ReadUInt16LE(&buffer[9]);
      // 10 	Value of data point (low-byte)
      // 11 	Value of data point
      // 12 	Value of data point
      // 13 	Value of data point (high-byte)
      uint32_t dataPointValue = ReadUInt32LE(&buffer[10]);

#if DEBUG
      Serial.print("Data point ID: ");
      Serial.println(dataPointId);
      Serial.print("Data point value: ");
      Serial.println(dataPointValue);
      Serial.print("Length: ");
      Serial.println(length);
#endif
    }
    else
    {
      Serial.println("CRC failed");
      return false;
    }
  }
  return false;
}

void VBUSDecoder::ExtractSeptett(unsigned char *buffer, int offset, int length)
{
  unsigned char septett = 0;
  for (int i = 0; i < length; i++)
  {
    if (buffer[offset + i] & 0x80)
    {
      buffer[offset + i] &= 0x7F;
      septett |= (1 << i);
    }
  }
  buffer[offset + length] = septett;
}

void VBUSDecoder::InjectSeptett(unsigned char *buffer, int offset, int length)
{
  unsigned char septett = buffer[offset + length];
  for (int i = 0; i < length; i++)
  {
    if (septett & (1 << i))
    {
      buffer[offset + i] |= 0x80;
    }
  }
  PrintHex8(&buffer[offset], length);
}

// CRC calculation
// From https://danielwippermann.github.io/resol-vbus/vbus-specification.html
unsigned char VBUSDecoder::CalcCrc(const unsigned char *buffer, int offset, int length)
{
  unsigned char crc = 0x7F;
  for (int i = 0; i < length; i++)
  {
    crc = (crc - buffer[offset + i]) & 0x7F;
  }
  return crc;
}

uint16_t VBUSDecoder::ReadUInt16LE(unsigned char *buffer)
{
  uint16_t Value = buffer[0];
  Value |= (((uint16_t)buffer[1]) << 8);
  return Value;
}

uint32_t VBUSDecoder::ReadUInt32LE(unsigned char *buffer)
{
  uint32_t Value = buffer[0];
  Value |= (((uint32_t)buffer[1]) << 8);
  Value |= (((uint32_t)buffer[2]) << 16);
  Value |= (((uint32_t)buffer[3]) << 24);
  return Value;
}

float VBUSDecoder::ReadTemp(unsigned char *buffer)
{
  int v;
  v = buffer[1] << 8 | buffer[0]; // bit shift 8 to left, bitwise OR
  if (buffer[1] == 0x00)
  {
    v = v & 0xFF;
  }

  if (buffer[1] == 0xFF)
    v = v - 0x10000;

  if (v == SENSORNOTCONNECTED)
    v = 0;

  return (float)((float)v * 0.1);
}

// Prints the hex values of the char array sent to it.
void VBUSDecoder::PrintHex8(unsigned char *data, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
#if DEBUG
  // Serial.print("0x");
  for (int i = 0; i < length; i++)
  {
    if (data[i] < 0x10)
    {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
}
