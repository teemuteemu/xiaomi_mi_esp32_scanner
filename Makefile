build:
	pio run

upload:
	pio run -t upload

monitor:
	pio device monitor

mosquitto_up:
	docker run \
		-it \
		-p 8883:8883 \
		-v $(PWD)/mosquitto/mosquitto.conf:/mosquitto/config/mosquitto.conf \
		-v $(PWD)/certs/ca.crt:/mosquitto/config/ca.crt \
		-v $(PWD)/certs/server.key:/mosquitto/config/server.key \
		-v $(PWD)/certs/server.crt:/mosquitto/config/server.crt \
		eclipse-mosquitto:2.0.11
