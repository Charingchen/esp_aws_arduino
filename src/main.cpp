#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// The MQTT topics that this device should publish/subscribe
#define TEMP_TOPIC   "esp32/temp"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

#define AWS_IOT_SHADOW_PUBLISH_TOPIC   "$aws/things/" THINGNAME "/shadow/update"
#define AWS_IOT_SHADOW_SUBSCRIBE_TOPIC "$aws/things/" THINGNAME "/shadow/update/delta"

int msgReceived = 0;
String rcvdPayload;
char sndPayloadOff[512];
char sndPayloadOn[512];

#define TH1 34
#define TH2 35
#define RELAY 33

const float th1_inv_beta = 0.0002569;
const float th2_inv_beta = 0.0002898;
const int average_num = 10;
const float adc_offset = 0.18;
WiFiClientSecure net = WiFiClientSecure(); 
MQTTClient client = MQTTClient(256);


void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  msgReceived = 1;
  rcvdPayload = payload;
}

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(1000);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SHADOW_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

float getTemp (int pin_name,float inv_beta)
{
  float average_read  = 0;
  for (int i = 0;i < average_num; i++){
    average_read += analogRead(pin_name);
  }
  average_read /= average_num;
  float voltage = average_read*3.3 / 4096 + adc_offset;
  Serial.print(pin_name);
  Serial.print('\t');
  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.print('\t');
  float r = (10000 * voltage) / (3.3-voltage);
  float temp = 1/(0.003354 + inv_beta * log(r/10000))-273.15;
  Serial.print("temp: ");
  Serial.println(temp);
  Serial.print('\n');
  return temp;
}

void publishMessage(float temp1, float temp2, String action)
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["action"] = action;
  doc["TH1 Temp"] = temp1;
  doc["TH2 Temp"] = temp2;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(TEMP_TOPIC, jsonBuffer);
}




void setup() {
  Serial.begin(9600);
  sprintf(sndPayloadOn,"{\"state\": {\"reported\":{\"status\": \"on\" }}}");
  sprintf(sndPayloadOff,"{\"state\":{\"reported\":{\"status\": \"off\" }}}");
  connectAWS();
  Serial.println("Setting Lamp Status to Off");
  client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOff);
  
  Serial.println("##############################################");
}

void loop() {
  if(msgReceived == 1)
    {
//      This code will run whenever a message is received on the SUBSCRIBE_TOPIC_NAME Topic
        delay(100);
        msgReceived = 0;
        Serial.print("Received Message:");
        Serial.println(rcvdPayload);
        StaticJsonDocument<200> sensor_doc;
        DeserializationError error_sensor = deserializeJson(sensor_doc, rcvdPayload);
        const char *sensor = sensor_doc["state"]["status"];
 
        Serial.print("AWS Says:");
        Serial.println(sensor); 
        
        if(strcmp(sensor, "on") == 0)
        {
         Serial.println("IF CONDITION");
         Serial.println("Turning Relay On");
         digitalWrite(RELAY, HIGH);
         client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOn);
        }
        else 
        {
         Serial.println("ELSE CONDITION");
         Serial.println("Turning RElay Off");
         digitalWrite(RELAY, LOW);
         client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOff);
        }
      Serial.println("##############################################");
    }

  float temp1 = getTemp(TH1,th1_inv_beta);
  float temp2 = getTemp(TH2,th2_inv_beta); 
  publishMessage(temp1,temp2,"update");

  client.loop();
  delay(5000);
}