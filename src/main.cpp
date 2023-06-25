#include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>
#include <InfluxDb.h> //https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino
#include "VBUSDecoder.h" //orginal from https://github.com/FatBeard/vbus-arduino-library
#include "credentials.h"

#define DEBUG

#define RXD2 22
#define TXD2 21
//#define LED 5

VBUSDecoder vb;

const char ssid[] = WIFI_SSID;
const char password[] = WIFI_PASSWD;

String chipid;

Ticker pushTimer;
// #define INTERVALTIME 300 // 300sec between updates
#define INTERVALTIME 60 // 300sec between updates
int timerFlag = 0;

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
    //digitalWrite(LED, !digitalRead(LED));
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
  vb.readSensor();

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

  Serial.print("Water Pump Status: ");
  Serial.println(vb.getP1Status());
  Serial.print("Water Pump Speed: ");
  Serial.println(vb.getP1Speed());
  Serial.print("Pump Hours Operation: ");
  Serial.println(vb.getP1OperatingHours());

  Serial.print("Water Pump 2 Status: ");
  Serial.println(vb.getP2Status());
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
    Serial.println("Influx temperature write error");
    return -1;
  }

  // Pump 1
  Point pump1("pump1");
  pump1.addTag("device", chipid);
  pump1.addField("status", vb.getP1Status());
  pump1.addField("speed", vb.getP1Speed());
  pump1.addField("operatinghours", vb.getP1OperatingHours());
  if (client.writePoint(pump1) == false)
  {
    Serial.println("Influx Pump 1 write error");
    return -1;
  }

  // Pump 2
  Point pump2("pump2");
  pump2.addTag("device", chipid);
  pump2.addField("status", vb.getP2Status());
  pump2.addField("speed", vb.getP2Speed());
  pump2.addField("operatinghours", vb.getP2OperatingHours());
  if (client.writePoint(pump2) == false)
  {
    Serial.println("Influx Pump 2 write error");
    return -1;
  }

  // system
  Point system("system");
  system.addTag("device", chipid);
  system.addField("alertstatus", vb.getAlertStatus());
  system.addField("scheme", vb.getScheme());
  system.addField("heatquantity", vb.getHeatQuantity());
  if (client.writePoint(system) == false)
  {
    Serial.println("Influx system write error");
    return -1;
  }


  return 0;
}

// ********* timer tick callback ******************
void pushTimerTick()
{
  timerFlag = 1;
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2, true);

  connectWifi();

  vb.initialise();

  uint64_t macAddress = ESP.getEfuseMac();
  uint64_t macAddressTrunc = macAddress << 40;
  chipid = macAddressTrunc >> 40;
  Serial.print("chipid = ");
  Serial.println(chipid);

  // Set update timer
  pushTimer.attach(INTERVALTIME, pushTimerTick);
  timerFlag = 1; // stuur meteen de eerste update

  Serial.println("Start Resol Interface");
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (timerFlag == 1)
    {
      if ((influxDbUpdate() != 0))
      {
        // error in server request
        delay(4000); // try after 4 sec
      }
      else
      {
        timerFlag = 0;
      }
    }
  }
  else
  {
    connectWifi();
  }
}