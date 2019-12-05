// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13

#include <SPI.h>
#include "mcp_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
#else
  #define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;
//const int LED        = 8;
//boolean ledON        = 1;

///////////////////////////////
#include <Servo.h>
Servo myservo;  // create servo object to control a servo
int can;    // variable from the can-buss
int dir;    // variable that control the servo
int temp = 50;   //a temp variable
///////////////////////////////


MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    SERIAL.begin(115200);     //make it posibol to print things


    /////////////////////////////////////////
    myservo.attach(6);  // attaches the servo on pin 9 to the servo object
    //myservo.write(90);   //sett servo to posetion 0
    ////////////////////////////////////////


    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println("Init CAN BUS Shield again");
        delay(100);
    }

    SERIAL.println("CAN BUS Shield init ok!");
}

void loop()
{
    signed char len = 0;
    signed char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned long canId = CAN.getCanId();

        SERIAL.println("-----------------------------");
        SERIAL.println("get data from ID: 0x");
        SERIAL.println(canId, HEX);

        ///////////////////////////////
        can = buf[1];                          //gets the secund value of the can-buss, witch is the direktion
        dir = map(can, -128, 127, 20, 80);     // scale it to use it with the servo (value between 0 and 180)

        SERIAL.print(can);
        SERIAL.println();
        SERIAL.print(dir);
        SERIAL.println();
        SERIAL.print(temp);

        if(temp > dir)
        {
          temp--;
          myservo.write(temp); 
        }
        
        if(temp < dir)
        {
          temp++;
          myservo.write(temp); 
        }
        
        delay(15);

        /*

        if(temp != dir)                       //chek if the char array have changest, if not do nothing, else change to position
        {

   
          if(temp > dir)                      //if the new command are bigger than the positon, go 1 stepp up at a time to the new position
          {
            for (int b = dir; b <= temp; b += 1) 
            {
           // in steps of 1 degree
           myservo.write(b);              // tell servo to go to position in variable 'dir'
           delay(15);                       // waits 15ms for the servo to reach the position
           }
           temp = dir;
          }
          
          if(temp < dir)                    //if the new command are smaller than the positon, go 1 stepp down at a time to the new position
          {
            for (int b = dir; b >= temp; b -= 1) 
            {
           // in steps of 1 degree
           myservo.write(b);              // tell servo to go to position in variable 'pos'
           delay(15);                       // waits 15ms for the servo to reach the position
           }
           temp = dir;
          }
        }

        */

///////////////////////////////////////////////////////////




        SERIAL.println();
    }
}

//END FILE
