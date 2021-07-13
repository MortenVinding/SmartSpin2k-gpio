/*
 * Copyright (C) 2020  Anthony Doud & Joel Baranick
 * All rights reserved
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

// #define CONFIG_SW_COEXIST_ENABLE 1

#include <memory>
#include <NimBLEDevice.h>
#include <Arduino.h>
#include <Main.h>

#define BLE_CLIENT_LOG_TAG "BLE_Client"
#define BLE_COMMON_LOG_TAG "BLE_Common"
#define BLE_SERVER_LOG_TAG "BLE_Server"
#define BLE_SETUP_LOG_TAG  "BLE_Setup"

// custom characteristic codes
#define BLE_firmwareUpdateURL     0x01
#define BLE_incline               0x02
#define BLE_simulatedWatts        0x03
#define BLE_simulatedHr           0x04
#define BLE_simulatedCad          0x05
#define BLE_simulatedSpeed        0x06
#define BLE_deviceName            0x07
#define BLE_shiftStep             0x08
#define BLE_stepperPower          0x09
#define BLE_stealthchop           0x0A
#define BLE_inclineMultiplier     0x0B
#define BLE_powerCorrectionFactor 0x0C
#define BLE_simulateHr            0x0D
#define BLE_simulateWatts         0x0E
#define BLE_simulateCad           0x0F
#define BLE_ERGMode               0x10
#define BLE_autoUpdate            0x11
#define BLE_ssid                  0x12
#define BLE_password              0x13
#define BLE_foundDevices          0x14
#define BLE_connectedPowerMeter   0x15
#define BLE_connectedHeartMonitor 0x16
#define BLE_shifterPosition       0x17
#define BLE_saveToSpiffs          0x18
#define BLE_targetPosition        0x19
#define BLE_externalControl       0x1A
#define BLE_syncMode              0x1B

// macros to convert different types of bytes into int The naming here sucks and
// should be fixed.
#define bytes_to_s16(MSB, LSB) (((signed int)((signed char)MSB))) << 8 | (((signed char)LSB))
#define bytes_to_u16(MSB, LSB) (((signed int)((signed char)MSB))) << 8 | (((unsigned char)LSB))
#define bytes_to_int(MSB, LSB) ((static_cast<int>((unsigned char)MSB))) << 8 | (((unsigned char)LSB))
// Potentially, something like this is a better way of doing this ^^

// Setup
void setupBLE();
extern TaskHandle_t BLECommunicationTask;
// ***********************Common**********************************
void BLECommunications(void *pvParameters);

// *****************************Server****************************

extern int bleConnDesc;
extern bool updateConnParametersFlag;

// TODO add the rest of the server to this class
class SpinBLEServer {
 public:
  void notifyShift(bool upDown);
};

extern SpinBLEServer spinBLEServer;

void startBLEServer();
bool spinDown();
void computeERG(int = 0);
void computeCSC();
void logCharacteristic(char *buffer, const size_t bufferCapacity, const byte *data, const size_t dataLength, const NimBLEUUID serviceUUID, const NimBLEUUID charUUID,
                       const char *format, ...);
void updateIndoorBikeDataChar();
void updateCyclingPowerMeasurementChar();
void calculateInstPwrFromHR();
void updateHeartRateMeasurementChar();
int connectedClientCount();
void controlPointIndicate();

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *, ble_gap_conn_desc *desc);
  void onDisconnect(BLEServer *);
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *);
};

// static void onNotify(NimBLECharacteristic *pCharacteristic, uint8_t *pData);
class ss2kCustomCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *);
};

// *****************************Client*****************************

// Keeping the task outside the class so we don't need a mask.
// We're only going to run one anyway.
void bleClientTask(void *pvParameters);

// UUID's the client has methods for
// BLEUUID serviceUUIDs[4] = {FITNESSMACHINESERVICE_UUID,
// CYCLINGPOWERSERVICE_UUID, HEARTSERVICE_UUID, FLYWHEEL_UART_SERVICE_UUID};
// BLEUUID charUUIDs[4] = {FITNESSMACHINEINDOORBIKEDATA_UUID,
// CYCLINGPOWERMEASUREMENT_UUID, HEARTCHARACTERISTIC_UUID,
// FLYWHEEL_UART_TX_UUID};

typedef struct DataHandle {
  uint8_t *data;
  size_t length;
} DataHandle_t;

class SpinBLEAdvertisedDevice {
 public:  // eventually these should be made private
  // // TODO: Do we dispose of this object?  Is so, we need to de-allocate the queue.
  // //       This distructor was called too early and the queue was deleted out from
  // //       under us.
  // ~SpinBLEAdvertisedDevice() {
  //   if (dataBuffer != nullptr) {
  //     Serial.println("Deleting queue");
  //     vQueueDelete(dataBuffer);
  //   }
  // }

  NimBLEAdvertisedDevice *advertisedDevice = nullptr;
  NimBLEAddress peerAddress;
  int connectedClientID = BLE_HS_CONN_HANDLE_NONE;
  BLEUUID serviceUUID   = (uint16_t)0x0000;
  BLEUUID charUUID      = (uint16_t)0x0000;
  bool userSelectedHR   = false;
  bool userSelectedPM   = false;
  bool userSelectedCSC  = false;
  bool userSelectedCT   = false;
  bool doConnect        = false;

  void set(BLEAdvertisedDevice *device, int id = BLE_HS_CONN_HANDLE_NONE, BLEUUID inserviceUUID = (uint16_t)0x0000, BLEUUID incharUUID = (uint16_t)0x0000) {
    advertisedDevice  = device;
    peerAddress       = device->getAddress();
    connectedClientID = id;
    serviceUUID       = BLEUUID(inserviceUUID);
    charUUID          = BLEUUID(incharUUID);
    // dataBuffer        = xQueueCreate((4),                  // length
    //                            sizeof(uint8_t *));  // size
  }

  void reset() {
    advertisedDevice = nullptr;
    // NimBLEAddress peerAddress;
    connectedClientID = BLE_HS_CONN_HANDLE_NONE;
    serviceUUID       = (uint16_t)0x0000;
    charUUID          = (uint16_t)0x0000;
    userSelectedHR    = false;  // Heart Rate Monitor
    userSelectedPM    = false;  // Power Meter
    userSelectedCSC   = false;  // Cycling Speed/Cadence
    userSelectedCT    = false;  // Controllable Trainer
    doConnect         = false;  // Initiate connection flag
    if (dataBuffer != nullptr) {
      Serial.println("Resetting queue");
      xQueueReset(dataBuffer);
    }
  }

  void print();

  bool enqueueData(uint8_t *data, size_t length);
  std::shared_ptr<DataHandle_t> dequeueData();
  static void deletePayload(DataHandle_t *data);

 private:
  QueueHandle_t dataBuffer = nullptr;
};

class SpinBLEClient {
 public:  // Not all of these need to be public. This should be cleaned up
          // later.
  boolean connectedPM        = false;
  boolean connectedHR        = false;
  boolean connectedCD        = false;
  boolean doScan             = false;
  bool intentionalDisconnect = false;
  int noReadingIn            = 0;
  int cscCumulativeCrankRev  = 0;
  int cscLastCrankEvtTime    = 0;

  BLERemoteCharacteristic *pRemoteCharacteristic = nullptr;

  // BLEDevices myBLEDevices;
  SpinBLEAdvertisedDevice myBLEDevices[NUM_BLE_DEVICES];

  void start();
  void serverScan(bool connectRequest);
  bool connectToServer();
  void scanProcess();
  void disconnect();
  // Check for duplicate services of BLEClient and remove the previously
  // connected one.
  void removeDuplicates(NimBLEClient *pClient);
  // Reset devices in myBLEDevices[]. Bool All (true) or only connected ones
  // (false)
  void resetDevices();
  void postConnect(NimBLEClient *pClient);

 private:
  class MyAdvertisedDeviceCallback : public NimBLEAdvertisedDeviceCallbacks {
   public:
    void onResult(NimBLEAdvertisedDevice *);
  };

  class MyClientCallback : public NimBLEClientCallbacks {
   public:
    void onConnect(BLEClient *);
    void onDisconnect(BLEClient *);
    uint32_t onPassKeyRequest();
    bool onConfirmPIN(uint32_t);
    void onAuthenticationComplete(ble_gap_conn_desc);
  };
};

extern SpinBLEClient spinBLEClient;

struct FitnessMachineFeatureFlags {
  enum Types : uint {
    AverageSpeedSupported              = 1U << 0,
    CadenceSupported                   = 1U << 1,
    TotalDistanceSupported             = 1U << 2,
    InclinationSupported               = 1U << 3,
    ElevationGainSupported             = 1U << 4,
    PaceSupported                      = 1U << 5,
    StepCountSupported                 = 1U << 6,
    ResistanceLevelSupported           = 1U << 7,
    StrideCountSupported               = 1U << 8,
    ExpendedEnergySupported            = 1U << 9,
    HeartRateMeasurementSupported      = 1U << 10,
    MetabolicEquivalentSupported       = 1U << 11,
    ElapsedTimeSupported               = 1U << 12,
    RemainingTimeSupported             = 1U << 13,
    PowerMeasurementSupported          = 1U << 14,
    ForceOnBeltAndPowerOutputSupported = 1U << 15,
    UserDataRetentionSupported         = 1U << 16
  };
};

struct FitnessMachineTargetFlags {
  enum Types : uint {
    SpeedTargetSettingSupported                           = 1U << 0,
    InclinationTargetSettingSupported                     = 1U << 1,
    ResistanceTargetSettingSupported                      = 1U << 2,
    PowerTargetSettingSupported                           = 1U << 3,
    HeartRateTargetSettingSupported                       = 1U << 4,
    TargetedExpendedEnergyConfigurationSupported          = 1U << 5,
    TargetedStepNumberConfigurationSupported              = 1U << 6,
    TargetedStrideNumberConfigurationSupported            = 1U << 7,
    TargetedDistanceConfigurationSupported                = 1U << 8,
    TargetedTrainingTimeConfigurationSupported            = 1U << 9,
    TargetedTimeTwoHeartRateZonesConfigurationSupported   = 1U << 10,
    TargetedTimeThreeHeartRateZonesConfigurationSupported = 1U << 11,
    TargetedTimeFiveHeartRateZonesConfigurationSupported  = 1U << 12,
    IndoorBikeSimulationParametersSupported               = 1U << 13,
    WheelCircumferenceConfigurationSupported              = 1U << 14,
    SpinDownControlSupported                              = 1U << 15,
    TargetedCadenceConfigurationSupported                 = 1U << 16
  };
};

inline FitnessMachineFeatureFlags::Types operator|(FitnessMachineFeatureFlags::Types a, FitnessMachineFeatureFlags::Types b) {
  return static_cast<FitnessMachineFeatureFlags::Types>(static_cast<int>(a) | static_cast<int>(b));
}

inline FitnessMachineTargetFlags::Types operator|(FitnessMachineTargetFlags::Types a, FitnessMachineTargetFlags::Types b) {
  return static_cast<FitnessMachineTargetFlags::Types>(static_cast<int>(a) | static_cast<int>(b));
}

struct FitnessMachineFeature {
  union {
    struct {
      union {
        enum FitnessMachineFeatureFlags::Types featureFlags;
        uint8_t bytes[4];
      } featureFlags;
      union {
        enum FitnessMachineTargetFlags::Types targetFlags;
        uint8_t bytes[4];
      } targetFlags;
    };
    uint8_t bytes[8];
  };
};
