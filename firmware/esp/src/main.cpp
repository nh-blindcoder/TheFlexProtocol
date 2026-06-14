#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <TFT_eSPI.h> 

TFT_eSPI tft = TFT_eSPI(); 
BLEScan* pBLEScan;

void updateScreen(float t);

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.getName() == "Nano") {
        if (advertisedDevice.haveManufacturerData()) {
          std::string data = advertisedDevice.getManufacturerData();
          int msb = (unsigned char)data[0];
          int lsb = (unsigned char)data[1];
          int tempInt = (msb << 8) | lsb;
          float temp = tempInt / 100.0;
          updateScreen(temp);
        }
      }
    }
};

void updateScreen(float t) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 40);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(4);
  tft.print(t);
  tft.println(" C");
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.println("scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(1, false);
  pBLEScan->clearResults();   
}