#include "GSM_MQTT.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
// Remove plib warnings
#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>
#define pgm_read_byte_near(addr) (*(const unsigned char *)(addr))


char Message[MESSAGE_BUFFER_LENGTH];
char Topic[TOPIC_BUFFER_LENGTH];
char inputString[UART_BUFFER_LENGTH]; 
char GSM_ReplyFlag;
char reply[10];
int TCP_Flag = 0;
int pingFlag = 0;
char tcpATerrorcount = 0;
int MQTT_Flag = 0;
int ConnectionAcknowledgement = NO_ACKNOWLEDGEMENT;
int PublishIndex = 0;
int TopicLength = 0;
int MessageLength = 0;
int MessageFlag = 0;
char modemStatus = 0;
uint32_t myIndex = 0;
uint32_t length = 0, lengthLocal = 0;
unsigned int _LastMessaseID = 0;
char _ProtocolVersion = 3;
unsigned long _PingPrevMillis = 0;
char _tcpStatus = 0;
char _tcpStatusPrev = 0;
unsigned long _KeepAliveTimeOut = 0;


uint8_t GSM_Response;
char* MQTT_HOST = "io.adafruit.com";
char* MQTT_PORT = "1883";

uint8_t GSM_Response = 0;
unsigned long previousMillis = 0;
int stringComplete = 0;  // whether the string is complete

void serialEvent();

void AutoConnect(){
    connect("PIC", 1, 1, "GiselleLFreude", "d8420283b79848939865671841e0dbc5", 0, 0, 0, 0, "", "");  
}

void OnConnect(void)
{
  /*
     This function is called when mqqt connection is established.
     put your subscription publish codes here.
  */
    subscribe(0, _generateMessageID(), "GiselleLFreude/feeds/example", 0);
    //publish(0, 0, 0, _generateMessageID(), "GiselleLFreude/feeds/panic-signal-watch", "Hello");
  /*    void subscribe(char DUP, unsigned int MessageID, char *SubTopic, char SubQoS);
          DUP       :This flag is set when the client or server attempts to re-deliver a SUBSCRIBE message
                    :This applies to messages where the value of QoS is greater than zero (0)
                    :Possible values (0,1)
                    :Default value 0
          Message ID:The Message Identifier (Message ID) field
                    :Used only in messages where the QoS levels greater than 0 (SUBSCRIBE message is at QoS =1)
          SubTopic  :Topic names to which  subscription is needed
          SubQoS    :QoS level at which the client wants to receive messages
                    :Possible values (0,1,2)
                    :Default value 0
  */

    //publish(0, 0, 0, _generateMessageID(), "GiselleLFreude/feeds/panic-signal-watch", "1");

 
  /*  void publish(char DUP, char Qos, char RETAIN, unsigned int MessageID, char *Topic, char *Message);
      DUP       :This flag is set when the client or server attempts to re-deliver a PUBLISH message
                :This applies to messages where the value of QoS is greater than zero (0)
                :Possible values (0,1)
                :Default value 0
      QoS       :Quality of Service
                :This flag indicates the level of assurance for delivery of a PUBLISH message
                :Possible values (0,1,2)
                :Default value 0
      RETAIN    :if the Retain flag is set (1), the server should hold on to the message after it has been delivered to the current subscribers.
                :When a new subscription is established on a topic, the last retained message on that topic is sent to the subscriber
                :Possible values (0,1)
                :Default value 0
      Message ID:The Message Identifier (Message ID) field
                :Used only in messages where the QoS levels greater than 0
      Topic     :Publishing topic
      Message   :Publishing Message
  */
}


void OnMessage(char *Topic, int TopicLength, char *Message, int MessageLength)
{
  /*
    This function is called whenever a message received from subscribed topics
    put your subscription publish codes here.
  */
   // digitalWrite(8, HIGH);
  //publish(0, 0, 0, _generateMessageID(), "GiselleLFreude/feeds/panic-signal-watch", "1");
  if ((strcmp(Message, "ARM"))==0)
  {
    _ALARM_FINAL_TIME_ = 90;
    LATDbits.LATD3 = 1;
    _ARM_SIGNAL_ = 1;
    publish(0, 1, 0, _generateMessageID(), "GiselleLFreude/feeds/ack-feed", "ARM");
  }
  

  if ((strcmp(Message, "DISARM"))==0)
  {
    LATDbits.LATD3 = 0;
    _ARM_SIGNAL_ = 0;
    publish(0, 1, 0, _generateMessageID(), "GiselleLFreude/feeds/ack-feed", "DISARM");
    _Alarm_Going_ = 0;
    LATDbits.LATD12 = 0;
    _ALARM_ELAPSED_TIME_ = 0;
    _ARM_SIGNAL_ = 0;
    //arm signal led
    LATDbits.LATD3 = 0;
    clearAccelerometerInterruptFlag();
    disableSensors();
  }
  
  if ((strcmp(Message, "TEST"))==0)
  {
    _ALARM_FINAL_TIME_ = 3;
    LATDbits.LATD3 = 1;
    _ARM_SIGNAL_ = 1; 
    publish(0, 1, 0, _generateMessageID(), "GiselleLFreude/feeds/ack-feed", "ARM");
    
  }
  /*
     Topic        :Name of the topic from which message is coming
     TopicLength  :Number of characters in topic name
     Message      :The containing array
     MessageLength:Number of characters in message
  */
    
    //WriteString((char*)TopicLength);
    //WriteString("\r\n");
    //WriteString((char*)Topic);
    //WriteString("\r\n");
    //WriteString((char*)MessageLength);
    //WriteString("\r\n");
    //WriteString((char*)Message);
    //WriteString("\r\n");
}



void begin(void)
{
  InitTiming();
  UARTinit();
  _KeepAliveTimeOut = 45;
  WriteString("AT\r\n");
  delay(1000);
  _tcpInit();
}


char _sendAT(char *command, unsigned long waitms)
{
  unsigned long PrevMillis = millis();
  strcpy(reply, "none");
  GSM_Response = 0;
  WriteString(command);
  delay(100);
  unsigned long currentMillis = millis();
  
//  char previous_ms[10];
//  itoa(previous_ms, PrevMillis, 10);
//  WriteString(previous_ms);
//  WriteString("\r\n");
//  
//  char current_ms[10];
//  itoa(current_ms, currentMillis, 10);
//  WriteString(current_ms);
//  WriteString("\r\n");
  
  while ( (GSM_Response == 0) && ((currentMillis - PrevMillis) < waitms) )
  {
    // delay(1);
    serialEvent();
    currentMillis = millis();
  }
  return GSM_Response;
}

char sendATreply(char *command, char *replystr, unsigned long waitms)
{
  strcpy(reply, replystr);
  unsigned long PrevMillis = millis();
  GSM_ReplyFlag = 0;
  WriteString(command);
  delay(100);
  unsigned long currentMillis = millis();
  
  while ( (GSM_ReplyFlag == 0) && ((currentMillis - PrevMillis) < waitms) )
  {
    // delay(1);
    serialEvent();
    currentMillis = millis();
  }
  return GSM_ReplyFlag;
}
void _tcpInit(void)
{
  switch (modemStatus)
  {
    case 0:
      {
        ResetNewBuffer();
        delay(1000);
        WriteString("+++");
        delay(500);
        if (_sendAT("AT\r\n", 5000) == 1)
        {
          modemStatus = 1;
        }
        else
        {
          modemStatus = 0;
          break;
        }
      }
    case 1:
      {
        ResetNewBuffer();
        if (_sendAT("ATE1\r\n", 5000) == 1)
        {
          modemStatus = 2;
        }
        else
        {
          modemStatus = 1;

          break;
        }
      }
    case 2:
      {
        ResetNewBuffer();
        if(sendATreply("AT+CREG?\r\n", "0,1", 5000) == 1)
        {
          ResetNewBuffer();
          _sendAT("AT+CIPMUX=0\r\n", 5000);
          ResetNewBuffer();
          _sendAT("AT+CIPMODE=1\r\n", 5000);
          ResetNewBuffer();
          if (sendATreply("AT+CGATT?\r\n", ": 1", 5000) != 1)
          {
            ResetNewBuffer();
            _sendAT("AT+CGATT=1\r\n", 5000);
          }
          ResetNewBuffer();
          modemStatus = 3;
          _tcpStatus = 2;
        }
        else
        {
          ResetNewBuffer();
          modemStatus = 2;
          break;
        }
      }
    case 3:
      {
        ResetNewBuffer();
        if (GSM_ReplyFlag != 7)
        {
          _tcpStatus = sendATreply("AT+CIPSTATUS\r\n", "STATE", 4000);
          if (_tcpStatusPrev == _tcpStatus)
          {
            tcpATerrorcount++;
            if (tcpATerrorcount >= 10)
            {
              tcpATerrorcount = 0;
              _tcpStatus = 7;
            }

          }
          else
          {
            _tcpStatusPrev = _tcpStatus;
            tcpATerrorcount = 0;
          }
        }
        _tcpStatusPrev = _tcpStatus;
        ResetNewBuffer();
        //PutCharacter(_tcpStatus);
        delay(100);
        switch (_tcpStatus)
        {
          case 2:
            {
              ResetNewBuffer();
              _sendAT("AT+CSTT=\"AIRTELGPRS.COM\"\r\n", 5000);
              break;
            }
          case 3:
            {
              ResetNewBuffer();
              _sendAT("AT+CIICR\r\n", 5000)  ;
              delay(5000);
              break;
            }
          case 4:
            {
              ResetNewBuffer();
              sendATreply("AT+CIFSR\r\n", ".", 4000) ;
              break;
            }
          case 5:
            {
              ResetNewBuffer();
              WriteString("AT+CIPSTART=\"TCP\",\"");
              WriteString(MQTT_HOST);
              WriteString("\",\"");
              WriteString(MQTT_PORT);
              if (_sendAT("\"\r\n", 5000) == 1)
              {
                unsigned long PrevMillis = millis();
                unsigned long currentMillis = millis();
                while ( (GSM_Response != 4) && ((currentMillis - PrevMillis) < 20000) )
                {
                  delay(1);
                  serialEvent();
                  currentMillis = millis();
                }
              }
              delay(3000);
              ResetNewBuffer();
              break;
            }
          case 6:
            {
              ResetNewBuffer();
              unsigned long PrevMillis = millis();
              unsigned long currentMillis = millis();
              while ( (GSM_Response != 4) && ((currentMillis - PrevMillis) < 20000) )
              {
                delay(1);
                serialEvent();
                currentMillis = millis();
              }
              break;
            }
          case 7:
            {
              ResetNewBuffer();
              sendATreply("AT+CIPSHUT\r\n", "OK", 4000) ;
              modemStatus = 0;
              _tcpStatus = 2;
              ResetNewBuffer();
              break;
            }
        }
      }
  }

}

void _ping(void)
{
  if (pingFlag == 1)
  {
    unsigned long currentMillis = millis();
    if ((currentMillis - _PingPrevMillis ) >= _KeepAliveTimeOut * 1000)
    {
      // save the last time you blinked the LED
      _PingPrevMillis = currentMillis;
      char c = (char) (PINGREQ * 16);
      PutCharacter(c);
      _sendLength(0);
    }
    
    if (end!=0)
        delay(1000);
        serialEvent();
  }
}

void _sendUTFString(char *string)
{
  int localLength = strlen(string);
  char c = (char) (localLength / 256);
  PutCharacter(c);
  char d = (char) (localLength % 256);
  PutCharacter(d);
  WriteString(string);
}
void _sendLength(int len)
{
  int  length_flag = 0;
  while (length_flag == 0)
  {
    if ((len / 128) > 0)
    {
      char c = (char) (len % 128 + 128);
      PutCharacter(c);
      len /= 128;
    }
    else
    {
      length_flag = 1;
      char d = (char) len;
      PutCharacter(d);
    }
  }
}
void connect(char *ClientIdentifier, char UserNameFlag, char PasswordFlag, char *UserName, char *Password, char CleanSession, char WillFlag, char WillQoS, char WillRetain, char *WillTopic, char *WillMessage)
{
  ConnectionAcknowledgement = NO_ACKNOWLEDGEMENT;
  char c = (char) (CONNECT * 16);
  PutCharacter(c);
  char ProtocolName[7] = "MQIsdp";
  int localLength = (2 + strlen(ProtocolName)) + 1 + 3 + (2 + strlen(ClientIdentifier));
  if (WillFlag != 0)
  {
    localLength = localLength + 2 + strlen(WillTopic) + 2 + strlen(WillMessage);
  }
  if (UserNameFlag != 0)
  {
    localLength = localLength + 2 + strlen(UserName);

    if (PasswordFlag != 0)
    {
      localLength = localLength + 2 + strlen(Password);
    }
  }
  _sendLength(localLength);
  _sendUTFString(ProtocolName);
  PutCharacter((char)_ProtocolVersion);
  PutCharacter((char)(UserNameFlag * User_Name_Flag_Mask + PasswordFlag * Password_Flag_Mask + WillRetain * Will_Retain_Mask + WillQoS * Will_QoS_Scale + WillFlag * Will_Flag_Mask + CleanSession * Clean_Session_Mask));
  PutCharacter((char)(_KeepAliveTimeOut / 256));
  PutCharacter((char)(_KeepAliveTimeOut % 256));
  _sendUTFString(ClientIdentifier);
  if (WillFlag != 0)
  {
    _sendUTFString(WillTopic);
    _sendUTFString(WillMessage);
  }
  if (UserNameFlag != 0)
  {
    _sendUTFString(UserName);
    if (PasswordFlag != 0)
    {
      _sendUTFString(Password);
    }
  }
}
void publish(char DUP, char Qos, char RETAIN, unsigned int MessageID, char *Topic, char *Message)
{
  ResetNewBuffer();
  PutCharacter((char)PUBLISH * 16 + DUP * DUP_Mask + Qos * QoS_Scale + RETAIN);
  int localLength = (2 + strlen(Topic));
  if (Qos > 0)
  {
    localLength += 2;
  }
  localLength += strlen(Message);
  _sendLength(localLength);
  _sendUTFString(Topic);
  if (Qos > 0)
  {
    PutCharacter((char)(MessageID / 256));
    PutCharacter((char)(MessageID % 256));
  }
  WriteString(Message);
  ResetNewBuffer();
}
void publishACK(unsigned int MessageID)
{
  ResetNewBuffer();
  PutCharacter((char)(PUBACK * 16));
  _sendLength(2);
  PutCharacter((char)(MessageID / 256));
  PutCharacter((char)(MessageID % 256));
  ResetNewBuffer();
}
void publishREC(unsigned int MessageID)
{
  ResetNewBuffer();
  PutCharacter((char)(PUBREC * 16));
  _sendLength(2);
  PutCharacter((char)(MessageID / 256));
  PutCharacter((char)(MessageID % 256));
  ResetNewBuffer();
}
void publishREL(char DUP, unsigned int MessageID)
{
  ResetNewBuffer();
  PutCharacter((char)(PUBREL * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  _sendLength(2);
  PutCharacter((char)(MessageID / 256));
  PutCharacter((char)(MessageID % 256));
  ResetNewBuffer();
}

void publishCOMP(unsigned int MessageID)
{
  ResetNewBuffer();
  PutCharacter((char)(PUBCOMP * 16));
  _sendLength(2);
  PutCharacter((char)(MessageID / 256));
  PutCharacter((char)(MessageID % 256));
  ResetNewBuffer();
}
void subscribe(char DUP, unsigned int MessageID, char *SubTopic, char SubQoS)
{
  ResetNewBuffer();
  PutCharacter((char)(SUBSCRIBE * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  int localLength = 2 + (2 + strlen(SubTopic)) + 1;
  _sendLength(localLength);
  PutCharacter((char)(MessageID / 256));
  PutCharacter((char)(MessageID % 256));
  _sendUTFString(SubTopic);
  PutCharacter(SubQoS);
  ResetNewBuffer();

}
void unsubscribe(char DUP, unsigned int MessageID, char *SubTopic)
{
  ResetNewBuffer();
  PutCharacter((char)UNSUBSCRIBE * 16 + DUP * DUP_Mask + 1 * QoS_Scale);
  int localLength = (2 + strlen(SubTopic)) + 2;
  _sendLength(localLength);

  PutCharacter((char)(MessageID / 256));
  PutCharacter((char)(MessageID % 256));

  _sendUTFString(SubTopic);
  ResetNewBuffer();
}
void disconnect(void)
{
  ResetNewBuffer();
  PutCharacter((char)(DISCONNECT * 16));
  _sendLength(0);
  pingFlag = 0;
  ResetNewBuffer();
}
//Messages
const char * CONNECTMessage = "Client request to connect to Server\r\n";
const char * CONNACKMessage = "Connect Acknowledgment\r\n";
const char * PUBLISHMessage = "Publish message\r\n";
const char * PUBACKMessage = "Publish Acknowledgment\r\n";
const char * PUBRECMessage = "Publish Received (assured delivery part 1)\r\n";
const char * PUBRELMessage = "Publish Release (assured delivery part 2)\r\n";
const char * PUBCOMPMessage = "Publish Complete (assured delivery part 3)\r\n";
const char * SUBSCRIBEMessage = "Client Subscribe request\r\n";
const char * SUBACKMessage = "Subscribe Acknowledgment\r\n";
const char * UNSUBSCRIBEMessage = "Client Unsubscribe request\r\n";
const char * UNSUBACKMessage = "Unsubscribe Acknowledgment\r\n";
const char * PINGREQMessage = "PING Request\r\n";
const char * PINGRESPMessage = "PING Response\r\n";
const char * DISCONNECTMessage = "Client is Disconnecting\r\n";

void printMessageType(uint8_t Message)
{
  switch (Message)
  {
    case CONNECT:
      {
        int k, len = strlen(CONNECTMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(CONNECTMessage + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case CONNACK:
      {
        int k, len = strlen(CONNACKMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(CONNACKMessage + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case PUBLISH:
      {
        int k, len = strlen(PUBLISHMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBLISHMessage + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case PUBACK:
      {
        int k, len = strlen(PUBACKMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBACKMessage + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case  PUBREC:
      {
        int k, len = strlen(PUBRECMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBRECMessage + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case PUBREL:
      {
        int k, len = strlen(PUBRELMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBRELMessage + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case PUBCOMP:
      {
        int k, len = strlen(PUBCOMPMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBCOMPMessage  + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case SUBSCRIBE:
      {
        int k, len = strlen(SUBSCRIBEMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(SUBSCRIBEMessage  + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case SUBACK:
      {
        int k, len = strlen(SUBACKMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(SUBACKMessage  + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case UNSUBSCRIBE:
      {
        int k, len = strlen(UNSUBSCRIBEMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(UNSUBSCRIBEMessage  + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case UNSUBACK:
      {
        int k, len = strlen(UNSUBACKMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(UNSUBACKMessage  + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case PINGREQ:
      {
        int k, len = strlen(PINGREQMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PINGREQMessage + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case PINGRESP:
      {
        int k, len = strlen(PINGRESPMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PINGRESPMessage + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case DISCONNECT:
      {
        int k, len = strlen(DISCONNECTMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(DISCONNECTMessage + k);
          //PutCharacter(myChar);
        }
        break;
      }
  }
}

//Connect Ack
const char * ConnectAck0 = "Connection Accepted\r\n";
const char * ConnectAck1 = "Connection Refused: unacceptable protocol version\r\n";
const char * ConnectAck2 = "Connection Refused: identifier rejected\r\n";
const char * ConnectAck3 = "Connection Refused: server unavailable\r\n";
const char * ConnectAck4 = "Connection Refused: bad user name or password\r\n";
const char * ConnectAck5 = "Connection Refused: not authorized\r\n";

void printConnectAck(uint8_t Ack)
{
  switch (Ack)
  {
    case 0:
      {
        int k, len = strlen(ConnectAck0);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck0 + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case 1:
      {
        int k, len = strlen(ConnectAck1);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck1 + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case 2:
      {
        int k, len = strlen(ConnectAck2);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck2 + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case 3:
      {
        int k, len = strlen(ConnectAck3);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck3 + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case 4:
      {
        int k, len = strlen(ConnectAck4);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck4 + k);
          //PutCharacter(myChar);
        }
        break;
      }
    case 5:
      {
        int k, len = strlen(ConnectAck5);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck5 + k);
          //PutCharacter(myChar);
        }
        break;
      }
  }
}
unsigned int _generateMessageID(void)
{
  if (_LastMessaseID < 65535)
  {
    return ++_LastMessaseID;
  }
  else
  {
    _LastMessaseID = 0;
    return _LastMessaseID;
  }
}
void processing(void)
{
  if (TCP_Flag == 0)
  {
    MQTT_Flag = 0;
    _tcpInit();
  }
  _ping();
}
int available(void)
{
  return MQTT_Flag;
}
void serialEvent()
{
    while(end!=0)//if there is data available
    {
        char inChar = newBuffer[front];
        ++front; //advance front by one
        --end; //line gets shorter by 1
        if (TCP_Flag == 0)
        {
            if (myIndex < 200)
            {
                inputString[myIndex++] = inChar;
            }
            if (inChar == '\n')
            {
                inputString[myIndex] = 0;
                stringComplete = 1;
                //WriteString(inputString);
                if (strstr(inputString, reply) != NULL)
                {
                    GSM_ReplyFlag = 1;
                    if (strstr(inputString, " INITIAL") != 0)
                    {
                        GSM_ReplyFlag = 2; //
                    }
                    else if (strstr(inputString, " START") != 0)
                    {
                        GSM_ReplyFlag = 3; //
                    }
                    else if (strstr(inputString, "IP CONFIG") != 0)
                    {
                        delay(10);
                        GSM_ReplyFlag = 4;
                    }
                    else if (strstr(inputString, " GPRSACT") != 0)
                    {
                        GSM_ReplyFlag = 4; //
                    }
                    else if ((strstr(inputString, " STATUS") != 0) || (strstr(inputString, "TCP CLOSED") != 0))
                    {
                        GSM_ReplyFlag = 5; //
                    }
                    else if (strstr(inputString, " TCP CONNECTING") != 0)
                    {
                        GSM_ReplyFlag = 6; //
                    }
                    else if ((strstr(inputString, " CONNECT OK") != 0) || (strstr(inputString, "CONNECT FAIL") != NULL) || (strstr(inputString, "PDP DEACT") != 0))
                    {
                        GSM_ReplyFlag = 7;
                    }
                }
                else if (strstr(inputString, "OK") != NULL)
                {
                    GSM_Response = 1;
                }
                else if (strstr(inputString, "ERROR") != NULL)
                {
                    GSM_Response = 2;
                }
                else if (strstr(inputString, ".") != NULL)
                {
                    GSM_Response = 3;
                }
                else if (strstr(inputString, "CONNECT FAIL") != NULL)
                {
                    GSM_Response = 5;
                }
                else if (strstr(inputString, "CONNECT") != NULL)
                {
                    GSM_Response = 4;
                    TCP_Flag = 1;
                   // WriteString("TCP_Flag = True\r\n");
                    delay(10);
                    ResetNewBuffer();
                    AutoConnect();
                    ResetNewBuffer();
                    delay(6500);
                    pingFlag = 1;
                    tcpATerrorcount = 0;
                    
                }
                else if (strstr(inputString, "CLOSED") != NULL)
                {
                    GSM_Response = 4;
                    TCP_Flag = 0;
                    MQTT_Flag = 0;
                }
                myIndex = 0;
                inputString[0] = 0;
              
            }
        }
        else
        {
            uint8_t ReceivedMessageType = (inChar / 16) & 0x0F;
            uint8_t DUP = (inChar & DUP_Mask) / DUP_Mask;
            uint8_t QoS = (inChar & QoS_Mask) / QoS_Scale;
            uint8_t RETAIN = (inChar & RETAIN_Mask);
            if ((ReceivedMessageType >= CONNECT) && (ReceivedMessageType <= DISCONNECT))
            {
                int NextLengthByte = 1;
                length = 0;
                lengthLocal = 0;
                uint32_t multiplier=1;
                delay(2);
                char Cchar = inChar;
                while ( (NextLengthByte == 1) && (TCP_Flag == 1))
                {
                    if (end!=0)
                    {
                        inChar = newBuffer[front];
                        ++front;
                        --end;
                        //WriteString(inChar + "\r\n");
                        if ((((Cchar & 0xFF) == 'C') && ((inChar & 0xFF) == 'L') && (length == 0)) || (((Cchar & 0xFF) == '+') && ((inChar & 0xFF) == 'P') && (length == 0)))
                        {
                            myIndex = 0;
                            inputString[myIndex++] = Cchar;
                            inputString[myIndex++] = inChar;
                            TCP_Flag = 0;
                            MQTT_Flag = 0;
                            pingFlag = 0;
                           // WriteString("Disconnecting\r\n");
                        }
                        else
                        {
                            if ((inChar & 128) == 128)
                            {
                              length += (inChar & 127) *  multiplier;
                              multiplier *= 128;
                              //WriteString("More\r\n");
                            }
                            else
                            {
                                NextLengthByte = 0;
                                length += (inChar & 127) *  multiplier;
                                multiplier *= 128;
                            }
                        }
                    }
                }
                lengthLocal = length;
                //PutCharacter(length);
                //WriteString("\r\n");
                if (TCP_Flag == 1)
                {
                    printMessageType(ReceivedMessageType);
                    myIndex = 0L;
                    uint32_t a = 0;
                    while ((length-- > 0) && (end != 0))
                    {
                        inputString[(uint32_t)myIndex++] = newBuffer[front];
                        ++front;
                        --end;
                        delay(1);
                    }
                    //WriteString(" \r\n");
                    if (ReceivedMessageType == CONNACK)
                    {
                        ConnectionAcknowledgement = inputString[0] * 256 + inputString[1];
                        if (ConnectionAcknowledgement == 0)
                        {
                            MQTT_Flag = 1;
                            OnConnect();
                        }
                        printConnectAck(ConnectionAcknowledgement);
                    }
                    else if (ReceivedMessageType == PUBLISH)
                    {
                        uint32_t TopicLength = (inputString[0]) * 256 + (inputString[1]);
                        //WriteString("Topic : '");
                        PublishIndex = 0;
                        uint32_t iter;
                        for (iter = 2; iter < TopicLength + 2; iter++)
                        {
                            //PutCharacter(inputString[iter]);
                            Topic[PublishIndex++] = inputString[iter];
                        }
                        Topic[PublishIndex] = 0;
                        //WriteString("' Message :'");
                        TopicLength = PublishIndex;

                        PublishIndex = 0;
                        uint32_t MessageSTART = TopicLength + 2UL;
                        int MessageID = 0;
                        if (QoS != 0)
                        {
                            MessageSTART += 2;
                            MessageID = inputString[TopicLength + 2UL] * 256 + inputString[TopicLength + 3UL];
                        }
                        for (iter = (MessageSTART); iter < (lengthLocal); iter++)
                        {
                            //PutCharacter(inputString[iter]);
                            Message[PublishIndex++] = inputString[iter];
                        }
                        Message[PublishIndex] = 0;
                        //WriteString("'\r\n");
                        MessageLength = PublishIndex;
                        if (QoS == 1)
                        {
                          publishACK(MessageID);
                        }
                        else if (QoS == 2)
                        {
                          publishREC(MessageID);
                        }
                        OnMessage(Topic, TopicLength, Message, MessageLength);
                        MessageFlag = 1;
                    }
                    else if (ReceivedMessageType == PUBREC)
                    {
                        //WriteString("Message ID :");
                        publishREL(0, inputString[0] * 256 + inputString[1]);
                        PutCharacter(inputString[0] * 256 + inputString[1]);
                        //WriteString("\r\n");
                    }
                    else if (ReceivedMessageType == PUBREL)
                    {
                        //WriteString("Message ID :");
                        publishCOMP(inputString[0] * 256 + inputString[1]);
                        //PutCharacter(inputString[0] * 256 + inputString[1]);
                        //WriteString("\r\n");
                    }
                    else if ((ReceivedMessageType == PUBACK) || (ReceivedMessageType == PUBCOMP) || (ReceivedMessageType == SUBACK) || (ReceivedMessageType == UNSUBACK))
                    {
                        //WriteString("Message ID :");
                        //PutCharacter(inputString[0] * 256 + inputString[1]);
                        //WriteString("\r\n");
                    }
                    else if (ReceivedMessageType == PINGREQ)
                    {
                        TCP_Flag = 0;
                        pingFlag = 0;
                        //WriteString("Disconnecting\r\n");
                        sendATreply("AT+CIPSHUT\r\n", ".", 4000) ;
                        modemStatus = 0;
                    }
                }
            }
            else if ((inChar = 13) || (inChar == 10))
            {
            }
            else
            {
                //WriteString("Received :Unknown Message Type :");
                //PutCharacter(inChar);
                //WriteString("\r\n");
            }
        }
    }
    if (end==0) //if there's no more data to read
    {
        ResetNewBuffer();
    }
}
void MakePhoneCall()
{
    disconnect();
    delay(7000);
    ResetNewBuffer();
    WriteString("ATD5412952229;\r\n"); //calling Giselle's phone
    delay(16000);
    ResetNewBuffer();
    resetGSM();
    delay(1000);
    resetGSM();
    delay(1000);
    _tcpStatus = 2;
    pingFlag = 0;
    TCP_Flag = 0;
    modemStatus = 0;
    begin();
    
}