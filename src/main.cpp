// Author: Christine Lai
// Project Name: Amulete: Smart Garment for Endometriosis
// Class Name: ITP 388: Developing Connected Devices
// Last Updated: May 8 2025

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

int heatPin = 12;
bool deviceConnected = false;
bool heatingOn = false;
long heatStartTime = 0;
const long autoOffDuration = 10800000; // 3 hours in milliseconds

BLECharacteristic *pTxCharacteristic;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    Serial.println("Device connected");
  }
  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    Serial.println("Device disconnected");
    pServer->startAdvertising();
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic) override
  {
    Serial.println("BLE triggered");

    std::string rxValue = pCharacteristic->getValue();
    String value = String(rxValue.c_str());
    value.trim();
    Serial.print("Received over BLE: ");
    Serial.println(value);

    if (value == "on")
    {
      digitalWrite(heatPin, HIGH);
      heatingOn = true;
      heatStartTime = millis(); // Start timer
      Serial.println("Heating ON: Say bye bye to your pain 💛");
    }
    else if (value == "off")
    {
      digitalWrite(heatPin, LOW);
      heatingOn = false;
      Serial.println("Heating OFF");
    }
    else
    {
      Serial.println("Unknown command. Please input 'on' or 'off'");
    }
  }
};

void setup()
{
  Serial.begin(115200);
  pinMode(heatPin, OUTPUT);
  digitalWrite(heatPin, LOW);

  BLEDevice::init("Amulete Device");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_TX,
      BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_RX,
      BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();

  Serial.println("BLE Ready. Waiting for connection...");
}

void loop()
{
  if (heatingOn && (millis() - heatStartTime >= autoOffDuration))
  {
    digitalWrite(heatPin, LOW);
    heatingOn = false;
    Serial.println("Auto-off triggered: Heating has been turned off ⏱️");
  }

  delay(10);
}
