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
bool first_temp = true;
float prev_temp = 0;
float prev_light = 0;
bool prev_motion = false;
int button_state;
int lastButtonState = LOW;
int relay_state = LOW;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

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

void publishMessage(float temp1, float temp2, String action, bool motion)
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["action"] = action;
  doc["Ambient Light Sensor"] = temp1;
  doc["pcb temp Sensor"] = temp2;
  doc["motion"] = motion;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  Serial.println(jsonBuffer);
  client.publish(STATUS_TOPIC, jsonBuffer);
}

float amb_light_read (){
  float raw_amb_read = analogRead(LIGHT);
  return raw_amb_read;
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
  
  relay_state = LOW;
  digitalWrite(RELAY, LOW);

  
  Serial.println("##############################################");
}

void button_detect (){
  int reading = digitalRead(PUSH_BUTTON);
  // If the switch changed, due to noise or pressing:

  // Debug note: change the reading to push to aws to debug.
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != button_state) {
      button_state = reading;

      // only toggle the LED if the new button state is HIGH
      if (button_state == LOW) {
        relay_state = !relay_state;
      }

      if(relay_state == HIGH){
      Serial.println("PUSHED Button");
      Serial.println("Turning Relay On");
      digitalWrite(RELAY, HIGH);
      // client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOn);
      }
      else{
        Serial.println("PUSHED Button again");
        Serial.println("Turning Relay Off");
        digitalWrite(RELAY, LOW);
        // client.publish(AWS_IOT_SHADOW_PUBLISH_TOPIC, sndPayloadOff);
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
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
  
  button_detect();

  float light = amb_light_read();
  float temp2 = getTemp(TH2,th2_inv_beta); 
  bool reading_motion = digitalRead(MOTION);
  // Check the temp and update if it different than before
  if (first_temp){
      publishMessage(light,temp2,"initial",reading_motion);
      first_temp = false;
      prev_light = light;
      prev_temp = temp2;
      prev_motion = reading_motion;
  }
  else{
    if (light > prev_light * 1.05 || light < prev_light * 0.95|| prev_motion != reading_motion 
    || temp2 > prev_temp * 1.05 || temp2 < prev_temp * 0.95) {
      publishMessage(light,temp2,"update",reading_motion);
      prev_temp = temp2;
      prev_light = light;
      prev_motion = reading_motion;
    }
  }
    

  client.loop();
  delay(500);
}