#include <Arduino.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#define   MESH_PREFIX      "esp_mesh"
#define   MESH_PASSWORD    "123456789"
#define   MESH_PORT        5555

#define   TEST_INTERVAL    5000

#define   STATION_SSID     "PraiseTheSun"
#define   STATION_PASSWORD "jebacjaslo"

#define HOSTNAME "Gateway"

const char *alive = "gate/alive";
const char *report = "gate/report";

// Prototypes
void received_callback(const uint32_t &from, const String &msg);
void mqtt_callback(char *topic, byte *payload, unsigned int length);

IPAddress getlocal_ip();

IPAddress my_ip(0, 0, 0, 0);
IPAddress mqtt_broker(192, 168, 31, 22);

painlessMesh mesh;
WiFiClient wifi;
PubSubClient mqtt(mqtt_broker, 1883, mqtt_callback, wifi);
uint32_t prev_millis = 0;
uint32_t round_trip_time = 0;
void setup() {
	Serial.begin(9600);

	// mesh.setDebugMsgTypes(ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE);   // all types on
	// mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

	// Channel set to 6. Make sure to use the same channel for your mesh and for you other
	// network (STATION_SSID)
	mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA);
	mesh.onReceive(&received_callback);

	mesh.stationManual(STATION_SSID, STATION_PASSWORD);
	mesh.setHostname(HOSTNAME);

	// Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
	mesh.setRoot(true);
	// This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
	mesh.setContainsRoot(true);
}

void loop() {
	mesh.update();
	mqtt.loop();

	if (my_ip != getlocal_ip()) {
		my_ip = getlocal_ip();

		if (mqtt.connect("gate"))
			mqtt.publish(alive, "Ready!");        
	}

    uint32_t curr_millis = millis();
    if (curr_millis - prev_millis > TEST_INTERVAL) {
        prev_millis = curr_millis;
        round_trip_time = curr_millis;
        mesh.sendBroadcast("Round trip time test");
    }
}

void received_callback(const uint32_t &from, const String &msg) {
	msg += String("\t");
    msg += String(millis() - round_trip_time);
    mqtt.publish(report, msg.c_str());
}

void mqtt_callback(char *topic, uint8_t *payload, unsigned int length) {
}

IPAddress getlocal_ip() {
	return IPAddress(mesh.getStationIP());
}
