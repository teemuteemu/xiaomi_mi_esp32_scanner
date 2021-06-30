#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEUUID.h>
#include <BLEAdvertisedDevice.h>

#define PIN_LED (2)
#define SCAN_TIME (5) // seconds

BLEScan* pBLEScan;

// Thermometers
String addressList[] = {
	"a4:c1:38:fd:d0:41",
	"a4:c1:38:68:6d:e5",
	"a4:c1:38:f0:bc:34"
};

class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
	void onResult(BLEAdvertisedDevice advertisedDevice) {
		String currentDeviceAddress = advertisedDevice.getAddress().toString().c_str();

		for (int i=0; i<sizeof(addressList)/sizeof(addressList[0]); i++) {
			if (addressList[i].equalsIgnoreCase(currentDeviceAddress)) {
				Serial.println(currentDeviceAddress);

				if (advertisedDevice.haveServiceData()) {
					String uuid(advertisedDevice.getServiceDataUUID().toString().c_str());

					if (uuid.substring(4,8).equalsIgnoreCase("181a")) {
						Serial.printf("%d - %d\n", advertisedDevice.getPayloadLength(), advertisedDevice.getPayload());
						/*
						std::string data = advertisedDevice.getServiceData();

						for (int j=0; j<data.length()-1; j++) {
							int val = atoi(data.substr(j, j+1).c_str());
							Serial.printf("%d", val);
						}
						Serial.println("");
						*/
						// int temperature = atoi(data.substr(0, 2).c_str());
						// int humidity = atoi(data.substr(2, 3).c_str());
						// int battery = atoi(data.substr(26, 30).c_str());

						// Serial.printf("temp: %d, hum: %d, batt: %d\n", temperature, humidity, battery);
					}
					// Serial.println(uuid.substring(4,10).c_str());
					// String data = advertisedDevice.getServiceData();
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
	/*
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
	getDevices(&foundDevices);
	*/
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

	digitalWrite(PIN_LED, HIGH);
	delay(100);
	digitalWrite(PIN_LED, LOW);
	delay(100);
}
