#include <WiFi.h>
#include <PubSubClient.h>

#define SSID "<sumo>"               // Replace with your WiFi SSID
#define WIFI_PWD "<tee5star1>"        // Replace with your WiFi password
#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT (1883)
#define MQTT_PUBLIC_TOPIC "uok/iot/<ta583>/capacitive" // Replace <ta583> with your username
#define MQTT_SUBSCRIBE_TOPIC "uok/iot/<ta583>/messages" // Choose your subscription topic
#define TOUCH_PIN (4)  // The GPIO pin to use for touch sensing (Ensure this pin supports touch)

WiFiClient wifiClient;
PubSubClient client(wifiClient);
boolean touchFlag = false;  // Flag to track touch state

// Callback function to handle messages received
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.println(topic);
    Serial.print("Message: ");

    // Print the message payload
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200); // Initialize serial for debugging

    // Connect to WiFi network
    Serial.print("Connecting to WiFi: ");
    WiFi.begin(SSID, WIFI_PWD); // Attempt to connect to WiFi

    // Wait for WiFi to connect
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(". ");
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Set up MQTT connection
    client.setServer(MQTT_BROKER, MQTT_PORT); // Set broker settings
    client.setCallback(callback); // Define the callback function for incoming messages

    // Connect to MQTT broker
    while (!client.connected()) {
        if (client.connect(("ESP32-" + String(random(0xffff), HEX)).c_str())) {
            Serial.println("MQTT connected.");

            // Subscribe to the additional topic
            client.subscribe(MQTT_SUBSCRIBE_TOPIC);
            Serial.print("Subscribed to topic: ");
            Serial.println(MQTT_SUBSCRIBE_TOPIC);
        } else {
            Serial.printf("Failed, rc=%d. Try again in 5 seconds\n", client.state());
            delay(5000); // Wait 5 seconds before retrying
        }
    }
}

void loop() {
    // Let the MQTT client manage its internals
    client.loop();

    // Read the touch sensor value (make sure TOUCH_PIN supports touch)
    int touchValue = touchRead(TOUCH_PIN);  // Read touch sensor value

    // Check if the touch sensor is activated (value is below a threshold)
    if (touchValue < 50) {  // Threshold value to determine if the touch sensor is triggered
        if (!touchFlag) {  // If it wasn't previously touched
            touchFlag = true; // Mark as currently touching
            Serial.println("Touched"); // Report touch event on serial
            client.publish(MQTT_PUBLIC_TOPIC, "touch"); // Publish "touch" message to MQTT topic
        }
    } else {  // If not touching
        touchFlag = false; // Reset touch state
    }
}
