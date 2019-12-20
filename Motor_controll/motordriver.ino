#include <SPI.h>
#include "mcp_can.h"

const int pwm =  6;  //initializing pin as pwm
const int in_1 = 8 ; //initializing pin as logics
const int in_2 = 7 ;

const int in1 = 4 ;
const int in2 = 3 ;

const int pwm2 = 9; 


const int SPI_CS_PIN = 10;

MCP_CAN CAN(SPI_CS_PIN);  // Set CS pin

void setup()
{
    pinMode(pwm, OUTPUT);   // we have to set PWM pin as output
    pinMode(in_1, OUTPUT);  // Logic pins are also set as output
    pinMode(in_2, OUTPUT); 

    pinMode(pwm2, OUTPUT);  //we have to set PWM pin as output
    pinMode(in1, OUTPUT);   //Logic pins are also set as output
    pinMode(in2, OUTPUT);
    
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");

        delay(100);
    }

    Serial.println("CAN BUS Shield init ok!");
}

void loop()
{
    //For Clock wise motion , in_1 = High , in_2 = Low
    unsigned char len = 0;
    signed char buf[8];


    if(CAN_MSGAVAIL != CAN.checkReceive())
    {
        digitalWrite(in_1,LOW) ;
        digitalWrite(in1,LOW) ;
        digitalWrite(in_2,LOW) ;
        digitalWrite(in2,LOW) ;
    }
    

    if(CAN_MSGAVAIL == CAN.checkReceive()) // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        unsigned long canId = CAN.getCanId();

        int ia = buf[0];

        Serial.println(ia);

        // Drive forward:
        if( ia >0 && ia <=128)
        {
            CAN.readMsgBuf(&len, buf);
            int ia = buf[0];
            Serial.println(ia);

            digitalWrite(in_1,HIGH) ;
            digitalWrite(in_2,LOW) ;
            analogWrite(pwm,ia);

            digitalWrite(in1,HIGH) ;
            digitalWrite(in2,LOW) ;
            analogWrite(pwm2,ia);

            delay(50);
        }

        // No drive:
        if (ia == 0)
        {
            Serial.println(ia);
          
            CAN.readMsgBuf(&len, buf);
            int ia = buf[0];
          
            digitalWrite(in_1,LOW) ;
            digitalWrite(in_2,LOW) ;
            analogWrite(pwm,ia);

            digitalWrite(in1,LOW) ;
            digitalWrite(in2,LOW) ;
            analogWrite(pwm2,ia);

            delay(50);
        }

        // Drive backwards:
        if  (ia < 0 && ia >= -128)
        {
            CAN.readMsgBuf(&len, buf);
            int ia = -buf[0];
            Serial.println(ia);

              
            digitalWrite(in_1,LOW) ;
            digitalWrite(in_2,HIGH) ;
            analogWrite(pwm,ia);

            digitalWrite(in1,LOW) ;
            digitalWrite(in2,HIGH) ;
            analogWrite(pwm2,ia);

            delay(50);
        }
    }
}

