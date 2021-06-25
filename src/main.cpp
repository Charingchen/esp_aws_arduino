#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// The MQTT topics that this device should publish/subscribe
#define STATUS_TOPIC   "esp32/temp"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

#define AWS_IOT_SHADOW_PUBLISH_TOPIC   "$aws/things/" THINGNAME "/shadow/update"
#define AWS_IOT_SHADOW_SUBSCRIBE_TOPIC "$aws/things/" THINGNAME "/shadow/update/delta"

int msgReceived = 0;

int button_count = 0;
volatile byte button_state = LOW;

String rcvdPayload;
char sndPayloadOff[512];
char sndPayloadOn[512];

#define LIGHT 34
#define TH2 35
#define RELAY 33
#define MOTION 32
#define PUSH_BUTTON 27

const float th1_inv_beta = 0.0002569;
const float th2_inv_beta = 0.0002898;
const int average_num = 10;
const float adc_offset = 0.18;
WiFiClientSecure net = WiFiClientSecure(); 
MQTTClient client = MQTTClient(256);

struct SwitchData
{
  bool init = false;
  float light;
  float temp;
  bool motion;
  int button_state;
  int button_count;
};

SwitchData switch1;

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
  // Serial.print(pin_name);
  // Serial.print('\t');
  // Serial.print("Voltage: ");
  // Serial.print(voltage);
  // Serial.print('\t');
  float r = (10000 * voltage) / (3.3-voltage);
  float temp = 1/(0.003354 + inv_beta * log(r/10000))-273.15;
  // Serial.print("temp: ");
  // Serial.println(temp);
  // Serial.print('\n');
  return temp;
}

void publishMessage(SwitchData data)
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();

  doc["Ambient Light Sensor"] = data.light;
  doc["PCB Temp Sensor"] = data.temp;
  doc["motion"] = data.motion;
  doc["button state"] = data.button_state;
  doc["button count"] = data.button_count;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  Serial.println(jsonBuffer);
  client.publish(STATUS_TOPIC, jsonBuffer);
}

float amb_light_read (){
  float raw_amb_read = analogRead(LIGHT);
  return raw_amb_read;
}

void IRAM_ATTR isr(){
  button_count += 1;
  button_state = !button_state;
}

void data_update (){
  float light_read = amb_light_read();
  float temp_read = getTemp(TH2,th2_inv_beta); 
  bool motion_read = digitalRead(MOTION);

  // Check if first time update data
  if (!switch1.init){
      switch1.init = true;
      switch1.light = light_read;
      switch1.temp = temp_read;
      switch1.motion = motion_read;
      switch1.button_state = button_state;
      switch1.button_count = button_count;
      
  }
  else{
    if (light_read > switch1.light * 1.05 || light_read < switch1.light* 0.95|| switch1.motion != motion_read 
    || temp_read > switch1.temp * 1.05 || temp_read < switch1.temp * 0.95 || switch1.button_state != button_state) {
 
      switch1.light = light_read;
      switch1.temp = temp_read;
      switch1.motion = motion_read;
      switch1.button_state = button_state;
      switch1.button_count = button_count;
    }
  }

}

void setup() {
  Serial.begin(9600);
  sprintf(sndPayloadOn,"{\"state\": {\"reported\":{\"status\": \"on\" }}}");
  sprintf(sndPayloadOff,"{\"state\":{\"reported\":{\"status\": \"off\" }}}");
  connectAWS();
  Serial.println("Setting Lamp Status to Off");
  client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOff);
  pinMode(RELAY,OUTPUT);
  pinMode(MOTION,INPUT);
  pinMode(PUSH_BUTTON,INPUT);
  

  // Create interrupts for the push the
  attachInterrupt(PUSH_BUTTON,isr,FALLING);
  
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
         button_state = HIGH;
        //  client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOn);
        }
        else 
        {
         Serial.println("ELSE CONDITION");
         Serial.println("Turning RElay Off");
         button_state = LOW;
        //  client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOff);
        }
      Serial.println("##############################################");
    }
  
  // int reading = digitalRead(PUSH_BUTTON);
  // digitalWrite(RELAY,button_state);
  // StaticJsonDocument<200> doc;
  // doc["time"] = millis();
  // doc["push button"] = reading;
  // doc["count"] = button_count;
  // doc["button state"] = button_state;
  // char jsonBuffer[512];
  // serializeJson(doc, jsonBuffer); // print to client
  // Serial.println(jsonBuffer);
  // client.publish(STATUS_TOPIC, jsonBuffer);

 
// toggle relay base on its state
 digitalWrite(RELAY,button_state);
 // Update shadows according to the button state
 if (button_state != switch1.button_state){
    if (button_state == HIGH) {
    client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOn);
 }
    else client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOff);
 }
 
  data_update();
  publishMessage(switch1);

  client.loop();
  delay(500);
}