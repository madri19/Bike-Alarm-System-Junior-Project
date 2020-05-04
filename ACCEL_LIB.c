// Remove plib warnings
#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include <plib.h>

#include "ACCEL_LIB.h"

#define	GetSystemClock()              (80000000ul)
#define	GetPeripheralClock()          (GetSystemClock()/(1 << OSCCONbits.PBDIV))

#define Fsck (375000)
#define BRG_VAL (GetPeripheralClock()/2/Fsck)

void i2c_wait(unsigned int cnt){
    while(--cnt){
        asm volatile("nop");
        asm volatile("nop");
    }
}

void enableI2C(){
    //Setup and enable I2C
    // Set Periph Bus Divider 60MHz / 8 = 9MHz Fpb
    mOSCSetPBDIV( OSC_PB_DIV_8 );
    //Enable channel
    OpenI2C1( I2C_EN, BRG_VAL );
}

void setupAccerlerometerI2C(){
    
    // Setup Accelerometer with I2C
    unsigned char SlaveAddress = 0x18; //accelerometer sensor address
    unsigned char RegisterAddress[9] = {0x31, 0x20, 0x21, 0x22, 0x23, 0x24, 0x32, 0x33, 0x30};
    unsigned char Data[9] = {0x00, 0x57, 0x00, 0x40, 0x00, 0x08, 0x2F, 0x00, 0x0A};
    char i2cData[9];
    int DataSz;
    int i, k;
    
    for(i = 0; i < 9; i++)
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

void clearAccelerometerInterruptFlag(){
    // Read a byte to clear the interrupt on the accelerometer
    unsigned char SlaveAddress = 0x18;//accelerometer sensor address
    char i2cData[3];
    int DataSz = 3;
    int i;
 
    // Send Data to eeprom to program one location
    i2cData[0] = (SlaveAddress << 1) | 0;//EEPROM Device Address and WR Command
    i2cData[1] = 0x31;//eeprom location to program
    i2cData[2] = 0x18;//data to write
    
    
    for(i = 0; i < 3; i++)
    {
        StartI2C1();//Send the Start Bit
        IdleI2C1();//Wait to complete

        int Index = 0;
        while( DataSz ){
            MasterWriteI2C1( i2cData[Index++] );
            IdleI2C1();//Wait to complete
            DataSz--;
            //ACKSTAT is 0 when slave acknowledge. if 1 then slave has not acknowledge the data.
            if( I2C1STATbits.ACKSTAT ){//if this executes, ERROR
                break;
            }
        }

        //now send a start sequence again
        RestartI2C1();//Send the Stop condition
        IdleI2C1();//Wait to complete

        MasterWriteI2C1( (SlaveAddress << 1) | 1 ); //transmit read command
        IdleI2C1();//Wait to complete

        unsigned char i2cbyte;
        i2cbyte = MasterReadI2C1();

        StopI2C1();//Send the Stop condition
        IdleI2C1();//Wait to complete
    }
}
