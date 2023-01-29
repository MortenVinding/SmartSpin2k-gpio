/*
 * Copyright (C) 2020  Anthony Doud & Joel Baranick
 * All rights reserved
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#pragma once

#include <NimBLEUUID.h>

// SmartSpin2K custom UUID's
#define SMARTSPIN2K_SERVICE_UUID        NimBLEUUID("77776277-7877-7774-4466-896665500000")
#define SMARTSPIN2K_CHARACTERISTIC_UUID NimBLEUUID("77776277-7877-7774-4466-896665500001")

// Heart Service
#define HEARTSERVICE_UUID        NimBLEUUID((uint16_t)0x180D)
#define HEARTCHARACTERISTIC_UUID NimBLEUUID((uint16_t)0x2A37)

//BatteryLevel Service
#define BATTERYSERVICE_UUID NimBLEUUID((uint16_t)0x180F) // heart rate sensor service uuid, as defined in gatt specifications
#define BATTERYCHARACTERISTIC_UUID NimBLEUUID ((uint16_t)0x2A19)

// Cycling Power Service
#define CSCSERVICE_UUID              NimBLEUUID((uint16_t)0x1816)
#define CSCMEASUREMENT_UUID          NimBLEUUID((uint16_t)0x2A5B)
#define CYCLINGPOWERSERVICE_UUID     NimBLEUUID((uint16_t)0x1818)
#define CYCLINGPOWERMEASUREMENT_UUID NimBLEUUID((uint16_t)0x2A63)
#define CYCLINGPOWERFEATURE_UUID     NimBLEUUID((uint16_t)0x2A65)
#define SENSORLOCATION_UUID          NimBLEUUID((uint16_t)0x2A5D)

// Fitness Machine Service
#define FITNESSMACHINESERVICE_UUID              NimBLEUUID((uint16_t)0x1826)
#define FITNESSMACHINEFEATURE_UUID              NimBLEUUID((uint16_t)0x2ACC)
#define FITNESSMACHINECONTROLPOINT_UUID         NimBLEUUID((uint16_t)0x2AD9)
#define FITNESSMACHINESTATUS_UUID               NimBLEUUID((uint16_t)0x2ADA)
#define FITNESSMACHINEINDOORBIKEDATA_UUID       NimBLEUUID((uint16_t)0x2AD2)
#define FITNESSMACHINETRAININGSTATUS_UUID       NimBLEUUID((uint16_t)0x2AD3)
#define FITNESSMACHINERESISTANCELEVELRANGE_UUID NimBLEUUID((uint16_t)0x2AD6)
#define FITNESSMACHINEPOWERRANGE_UUID           NimBLEUUID((uint16_t)0x2AD8)
#define FITNESSMACHINEINCLINATIONRANGE_UUID     NimBLEUUID((uint16_t)0x2AD5)

// GATT service/characteristic UUIDs for Flywheel Bike from ptx2/gymnasticon/
#define FLYWHEEL_UART_SERVICE_UUID NimBLEUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e")
#define FLYWHEEL_UART_RX_UUID      NimBLEUUID("6e400002-b5a3-f393-e0a9-e50e24dcca9e")
#define FLYWHEEL_UART_TX_UUID      NimBLEUUID("6e400003-b5a3-f393-e0a9-e50e24dcca9e")
#define FLYWHEEL_BLE_NAME          "Flywheel 1"

// The Echelon Services
#define ECHELON_DEVICE_UUID  NimBLEUUID("0bf669f0-45f2-11e7-9598-0800200c9a66")
#define ECHELON_SERVICE_UUID NimBLEUUID("0bf669f1-45f2-11e7-9598-0800200c9a66")
#define ECHELON_WRITE_UUID   NimBLEUUID("0bf669f2-45f2-11e7-9598-0800200c9a66")
#define ECHELON_DATA_UUID    NimBLEUUID("0bf669f4-45f2-11e7-9598-0800200c9a66")

// Dummy UUID for Peloton Serial Data Interface
#define PELOTON_DATA_UUID NimBLEUUID("00000000-0000-0000-0000-0000000000321")
#define PELOTON_ADDRESS   NimBLEAddress("00:00:00:00:00:00:00")
// peloton Serial
#define PELOTON_RQ_SIZE      4
#define PELOTON_HEADER       0xF1
#define PELOTON_REQUEST      0xF5
#define PELOTON_FOOTER       0xF6
#define PELOTON_CAD_ID       0x41
#define PELOTON_RES_ID       0x49
#define PELOTON_POW_ID       0x44
#define PELOTON_REQ_POS      1
#define PELOTON_CHECKSUM_POS 2
