// SmartSpin2K code
// This software registers an ESP32 as a BLE FTMS device which then uses a stepper motor to turn the resistance knob on a regular spin bike.
// BLE code based on examples from https://github.com/nkolban
// Copyright 2020 Anthony Doud
// This work is licensed under the GNU General Public License v2
// Prototype hardware build from plans in the SmartSpin2k repository are licensed under Cern Open Hardware Licence version 2 Permissive

#include "Main.h"
#include <math.h>
#include "BLE_Common.h"
//#include <queue>

int bleConnDesc = 1;
bool _BLEClientConnected = false;
bool updateConnParametersFlag = false;
TaskHandle_t BLECommunicationTask;

// See: https://github.com/oesmith/gatt-xml/blob/master/org.bluetooth.characteristic.indoor_bike_data.xml
uint8_t     const FitnessMachineIndoorBikeData::flagBitIndices[FieldCount]    = {    0,    1,   2,   3,   4,   5,   6,   7,   8,   8,   8,   9,  10,  11,   12 };
uint8_t     const FitnessMachineIndoorBikeData::flagEnabledValues[FieldCount] = {    0,    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,    1 };
size_t      const FitnessMachineIndoorBikeData::byteSizes[FieldCount]         = {    2,    2,   2,   2,   3,   2,   2,   2,   2,   2,   1,   1,   1,   2,    2 };
uint8_t     const FitnessMachineIndoorBikeData::signedFlags[FieldCount]       = {    0,    0,   0,   0,   0,   1,   1,   1,   0,   0,   0,   0,   0,   0,    0 };
double_t    const FitnessMachineIndoorBikeData::resolutions[FieldCount]       = { 0.01, 0.01, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.1, 1.0,  1.0 };

//https://github.com/oesmith/gatt-xml/blob/master/org.bluetooth.characteristic.cycling_power_measurement.xml
//    
//                                                                              IPWR, PPB,  PPR,    AT, ATS,    WRD,    CRD, EFM,  ETM,  EA, TDS, BDS,   AE,  OCI 
//int8_t      const CyclingPowerMeasurement::flagBitIndices[FieldCount]        = {  -1,    0,    1,     2,   3,      4,      5,   6,    7,   8,   9,  10,   11,  12};
//uint8_t     const CyclingPowerMeasurement::flagEnabledValues[FieldCount]      = {   0,    1,    1,     1,   1,      1,      1,   1,    1,   1,   1,   1,    1,   1};
//size_t      const CyclingPowerMeasurement::byteSizes[FieldCount]              = {   2,    1,    0,     2,   0,      6,      5,   4,    4,   6,   2,   2,    2,   0};
//uint8_t     const CyclingPowerMeasurement::signedFlags[FieldCount]            = {   0,    0,    0,     0,   0,      0,      0,   1,    1,   0,   0,   0,    0,   0};
//double_t    const CyclingPowerMeasurement::resolutions[FieldCount]            = { 1.0,  1.0,  1.0,   1.0, 1.0,    1.0,    1, 1.0,  1.0, 1.0, 1.0, 1.0,  1.0, 1.0};
//double_t    const CyclingPowerMeasurement::resolutions[FieldCount]            = {   1,  1.0,   .5,  1/32, 1.0, 1/2048, 1/1024, 1.0, 1/32, 1.0, 1.0, 1.0,  1.0, 1.0};

void BLECommunications(void *pvParameters)
{
  for (;;)
  {
      //**********************************Client***************************************/
            for (size_t x = 0; x < NUM_BLE_DEVICES; x++)
            {
                if (spinBLEClient.myBLEDevices[x].advertisedDevice)
                {
                    myAdvertisedBLEDevice myAdvertisedDevice = spinBLEClient.myBLEDevices[x];
                    if ((myAdvertisedDevice.connectedClientID != BLE_HS_CONN_HANDLE_NONE) && (myAdvertisedDevice.doConnect == false)) //client must not be in connection process
                    {
                        if (NimBLEDevice::getClientByPeerAddress(myAdvertisedDevice.peerAddress)) //nullptr chack
                        {
                        BLEClient *pClient = NimBLEDevice::getClientByPeerAddress(myAdvertisedDevice.peerAddress);
                        if ((myAdvertisedDevice.serviceUUID != BLEUUID((uint16_t)0x0000)) && (pClient->isConnected()))
                        {
                        //Write the recieved data to the Debug Director
                        BLERemoteCharacteristic *pRemoteBLECharacteristic = pClient->getService(myAdvertisedDevice.serviceUUID)->getCharacteristic(myAdvertisedDevice.charUUID);
                        std::string pData = pRemoteBLECharacteristic->getValue();
                        int length = pData.length();
                        String debugOutput = "";
                        for (int i = 0; i < length; i++)
                        {
                            debugOutput += String(pData[i], HEX) + " ";
                        }
                        debugDirector(debugOutput + "<-" + String(myAdvertisedDevice.serviceUUID.toString().c_str()) + " | " + String(myAdvertisedDevice.charUUID.toString().c_str()), true, true);
                         if (pRemoteBLECharacteristic->getUUID() == CYCLINGPOWERMEASUREMENT_UUID)
                         {
                             BLE_CPSDecode(pRemoteBLECharacteristic);
                             if(!spinBLEClient.connectedPM)
                             {
                                 spinBLEClient.connectedPM = true;
                             }
                             for (size_t y = 0; y < NUM_BLE_DEVICES; y++) //Disconnect oldest PM to avoid two connected. 
                             {
                                if((myAdvertisedDevice.connectedClientID != spinBLEClient.myBLEDevices[y].connectedClientID) && (spinBLEClient.myBLEDevices[y].connectedClientID != BLE_HS_CONN_HANDLE_NONE) && (spinBLEClient.myBLEDevices[y].charUUID == CYCLINGPOWERMEASUREMENT_UUID))
                                {
                                    spinBLEClient.intentionalDisconnect = true;
                                    pRemoteBLECharacteristic->getRemoteService()->getClient()->disconnect();
                                    debugDirector("Found Duplicate CPS, Disconnecting one.");
                                }
                             }          
                         }
                        if ((pRemoteBLECharacteristic->getUUID() == FITNESSMACHINEINDOORBIKEDATA_UUID) || (pRemoteBLECharacteristic->getUUID() == FLYWHEEL_UART_SERVICE_UUID) || (pRemoteBLECharacteristic->getUUID() == HEARTCHARACTERISTIC_UUID))
                        {
                            BLE_FTMSDecode(pRemoteBLECharacteristic);
                             if(!spinBLEClient.connectedPM)
                             {
                                 spinBLEClient.connectedPM = true;
                             }
                        }
                    }
                }
            }
        }
    }

      //***********************************SERVER**************************************/
    if ((spinBLEClient.connectedHR && !spinBLEClient.connectedPM) && (userConfig.getSimulatedHr() > 0) && userPWC.hr2Pwr)
    {
      calculateInstPwrFromHR();
    }
    if (!spinBLEClient.connectedPM && !userPWC.hr2Pwr)
    {
      userConfig.setSimulatedCad(0);
      userConfig.setSimulatedWatts(0);
    }
    if (!spinBLEClient.connectedHR)
    {
      userConfig.setSimulatedHr(0);
    }

    if (_BLEClientConnected)
    {
      //update the BLE information on the server
      computeCSC();
      updateIndoorBikeDataChar();
      updateCyclingPowerMesurementChar();
      updateHeartRateMeasurementChar();
      GlobalBLEClientConnected = true;
      
      if (updateConnParametersFlag)
      {
        vTaskDelay(100/portTICK_PERIOD_MS);
        BLEDevice::getServer()->updateConnParams(bleConnDesc, 40, 50, 0, 100);
        updateConnParametersFlag = false;
      }
    }
    else
    {
      GlobalBLEClientConnected = false;
    }
    if (!_BLEClientConnected)
    {
      digitalWrite(LED_PIN, LOW); //blink if no client connected
    }
    vTaskDelay((BLE_NOTIFY_DELAY / 2) / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay((BLE_NOTIFY_DELAY / 2) / portTICK_PERIOD_MS);
    #ifdef DEBUG_STACK
    debugDirector("BLEServer High Water Mark: " + String(uxTaskGetStackHighWaterMark(BLECommunicationTask)));
    #endif
  }
}

std::unique_ptr<SensorData> SensorDataFactory::getSensorData(BLERemoteCharacteristic *characteristic, const uint8_t *data, size_t length) {

    if (characteristic->getUUID() == HEARTCHARACTERISTIC_UUID) {
        return std::unique_ptr<SensorData>(new HeartRateData(characteristic, data, length));
    }

    if (characteristic->getUUID() == FLYWHEEL_UART_SERVICE_UUID) {
        return std::unique_ptr<SensorData>(new FlywheelData(characteristic, data, length));
    }

    if (characteristic->getUUID() == FITNESSMACHINEINDOORBIKEDATA_UUID) {
        return std::unique_ptr<SensorData>(new FitnessMachineIndoorBikeData(characteristic, data, length));
    }
    
    return std::unique_ptr<SensorData>(new NullData(characteristic, data, length));
}

String SensorData::getId() {
    return id;
}

bool    NullData::hasHeartRate()        { return false; }
bool    NullData::hasCadence()          { return false; }
bool    NullData::hasPower()            { return false; }
int     NullData::getHeartRate()        { return INT_MIN; }
float   NullData::getCadence()          { return NAN; }
int     NullData::getPower()            { return INT_MIN; }

bool    HeartRateData::hasHeartRate()   { return true; }
bool    HeartRateData::hasCadence()     { return false; }
bool    HeartRateData::hasPower()       { return false; }
int     HeartRateData::getHeartRate()   { return (int)data[1]; }
float   HeartRateData::getCadence()     { return NAN; }
int     HeartRateData::getPower()       { return INT_MIN; }

bool    FlywheelData::hasHeartRate()    { return false; }
bool    FlywheelData::hasCadence()      { return data[0] == 0xFF; }
bool    FlywheelData::hasPower()        { return data[0] == 0xFF; }
int     FlywheelData::getHeartRate()    { return INT_MIN; }

float   FlywheelData::getCadence() {
    if (!hasCadence()) {
        return NAN;
    }
    return float(bytes_to_u16(data[4], data[3]));
}

int     FlywheelData::getPower() { 
    if (!hasPower()) {
        return INT_MIN; 
    }
    return data[12];
}

bool FitnessMachineIndoorBikeData::hasHeartRate() {
    return values[Types::HeartRate] != NAN;
}

bool FitnessMachineIndoorBikeData::hasCadence() {
    return values[Types::InstantaneousCadence] != NAN;
}

bool FitnessMachineIndoorBikeData::hasPower() {
    return values[Types::InstantaneousPower] != NAN;
}

int FitnessMachineIndoorBikeData::getHeartRate() {
    double_t value = values[Types::HeartRate];
    if (value == NAN) {
        return INT_MIN;
    }
    return int(value);
}

float FitnessMachineIndoorBikeData::getCadence() {
    double_t value = values[Types::InstantaneousCadence];
    if (value == NAN) {
        return nanf("");
    }
    return float(value);
}

int FitnessMachineIndoorBikeData::getPower() {
    double_t value = values[Types::InstantaneousPower];
    if (value == NAN) {
        return INT_MIN;
    }
    return int(value);
}

FitnessMachineIndoorBikeData::FitnessMachineIndoorBikeData(BLERemoteCharacteristic *characteristic, const uint8_t *data, size_t length) : 
        SensorData("FTMS", characteristic, data, length), flags(bytes_to_u16(data[1], data[0])) {
    uint8_t dataIndex = 2;
    values = new double_t[FieldCount];
    std::fill_n(values, FieldCount, NAN);
    for (int typeIndex = Types::InstantaneousSpeed; typeIndex <= Types::RemainingTime; typeIndex++) {
        if (bitRead(flags, flagBitIndices[typeIndex]) == flagEnabledValues[typeIndex]) {
            uint8_t byteSize = byteSizes[typeIndex];
            if (byteSize > 0) {
                int value = data[dataIndex];
                for (int dataOffset = 1; dataOffset < byteSize; dataOffset++) {
                    uint8_t dataByte = data[dataIndex + dataOffset];
                    value |= (dataByte << (dataOffset * 8));
                }
                dataIndex += byteSize;
                value = convert(value, byteSize, signedFlags[typeIndex]);
                double_t result = double_t(int((value * resolutions[typeIndex] * 10) + 0.5)) / 10.0;
                values[typeIndex] = result;
            }
        }
    }
}

FitnessMachineIndoorBikeData::~FitnessMachineIndoorBikeData() {
    delete []values;
}

int FitnessMachineIndoorBikeData::convert(int value, size_t length, uint8_t isSigned) {
    int mask = 255 * length;
    int convertedValue = value & mask;
    if (isSigned) {
        convertedValue = convertedValue - (convertedValue >> (length * 8 - 1) << (length * 8));
    }
    return convertedValue;
}

