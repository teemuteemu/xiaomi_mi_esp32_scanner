#include <Arduino.h>
#include <sstream>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEUUID.h>
#include <BLEAdvertisedDevice.h>

#include "../settings.h"

#define PIN_LED (2)
#define SCAN_TIME (5) // seconds
boolean METRIC = true; //Set true for metric system; false for imperial

BLEScan* pBLEScan;

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

            sprintf(outputBuff, "{\"device\":\"%s\",\"temperature\":%f,\"humidity\":%f,\"battery\":%lu}\n",
                    advertisedDevice.getAddress().toString().c_str(),
                    temperature,
                    humidity,
                    value);
            Serial.println(outputBuff);
					}
				}
			}
		}
	}
};

void setup() {
	pinMode(PIN_LED, OUTPUT);
	Serial.begin(115200);
	Serial.println("moi...");

	BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
	BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME, false);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

	digitalWrite(PIN_LED, HIGH);
	delay(100);
	digitalWrite(PIN_LED, LOW);
	delay(100);
}
