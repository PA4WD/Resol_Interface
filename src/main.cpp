#include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>
#include <InfluxDb.h>    //https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino
#include "VBUSDecoder.h" //orginal from https://github.com/FatBeard/vbus-arduino-library
#include "credentials.h"

// http://danielwippermann.github.io/resol-vbus/#/
// https://github.com/danielwippermann/resol-vbus-c

//#define DEBUG

#define RXD2 22
#define TXD2 21
// #define LED 5

VBUSDecoder vb;

const char ssid[] = WIFI_SSID;
const char password[] = WIFI_PASSWD;

String chipid;

Ticker pushTimer;
// #define INTERVALTIME 300 // 300sec between updates
#define INTERVALTIME 60 // 300sec between updates
int timerFlag = 0;
bool firstRead = false;

void connectWifi()
{
  // attempt to connect to Wifi network:
  WiFi.mode(WIFI_STA);
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  // Connect to WEP/WPA/WPA2 network:
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    // digitalWrite(LED, !digitalRead(LED));
  }

  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.print(F("IPv4: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("IPv6: "));
  Serial.println(WiFi.localIPv6());
}

int influxDbUpdate()
{
  // vb.readSensor();

#ifdef DEBUG
  Serial.println("");
  Serial.println("*** Fresh Read ***");
  Serial.print("Collector Temp: ");
  Serial.println(vb.getS1Temp());
  Serial.print("Top Tank Temp: ");
  Serial.println(vb.getS3Temp());
  Serial.print("Bottom Tank Temp: ");
  Serial.println(vb.getS2Temp());
  Serial.print("Sensor 4 Temp: ");
  Serial.println(vb.getS4Temp());

  // Serial.print("Water Pump Status: ");
  // Serial.println(vb.getP1Status());
  Serial.print("Water Pump Speed: ");
  Serial.println(vb.getP1Speed());
  Serial.print("Pump Hours Operation: ");
  Serial.println(vb.getP1OperatingHours());

  // Serial.print("Water Pump 2 Status: ");
  // Serial.println(vb.getP2Status());
  Serial.print("Water Pump 2 Speed: ");
  Serial.println(vb.getP2Speed());
  Serial.print("Pump 2 Hours Operation: ");
  Serial.println(vb.getP2OperatingHours());

  Serial.print("System Alert Status: ");
  Serial.println(vb.getAlertStatus());
  Serial.print("Scheme: ");
  Serial.println(vb.getScheme());
  Serial.print("System Time: ");
  Serial.println(vb.getSystemTime());

  Serial.print("Heat Quantity: ");
  Serial.println(vb.getHeatQuantity());
  Serial.println("*** End Read ***");
#endif

  InfluxDBClient client(INFLUXDB_HOST, INFLUXDB_DATABASE);

  // Temperature
  Point temperature("temperature");
  temperature.addTag("device", chipid);
  temperature.addField("sensor1", vb.getS1Temp());
  temperature.addField("sensor2", vb.getS2Temp());
  temperature.addField("sensor3", vb.getS3Temp());
  temperature.addField("sensor4", vb.getS4Temp());
  if (client.writePoint(temperature) == false)
  {
    Serial.println(F("Influx temperature write error"));
    return -1;
  }

  // Pump 1
  Point pump1("pump1");
  pump1.addTag("device", chipid);
  pump1.addField("speed", vb.getP1Speed());
  pump1.addField("operatinghours", vb.getP1OperatingHours());
  if (client.writePoint(pump1) == false)
  {
    Serial.println(F("Influx Pump 1 write error"));
    return -1;
  }

  // Pump 2
  Point pump2("pump2");
  pump2.addTag("device", chipid);
  pump2.addField("speed", vb.getP2Speed());
  pump2.addField("operatinghours", vb.getP2OperatingHours());
  if (client.writePoint(pump2) == false)
  {
    Serial.println(F("Influx Pump 2 write error"));
    return -1;
  }

  // system
  Point system("system");
  system.addTag("device", chipid);
  //system.addField("alertstatus", vb.getAlertStatus());
  //system.addField("scheme", vb.getScheme());
  system.addField("heatquantity", vb.getHeatQuantity());
  if (client.writePoint(system) == false)
  {
    Serial.println(F("Influx system write error"));
    return -1;
  }

  return 0;
}

// ********* timer tick callback ******************
void pushTimerTick()
{
  timerFlag = 1;
}

unsigned char buffer[128];
u_int8_t bufferIdx;

void Serial_Data_Receiver()
{
  char incomingByte;
  while (Serial1.available() > 0)
  {
    incomingByte = Serial1.read();

    if (incomingByte == 0xAA)
    {
      if (bufferIdx > 1)
      {
        //unsigned char buffer2[128];
        //memcpy(buffer2, buffer, bufferIdx);

#ifdef DEBUG
        Serial.println(F("--------------------------------------"));
        for (int i = 0; i < bufferIdx; i++)
        {
          Serial.print(buffer[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
#endif

        if (vb.messageDecode(buffer, bufferIdx))
        {
          firstRead = true;

#ifdef DEBUG
          Serial.print(F("Collector Temp: "));
          Serial.println(vb.getS1Temp());
          Serial.print(F("Top Tank Temp: "));
          Serial.println(vb.getS2Temp());
          Serial.print(F("Bottom Tank Temp: "));
          Serial.println(vb.getS3Temp());
          Serial.print(F("Sensor 4 Temp: "));
          Serial.println(vb.getS4Temp());

          // Serial.print(F("Water Pump Status: "));
          // Serial.println(vb.getP1Status());
          Serial.print(F("Water Pump Speed: "));
          Serial.println(vb.getP1Speed());
          Serial.print(F("Pump Hours Operation: "));
          Serial.println(vb.getP1OperatingHours());

          // Serial.print(F("Water Pump 2 Status: "));
          // Serial.println(vb.getP2Status());
          Serial.print(F("Water Pump 2 Speed: "));
          Serial.println(vb.getP2Speed());
          Serial.print(F("Pump 2 Hours Operation: "));
          Serial.println(vb.getP2OperatingHours());

          Serial.print(F("System Alert Status: "));
          Serial.println(vb.getAlertStatus());
          Serial.print(F("Scheme: "));
          Serial.println(vb.getScheme());
          Serial.print(F("System Time: "));
          Serial.println(vb.getSystemTime());

          Serial.print(F("Heat Quantity: "));
          Serial.println(vb.getHeatQuantity());
#endif
        }
      }

      // SYNC byte
      memset(buffer, 0x00, sizeof(buffer));
      bufferIdx = 0;
      buffer[bufferIdx] = incomingByte;
    }
    else
    {
      bufferIdx++;
      buffer[bufferIdx] = incomingByte;
    }
  }
}

void Serial_Error(hardwareSerial_error_t error)
{
  Serial.print(F("Serial error :"));
  switch (error)
  {
  case UART_NO_ERROR:
    Serial.println(F("UART_NO_ERROR"));
    break;
  case UART_BREAK_ERROR:
    Serial.println(F("UART_BREAK_ERROR"));
    break;
  case UART_BUFFER_FULL_ERROR:
    Serial.println(F("UART_BUFFER_FULL_ERROR"));
    break;
  case UART_FIFO_OVF_ERROR:
    Serial.println(F("UART_FIFO_OVF_ERROR"));
    break;
  case UART_FRAME_ERROR:
    Serial.println(F("UART_FRAME_ERROR"));
    break;
  case UART_PARITY_ERROR:
    Serial.println(F("UART_PARITY_ERROR"));
    break;
  default:
    Serial.println(F("Unknown error"));
    break;
  }
}

void setup()
{
  Serial.begin(9600);
  // Serial1.setRxBufferSize(512);
  // Serial1.setRxTimeout(192);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2, true);

  connectWifi();

  // Serial1.onReceive(Serial_Data_Processor, true);
  Serial1.onReceiveError(Serial_Error);

  uint64_t macAddress = ESP.getEfuseMac();
  uint64_t macAddressTrunc = macAddress << 40;
  chipid = macAddressTrunc >> 40;
  Serial.print(F("chipid = "));
  Serial.println(chipid);

  // Set update timer
  pushTimer.attach(INTERVALTIME, pushTimerTick);
  timerFlag = 1; // stuur meteen de eerste update

  Serial.println(F("Start Resol Interface"));
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial_Data_Receiver();

    if (timerFlag == 1 && firstRead)
    {
      influxDbUpdate();
      timerFlag = 0;

      // if ((influxDbUpdate() != 0))
      // {
      //   // error in server request
      //   delay(4000); // try after 4 sec
      // }
      // else
      // {
      //   timerFlag = 0;
      // }
    }
  }
  else
  {
    connectWifi();
  }
}