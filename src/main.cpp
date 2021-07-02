#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "../settings.h"

#define PIN_LED (2)
#define SCAN_TIME (5) // seconds

const char* MQTT_TOPIC = "midata";

extern const uint8_t ca_crt_start[] asm("_binary_certs_ca_crt_start");
extern const uint8_t ca_crt_end[] asm("_binary_certs_ca_crt_end");
extern const uint8_t client_crt_start[] asm("_binary_certs_client_crt_start");
extern const uint8_t client_crt_end[] asm("_binary_certs_client_crt_end");
extern const uint8_t client_key_start[] asm("_binary_certs_client_key_start");
extern const uint8_t client_key_end[] asm("_binary_certs_client_key_end");

boolean MQTT_READY = false;
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String currentDeviceAddress = advertisedDevice.getAddress().toString().c_str();

        for (int i=0; i<sizeof(miAddressList)/sizeof(miAddressList[0]); i++) {
            if (miAddressList[i].equalsIgnoreCase(currentDeviceAddress)) {
                if (advertisedDevice.haveServiceData()) {
                    String uuid(advertisedDevice.getServiceDataUUID().toString().c_str());

					
                    if (uuid.substring(4,8).equalsIgnoreCase("181a")) {
                        uint8_t cServiceData[100];
                        uint8_t* payloadPtr = advertisedDevice.getPayload();

            
                        for (int i = 0; i<advertisedDevice.getPayloadLength(); i++) {
                            cServiceData[i] = *(payloadPtr + i);
                        }

            
                        /* payload:
                         *
                         * bytes   
                         * 10 - 11  temperature int16
                         * 12       humidity percentage
                         * 13       battery percentage
                         * 14 - 15  battery mV
                         * 16       frame packet
                         */
            
                        char outputBuff[128];
                        char charValue[5] = {0,};
                        unsigned long value;
                        sprintf(charValue, "%02X%02X", cServiceData[10], cServiceData[11]);
                        value = strtol(charValue, 0, 16);
                        float temperature = (float)value/10;

                        sprintf(charValue, "%02X", cServiceData[12]);
                        value = strtol(charValue, 0, 16);
                        float humidity = (float)value;

                        sprintf(charValue, "%02X", cServiceData[13]);
                        value = strtol(charValue, 0, 16);
                        unsigned long int battery = value;

                        sprintf(charValue, "%02X", cServiceData[16]);
                        value = strtol(charValue, 0, 16);           

                        sprintf(outputBuff, "{\"device\":\"%s\",\"temperature\":%f,\"humidity\":%f,\"battery\":%lu,\"frame\":%lu}\n",
                                advertisedDevice.getAddress().toString().c_str(),
                                temperature,
                                humidity,
                                battery,
                                value);

                        // Serial.println(outputBuff);
                        mqttClient.publish(MQTT_TOPIC, outputBuff);
                    }
                }
            }
        }
    }
};

void bleTask(void* param) {
    while (!MQTT_READY) {
        vTaskDelay(200);
    }

    BLEDevice::init("");
    BLEScan* pBLEScan;
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  // less or equal setInterval value

    while (1) {
        BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME, false);
        pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    }
}

void wifiTask(void* param) {
    vTaskDelay(10);

    Serial.printf("\nConnecting to %s", ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500);
        Serial.print(".");
    }
    wifiClient.setCACert((const char*)ca_crt_start);
    wifiClient.setPrivateKey((const char*)client_key_start);	// for client verification
    wifiClient.setCertificate((const char*)client_crt_start); // for client verification

    Serial.println(WiFi.localIP());
    mqttClient.setServer(mqttBroker, mqttPort);
  
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");

        if (mqttClient.connect("ESP32")) {
            Serial.println("connected");
            MQTT_READY = true;
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            vTaskDelay(5000);
        }
    }

    while (1) {}
}

void setup() {
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(115200);
    Serial.println("moi...");
    xTaskCreate(wifiTask, "WiFi", 10000, NULL, 1, NULL);
    xTaskCreate(bleTask, "BLE", 10000, NULL, 1, NULL);
}

void loop() {
    int lowDelay = MQTT_READY ? 3000 : 100;
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_LED, LOW);
    delay(lowDelay);
}
