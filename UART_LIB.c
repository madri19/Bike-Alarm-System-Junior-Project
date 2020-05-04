// Handle all UART interface
#include "UART_LIB.h"
#include <xc.h>
#include <sys/attribs.h>
#include <plib.h>
//#include "GSM_MQTT.h"
#include "UART_LIB.h"
#include "TIMING_MS.h"

void ResetNewBuffer();


//#pragma config FPLLODIV = DIV_1, FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FWDTEN = OFF, FCKSM = CSECME, FPBDIV = DIV_1
//#pragma config OSCIOFNC = ON, POSCMOD = XT, FSOSCEN = ON, FNOSC = PRIPLL
//#pragma config CP = OFF, BWP = OFF, PWP = OFF



void UARTinit()
{
    ResetNewBuffer();
    INTEnableSystemMultiVectoredInt();
    UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(UART1, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART1, GetPeripheralClock(), DESIRED_BAUDRATE);
    UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
    // Configure UART1 RX Interrupt
    INTEnable(INT_SOURCE_UART_RX(UART1), INT_ENABLED);
    INTSetVectorPriority(INT_VECTOR_UART(UART1), INT_PRIORITY_LEVEL_2);
    INTSetVectorSubPriority(INT_VECTOR_UART(UART1), INT_SUB_PRIORITY_LEVEL_0);

    // configure for multi-vectored mode
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

  
    //setup led(RA3) which is toggled at every RX interrupt
    mPORTAClearBits(BIT_3);
    mPORTASetPinsDigitalOut(BIT_3);

    //UART1 is also connected to Pin 0(RX)(RF2) and Pin 1(TX)(RF8)
    mPORTFClearBits(BIT_2);
    mPORTFClearBits(BIT_8);
    mPORTFSetPinsDigitalIn(BIT_2);
    mPORTFSetPinsDigitalOut(BIT_8);

    // configure for multi-vectored mode
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

    // enable interrupts
    INTEnableInterrupts();

    // wait some time until the UART is done setting up connection
    int i = 100000;
    while(i)
    {
      asm volatile("nop");
      i--;
    }
}

// helper uart functions
void WriteString(const char *string)
{
  while (*string != '\0')
    {
      while(!UARTTransmitterIsReady(UART1));
      
      UARTSendDataByte(UART1, *string);
      string++;
    }
  while(!UARTTransmissionHasCompleted(UART1));
}

void PutCharacter(const char character)
{
  while (!UARTTransmitterIsReady(UART1));
  UARTSendDataByte(UART1, character);
  while (!UARTTransmissionHasCompleted(UART1));
}


void ResetNewBuffer()
{
    front = 0;
    end = 0;
    int i = 0;
    while(i < 64)
    {
        newBuffer[i]= '\0';
        i++;
    }
}
 
 void __ISR(24, IPL2SOFT) IntUart1Handler(void)
 {
   // Is this an RX interrupt?
  if (INTGetFlag(INT_SOURCE_UART_RX(UART1)))
    {
      // Clear the RX interrupt Flag
      INTClearFlag(INT_SOURCE_UART_RX(UART1));
      
      
      // Echo what we just received.
      //PutCharacter(character);
      
      //add received character to new (larger) buffer
      

      while(U1STAbits.URXDA==1) //if the buffer has stuff in it
      {
         //copy the stuff
         char character = UARTGetDataByte(UART1);
         newBuffer[end] = character;
         end++;
      }
      
      if (U1STAbits.OERR == 1)//if an overflow occurs, clear out the fifo
      {                         //We already saved the data we need
           U1STAbits.OERR = 0;
      }

      // Toggle LED to indicate UART activity
      mPORTAToggleBits(BIT_7);

    }

  // We don't care about TX interrupt
  if ( INTGetFlag(INT_SOURCE_UART_TX(UART1)) )
    {
      INTClearFlag(INT_SOURCE_UART_TX(UART1));
    }  
 }