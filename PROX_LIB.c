// Remove plib warnings
#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include <plib.h>

#include "PROX_LIB.h"

#define	GetSystemClock()              (80000000ul)
#define	GetPeripheralClock()          (GetSystemClock()/(1 << OSCCONbits.PBDIV))

#define Fsck (375000)
#define BRG_VAL (GetPeripheralClock()/2/Fsck)

#include "PROX_LIB.h"

void PROXINIT()
{
        // Presetup for I2C
    // **************************************************************************************************************
    unsigned char SlaveAddress;
    unsigned char RegisterAddress[10] = {0x83, 0x89, 0x82, 0x8C, 0x8D, 0x8A,0x8B, 0x80, 0x8E, 0x80};
    unsigned char Data[10] = {0x0A, 0x62, 0x00, 0xFF, 0xFF, 0x09, 0x00, 0x01, 0x00, 0x03};
    char i2cData[10];
    int DataSz;
    
    SlaveAddress = 0x13;//proximity sensor address
    int i;
    for(i = 0; i < 10; i++)
    {
        // Send Data to eeprom to program one location
        i2cData[0] = (SlaveAddress << 1) | 0;//EEPROM Device Address and WR Command
        i2cData[1] = RegisterAddress[i];//eeprom location to program (high address byte)
        //i2cData[2] = 0x40;//eeprom location to program (low address byte)
        i2cData[2] = Data[i];//data to write
        DataSz = 3;
        
        StartI2C1();//Send the Start Bit
        IdleI2C1();//Wait to complete
        
        int Index = 0;
        while( DataSz ){
            MasterWriteI2C1( i2cData[Index++] );
            IdleI2C1();//Wait to complete
            DataSz--;
            //ACKSTAT is 0 when slave acknowledge. if 1 then slave has not acknowledge the data.
            if( I2C1STATbits.ACKSTAT ){//if this executes, ERROR
                --i;
                break;
            }
        }
        StopI2C1();//Send the Stop condition
        IdleI2C1();//Wait to complete
        // wait for eeprom to complete write process. poll the ack status
    }
}

void disableProxSensor()
{
//    Wire.beginTransmission(19); // Disable the proximity sensor
//    Wire.write(byte(0x89));   // sends address byte
//    Wire.write(byte(0x60));   // Disable
//    Wire.endTransmission();   // stop transmitting
    unsigned char SlaveAddress = 0x13;
    unsigned char RegisterAddress = 0x89;
    unsigned char Data = 0x60;
    char i2cData[10];
    int DataSz, i;

    // Send Data to eeprom to program one location
    i2cData[0] = (SlaveAddress << 1) | 0;//EEPROM Device Address and WR Command
    i2cData[1] = RegisterAddress;//eeprom location to program (high address byte)
    i2cData[2] = Data;//data to write
    DataSz = 3;

    StartI2C1();//Send the Start Bit
    IdleI2C1();//Wait to complete

    int Index = 0;
    while( DataSz ){
        MasterWriteI2C1( i2cData[Index++] );
        IdleI2C1();//Wait to complete
        DataSz--;
    }

    StopI2C1();//Send the Stop condition
    IdleI2C1();//Wait to complete

}