# Xiaomi Mi thermometer -> _ESP32_ -> MQTT

1. `cp settings.example.h settings.h`
2. Set your MI's MAC addresses, MQTT broker, wifi SSID & password
3. generate certs (give them proper CNs) `sh gen_certs.sh`
3. `pio run -t upload`
