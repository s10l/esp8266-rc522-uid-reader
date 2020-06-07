#include <Arduino.h>
#include <ArduinoOTA.h>

unsigned long wifiTimeout = 60 * 1000; // wifi timeout before restart the ESP
String deviceId;                       // the deviceId is derived from the hostname

void enableOTA();
void enableWiFi();
void h_setup();
void h_loop();

void setup()
{
    // initialize serial:
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    enableWiFi();
    enableOTA();

    deviceId = WiFi.hostname();
    deviceId.toLowerCase();

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address:   ");
    Serial.println(WiFi.localIP());
    Serial.print("DNS address:  ");
    Serial.println(WiFi.dnsIP());
    Serial.print("Device id:    ");
    Serial.println(deviceId);
    Serial.println();

    h_setup();

    Serial.println();
    Serial.println("Setup completed");
    Serial.println();
}

void loop()
{
    enableWiFi();
    ArduinoOTA.handle();
    h_loop();
}

void enableWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        // ssid and password were previously set by another sketch
        Serial.println("Attempting to connect to WPA network");

#ifdef WIFI_SSID
#ifdef WIFI_PASS
        Serial.println("WIFI_SSID and WIFI_PASS found!");
        WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif
#endif

        unsigned long wifiStart = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(50);
            Serial.print(".");

            if (millis() - wifiStart > wifiTimeout)
            {
                ESP.restart();
            }
        }

        Serial.println();
        Serial.println();

        Serial.println("WiFi...       ok");
    }
}

void enableOTA()
{
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
            type = "sketch";
        }
        else
        { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
        {
            Serial.println("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            Serial.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            Serial.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            Serial.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            Serial.println("End Failed");
        }
    });

    Serial.print("OTA...        ");
    ArduinoOTA.begin();
    Serial.println("ok");
}