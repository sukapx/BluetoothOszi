#include <Arduino.h>
#include "BluetoothSerial.h"
#include <array>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

bool deviceConnected = false;
#define UUID_BASE "-5239-4069-806d-7e5c97393755"
#define UUID_SERVICE "00000001" UUID_BASE
#define UUID_CHARAC1 "00000002" UUID_BASE
#define UUID_CHARAC2 "00000003" UUID_BASE
#define UUID_CHARAC3 "00000004" UUID_BASE

BLECharacteristic* bleValue1;
BLECharacteristic* bleValue2;
BLECharacteristic* bleValue3;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("device Connected");
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    Serial.println("device disconnected");
    deviceConnected = false;
  }
};

class SomeValue: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);
        Serial.println();
      }
    }
};

/**
 * @brief Setup Application
 */
void setup() {
  Serial.begin(115200);
  Serial.println("Setting up ...");

  // Create the BLE Device
  BLEDevice::init("ESP32 BLE");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bmeService = pServer->createService(UUID_SERVICE);

  bleValue1 = bmeService->createCharacteristic(UUID_CHARAC1, 
                                                BLECharacteristic::PROPERTY_NOTIFY |
                                                BLECharacteristic::PROPERTY_READ);
  bleValue2 = bmeService->createCharacteristic(UUID_CHARAC2, 
                                                BLECharacteristic::PROPERTY_NOTIFY |
                                                BLECharacteristic::PROPERTY_READ);
  bleValue3 = bmeService->createCharacteristic(UUID_CHARAC3, 
                                                BLECharacteristic::PROPERTY_WRITE_NR);
  bleValue3->setCallbacks(new SomeValue());

  // Without these, Nordic App doesn't show notification update 
  auto bleValue1Descriptor = new BLE2902();
  bleValue1->addDescriptor(bleValue1Descriptor);

  auto bleValue2Descriptor = new BLE2902();
  bleValue2->addDescriptor(bleValue2Descriptor);

  auto bleValue3Descriptor = new BLE2902();
  bleValue3->addDescriptor(bleValue3Descriptor);
  
  // Start the service
  bmeService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(UUID_SERVICE);
  pServer->getAdvertising()->start();

  Serial.println("Setup Done!");
}

/**
 * @brief Application loop
 */
unsigned long lastTime = 0;
const unsigned long timerDelay = 1000;
void loop() {
  if ((millis() - lastTime) > timerDelay) {
    lastTime = millis();
    float value = millis()*0.001;
    Serial.print("Uptime: ");
    Serial.println(value);

    if (deviceConnected) {
      Serial.println("Send BLE Characteristics");

      bleValue1->setValue(value);
      bleValue1->notify();
      
      value = analogRead(36) / 4096.F;
      bleValue2->setValue(value);
      bleValue2->notify();
    }
  }else{
    delay(1);
  }
}
