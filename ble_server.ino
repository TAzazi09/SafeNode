#include <Arduino.h>
#include <BLEDevice.h>

// Define Built-in LED for ESP32
#define LED_BUILTIN 2

// UUIDs for BLE Service and Characteristics
BLEUUID serviceID("afeb2674-9d91-42c1-961c-689412fe8fb0");
BLEUUID readonlyCharID("ea550889-c4e8-4e52-a08b-44d40832b618"); // Example Read-only UUID
BLEUUID runtimeCharID("b5d48fc7-3a2f-44d4-b087-8e8f1e7f9b5c");   // Dynamic runtime characteristic UUID
BLEUUID notifyCharID("e6244b1b-8469-4de8-83c5-9c2d3d245c1c");    // Notify characteristic UUID
BLEUUID writableCharID("c73d5f9a-14e5-4afd-98d0-87f99fde3602");  // Writable characteristic UUID

// Global Variables
unsigned long startMillis; // Storing device start time
int notifyCounter = 0;     // Counter for notify characteristic
uint8_t writable_store[1]; // Storage for writable characteristic value

// Declare Notify Characteristic globally for use in loop()
BLECharacteristic *notifyCharacteristic;

// Callback Class for Writable Characteristic
class MyCallbacks : public BLECharacteristicCallbacks {
public:
  void onWrite(BLECharacteristic *pCharacteristic) {
    if (writableCharID.equals(pCharacteristic->getUUID())) {
      uint8_t *value = pCharacteristic->getData(); // Get the data written
      digitalWrite(LED_BUILTIN, value[0] ? HIGH : LOW); // Update LED state
    }
  }
};

MyCallbacks cb; // Callback instance

void setup() {
  // Initialize BLE
  BLEDevice::init("<ta583>");

  // Create BLE Server
  BLEServer *pServer = BLEDevice::createServer();

  // Create BLE Service
  BLEService *pService = pServer->createService(serviceID);

  // Example Read-only Characteristic
  BLECharacteristic *readCharacteristic = pService->createCharacteristic(
      readonlyCharID,
      BLECharacteristic::PROPERTY_READ);
  readCharacteristic->setValue("Example Text");

  // Writable Characteristic
  BLECharacteristic *writeCharacteristic = pService->createCharacteristic(
      writableCharID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  writeCharacteristic->setValue(writable_store, 1); // Initialize value
  writeCharacteristic->setCallbacks(&cb);

  // Dynamic Runtime Characteristic (Read-only)
  BLECharacteristic *runtimeCharacteristic = pService->createCharacteristic(
      runtimeCharID,
      BLECharacteristic::PROPERTY_READ);
  runtimeCharacteristic->setValue("0"); // Initial runtime value

  // Notify Characteristic
  notifyCharacteristic = pService->createCharacteristic(
      notifyCharID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  notifyCharacteristic->setValue("0"); // Initialize value

  // Start BLE Service
  pService->start();

  // Configuration
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  // Configure LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Store start time
  startMillis = millis();
}

void loop() {
  // Calculate runtime in seconds
  unsigned long secondsElapsed = (millis() - startMillis) / 1000;

  // Update Runtime Characteristic
  BLECharacteristic *runtimeCharacteristic = BLEDevice::getServer()
                                              ->getService(serviceID)
                                              ->getCharacteristic(runtimeCharID);
  if (runtimeCharacteristic) {
    runtimeCharacteristic->setValue(String(secondsElapsed).c_str());
  }

  // Increment Notify Counter and Notify Clients
  notifyCounter++;
  notifyCharacteristic->setValue(String(notifyCounter).c_str());
  notifyCharacteristic->notify();

  delay(1000); // Loop delay for updates every second
}
