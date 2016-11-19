#include <Homie.h>
#include <DHT.h>

#define FW_NAME		"raton-dht"
#define FW_VERSION	"0.0.2"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

#define DHT_PIN       D7
#define DHT_TYPE      DHT22
#define PUB_INTERVAL  60

HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");
HomieNode batteryNode("battery", "battery");

DHT dht(DHT_PIN, DHT_TYPE);

ADC_MODE(ADC_VCC);
unsigned long lastPublish = 0;

void setupHandler() {
	temperatureNode.setProperty("unit").send("c");
	humidityNode.setProperty("unit").send("%");
	batteryNode.setProperty("unit").send("V");

	dht.begin();
}

void loopHandler() {
	if (millis() - lastPublish >= 10 * 1000UL) {
		float t = dht.readTemperature();
		float h = dht.readHumidity();
		float v = ESP.getVcc() / 1000.0f;

		Serial << "t = " << t << "°C / h = " << h << " % /  v = " << v  << "V" << endl;
		lastPublish = millis();
	}
}

void setup() {
	Serial.begin(115200);

	Homie_setFirmware(FW_NAME, FW_VERSION);

	Homie.setSetupFunction(setupHandler);
	Homie.setLoopFunction(loopHandler);

	Homie.disableLedFeedback(); 
	Homie.setup();
}

void loop() {
	Homie.loop();
}
