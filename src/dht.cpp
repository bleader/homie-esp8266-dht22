#include <Homie.h>
#include <DHT.h>

#define FW_NAME		"raton-dht"
#define FW_VERSION	"0.0.3"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

#define DHT_PIN       D7
#define DHT_TYPE      DHT22

HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");
HomieNode batteryNode("battery", "battery");

DHT dht(DHT_PIN, DHT_TYPE);

ADC_MODE(ADC_VCC);
bool published = false;
uint32_t connected_at;

void setupHandler() {
	temperatureNode.setProperty("unit").send("c");
	humidityNode.setProperty("unit").send("%");
	batteryNode.setProperty("unit").send("V");

	dht.begin();
}

void loopHandler() {
	/* make sure not to send anything until mqtt is connected */
	if (Homie.isConnected() && (!published)) {
		float t = dht.readTemperature();
		float h = dht.readHumidity();
		float v = ESP.getVcc() / 1000.0f;

		Serial << "t = " << t << "°C / h = " << h << " % /  v = " << v  << "V" << endl;
		if (!isnan(v)) {
			batteryNode.setProperty("relative").send(String(h));
		}
		if (!isnan(h))
			humidityNode.setProperty("relative").send(String(h));
		if (!isnan(t))
			temperatureNode.setProperty("degrees").send(String(t));
		published = true;
		Homie.prepareToSleep();
	}
}

void onHomieEvent(const HomieEvent& event) {
	switch(event.type) {
		case HomieEventType::READY_TO_SLEEP:
			Serial << "Ready to sleep" << endl;
			ESP.deepSleep(10 * 1000000); /* 10min */
			break;
		case HomieEventType::MQTT_CONNECTED:
			connected_at = millis();
			break;
	}
}

void setup() {
	published = false;

	Serial.begin(115200);
	Serial.println();
	Serial.println();

	Homie_setFirmware(FW_NAME, FW_VERSION);

	Homie.setSetupFunction(setupHandler);
	Homie.setLoopFunction(loopHandler);

	Homie.disableResetTrigger();
	Homie.disableLedFeedback(); 
	Homie.onEvent(onHomieEvent);
	Homie.setup();
}

void loop() {
	Homie.loop();
}
