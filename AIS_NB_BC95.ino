#include "AIS_NB_BC95.h"
#include <ArduinoJson.h>
#include <BH1750FVI.h>
#include "DHT.h"

#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);


DHT dht(DHTPIN, DHTTYPE);
String apnName = "devkit.nb";

//ข้อมูลserver
String server_ip = "43.229.79.125";
String server_port = "9999";

//ข้อมูลตัวเครื่อง
String serial_number = "1199";
String mac_address = "00:00:00:00:99";


AIS_NB_BC95 AISnb;

const long interval = 4000;  //millisecond เซตเวลาส่งข้อมูล
unsigned long previous_millis = 0;

long cnt = 0;
void setup()
{ 
  AISnb.debug = true;
  Serial.begin(9600);
  LightSensor.begin();
  dht.begin();
  
  AISnb.setupDevice(server_port);
  String ip1 = AISnb.getDeviceIP();  
  delay(1000);
  
  pingRESP pingR = AISnb.pingIP(server_ip);
  previous_millis = millis();
}




void loop()
{ 
  unsigned long current_millis = millis();
  if (current_millis - previous_millis >= interval)
    {
      cnt++;

      uint16_t lux = LightSensor.GetLightIntensity(); // ดึงค่าแสง
      float humidity = dht.readHumidity(); // ดึงค่าความชื้น
      float temperature = dht.readTemperature(); // ดึงค่าอุณหภูมิ
      
      String json_sting_data = data_to_json(serial_number,mac_address,lux,temperature,humidity);
      Serial.println(json_sting_data);
      
      UDPSend udp = AISnb.sendUDPmsgStr(server_ip, server_port, json_sting_data );
      
      previous_millis = current_millis;
    }
  UDPReceive resp = AISnb.waitResponse();
     
}


String data_to_json(String serial_number,String mac_address,float light,float temperature,float humidity)
{
  DynamicJsonDocument doc(2048);
  doc["serial_number"] = serial_number;
  doc["mac_address"] = mac_address;

  DynamicJsonDocument obj_light(200);
  obj_light["light"] = light;

  DynamicJsonDocument obj_temperature(200);
  obj_temperature["temperature"] = temperature;

  DynamicJsonDocument obj_humidity(200);
  obj_humidity["humidity"] = humidity;



  JsonArray sensors = doc.createNestedArray("sensors");
  sensors.add(obj_light);
  sensors.add(obj_temperature);
  sensors.add(obj_humidity);


  String output;
  serializeJson(doc, output);
  return output;
  
//  Serial.println(output);
}
