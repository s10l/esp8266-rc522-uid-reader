#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <arduino_secrets.h>
#include <SPI.h>
#include <MFRC522.h>
#include "esp8266-hive.h"

#define SS_PIN 4  //D2
#define RST_PIN 5 //D1

const char mqtt_server[] = SECRET_HOST; // dns (not mdns) or ip of mqtt host
const int mqtt_port = 1883;             // port, default 1883

// the following variable are used for mqtt transport
IPAddress mqtt_address;        // the ip address of mqtt_server
char mqtt_message_payload[32]; // the mqtt message payload
char mqtt_topic[32];           // the mqtt topic that will be used, we derivate the topic
                               // from mac adresse and gpio

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long deadTime = 1500; // deadTime after we rise to high

WiFiClient espClient;
PubSubClient client(espClient);

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

unsigned long mqttTimeout = 5 * 1000;
void enableMFRC522();
void mqttHandle();
void mqttPublish(String id);

void h_setup()
{
  enableMFRC522();

  // resolve mqtt address
  WiFi.hostByName(mqtt_server, mqtt_address);

  // set topic
  String topic = deviceId + "/rfid";
  topic.toLowerCase();
  topic.toCharArray(mqtt_topic, 32);

  Serial.println("");
  Serial.println("MQTT");
  Serial.print("address:      ");
  Serial.print(mqtt_server);
  Serial.print(" (");
  Serial.print(mqtt_address);
  Serial.println(")");
  Serial.print("Port:         ");
  Serial.println(mqtt_port);
  Serial.print("Topic:        ");
  Serial.println(mqtt_topic);
}

void h_loop()
{
  mqttHandle();

  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  String content = "";
  // byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? ":0" : ":"));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();

  mqttPublish(content.substring(1));

  delay(deadTime);
}

void mqttHandle()
{
  if (!client.connected())
  {
    unsigned long mqttStart = millis();

    // Loop until we're reconnected
    while (!client.connected())
    {
      // resolve mqtt address
      WiFi.hostByName(mqtt_server, mqtt_address);
      client.setServer(mqtt_address, mqtt_port);

      Serial.print("Attempting MQTT connection... ");
      Serial.print(mqtt_server);
      Serial.print(" (");
      Serial.print(mqtt_address);
      Serial.println(")");

      // Create a random client ID
      // TODO maybe we should use our hostname
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);

      // Attempt to connect
      if (client.connect(clientId.c_str()))
      {
        Serial.println("connected");
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 1 seconds");

        // Wait a seconds before retrying
        delay(500);
      }
    }

    if (millis() - mqttStart > mqttTimeout)
    {
      ESP.restart();
    }
  }
}

void mqttPublish(String id)
{
  Serial.print("> ");
  Serial.print(mqtt_topic);
  Serial.print(" ");
  Serial.println(id);

  //  snprintf(mqtt_message_payload, 32, "%s", id);
  id.toCharArray(mqtt_message_payload, id.length() + 1);
  client.publish(mqtt_topic, mqtt_message_payload);
}

void enableMFRC522()
{
  // Initiate  SPI bus
  Serial.print("SPI...        ");
  SPI.begin();
  Serial.println("ok");

  delay(500);

  // Initiate MFRC522
  Serial.print("MFRC522...    ");
  mfrc522.PCD_Init();
  Serial.println("ok");

  yield();
}
