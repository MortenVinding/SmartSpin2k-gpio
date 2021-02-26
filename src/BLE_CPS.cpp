// SmartSpin2K code
// This software registers an ESP32 as a BLE FTMS device which then uses a stepper motor to turn the resistance knob on a regular spin bike.
// BLE code based on examples from https://github.com/nkolban
// Copyright 2020 Anthony Doud
// This work is licensed under the GNU General Public License v2
// Prototype hardware build from plans in the SmartSpin2k repository are licensed under Cern Open Hardware Licence version 2 Permissive

/*Calculate Cadence and power from Cycling Power Measurement*/

#include "Main.h"
#include "BLE_Common.h"
#include <Arduino.h>

//Takes a NimBLERemoteCharacteristic with CYCLINGPOWERMEASUREMENT_UUID and extracts the data. Also disconnects a secondary power meter. 
void BLE_CPSDecode(NimBLERemoteCharacteristic *pBLERemoteCharacteristic)
{
    if (pBLERemoteCharacteristic->getUUID() == CYCLINGPOWERMEASUREMENT_UUID) //double checking this was 
    {
        std::string pData = pBLERemoteCharacteristic->getValue();
        if (pBLERemoteCharacteristic->getRemoteService()->getClient()->getConnId() == spinBLEClient.lastConnectedPMID) //disregarding other pm's that may still be connected
        {
            byte flags = pData[0];
            int cPos = 2; //lowest position cadence could ever be

            //Instanious power is always present. Do that first.
            //first calculate which fields are present. Power is always 2 & 3, cadence can move depending on the flags.
            userConfig.setSimulatedWatts(bytes_to_u16(cPos + 1, cPos));
            cPos += 2;
            if (userConfig.getDoublePower())
            {
                userConfig.setSimulatedWatts(userConfig.getSimulatedWatts() * 2);
            }

            if (bitRead(flags, 0))
            {
                //pedal balance field present
                cPos++;
            }
            if (bitRead(flags, 1))
            {
                //pedal power balance reference
                //no field associated with this.
            }
            if (bitRead(flags, 2))
            {
                //accumulated torque field present
                cPos += 2;
            }
            if (bitRead(flags, 3))
            {
                //accumulated torque field source
                //no field associated with this.
            }
            if (bitRead(flags, 4))
            {
                //Wheel Revolution field PAIR Data present. 32-bits for wheel revs, 16 bits for wheel event time.
                //Why is that so hard to find in the specs?
                cPos += 6;
            }
            if (bitRead(flags, 5))
            {
                //Crank Revolution data present, lets process it.
                spinBLEClient.crankRev[1] = spinBLEClient.crankRev[0];
                spinBLEClient.crankRev[0] = bytes_to_int(pData[cPos + 1], pData[cPos]);
                spinBLEClient.crankEventTime[1] = spinBLEClient.crankEventTime[0];
                spinBLEClient.crankEventTime[0] = bytes_to_int(pData[cPos + 3], pData[cPos + 2]);
                if ((spinBLEClient.crankRev[0] > spinBLEClient.crankRev[1]) && (spinBLEClient.crankEventTime[0] - spinBLEClient.crankEventTime[1] != 0))
                {
                    int tCAD = (((abs(spinBLEClient.crankRev[0] - spinBLEClient.crankRev[1]) * 1024) / abs(spinBLEClient.crankEventTime[0] - spinBLEClient.crankEventTime[1])) * 60);
                    if (tCAD > 1)
                    {
                        if (tCAD > 200) //Cadence Error
                        {
                            tCAD = 0;
                        }
                        userConfig.setSimulatedCad(tCAD);
                        spinBLEClient.noReadingIn = 0;
                    }
                    else
                    {
                        spinBLEClient.noReadingIn++;
                    }
                }
                else //the crank rev probably didn't update
                {
                    if (spinBLEClient.noReadingIn > 2) //Require three consecutive readings before setting 0 cadence
                    {
                        userConfig.setSimulatedCad(0);
                    }
                    spinBLEClient.noReadingIn++;
                }

                debugDirector(" CAD: " + String(userConfig.getSimulatedCad()), false);
                debugDirector("");
            }
        }

        else
        {
            debugDirector("Disconnecting secondary PM");
            spinBLEClient.intentionalDisconnect = true;
            pBLERemoteCharacteristic->getRemoteService()->getClient()->disconnect();
        }
    }
}