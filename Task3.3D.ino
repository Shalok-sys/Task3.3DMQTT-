#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>

char ssid[] = "builder";    // your network SSID (name)
char pass[] = "splicing";    // your network password 

const int trig = 9;
const int echo = 10;
const int led = 8;

bool wave = false;

long duration;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "broker.emqx.io"; //IP address of the EMQX broker which is public in this case
int port = 1883;
const char subscribe_topic[]  = "SIT210/wave";
const char publish_topic[]  = "SIT210/wave";
const char publish_topic2[]  = "SIT210/pat";
const char subscribe_topic2[]  = "SIT210/pat";

String msg;

void setup() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(led, OUTPUT);
  // Create serial connection and wait for it to become available.
  Serial.begin(9600);
  // Connect to WiFi
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker.");

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");

  Serial.print("Subscribing to topics: ");
  Serial.print(subscribe_topic2);
  Serial.print(" and ");
  Serial.println(subscribe_topic);

  // subscribe to both the topic
  mqttClient.subscribe(subscribe_topic);
  mqttClient.subscribe(subscribe_topic2);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Waiting for messages on topics: ");
  Serial.print(subscribe_topic2);
  Serial.print(" and ");
  Serial.println(subscribe_topic);
}

void loop() {
  // Sender client - Ultrasonic Sensor (publisher)
  generate_ultrasound_wave();
  read_dis_n_dur();
  if(wave){
      mqttClient.beginMessage(publish_topic);
      mqttClient.print("Shalok");
      mqttClient.endMessage();
      Serial.println("Message sent after confirming the waves were detected");
  }
  // Publisher system for SIT210/pat
  mqttClient.beginMessage(publish_topic2);
  mqttClient.print("Patupdate");
  mqttClient.endMessage();


  // Receiver client - LED (subscriber)
  int messageSize = mqttClient.parseMessage(); // messagesize tell the size of the message received in bytes
  if (messageSize) {
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    // use the Stream interface to print the contents
    while (mqttClient.available()) { //mqttClient.available() is a function that returns the number of bytes available to read from the MQTT client. If there are no bytes available, it returns 0.
      msg += (char)mqttClient.read();
    }
    if(msg.indexOf("Shalok") != 1){
      blink_thrice();
    }
    if(msg.indexOf("Patupdate") != 1){
      pat_led();
    }

    Serial.println();
  }
  // send message, the Print interface can be used to set the message contents
  delay(3000);
}

void generate_ultrasound_wave(){
  digitalWrite(trig,LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
}
void read_dis_n_dur(){
  duration = pulseIn(echo,HIGH);
  if(duration ==  0){ // returns 0 if no complete pulse was received within the timeout.
    wave = false;
  }
  else{
    wave = true;
  }
}
void blink_thrice(){
      digitalWrite(led, HIGH);
      delay(1000);
      digitalWrite(led, LOW);
      delay(500);
      digitalWrite(led, HIGH);
      delay(1000);
      digitalWrite(led, LOW);
      delay(500);
      digitalWrite(led, HIGH);
      delay(1000);
      digitalWrite(led, LOW);
      delay(500);
}

void pat_led(){
  for(int i = 100; i<=1000; i+=100){
      digitalWrite(led, HIGH);
      delay(i);
      digitalWrite(led, LOW);
      delay(i/2);
  }
}
