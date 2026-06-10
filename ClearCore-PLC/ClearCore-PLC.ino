/*
  ModbusTCP Server for ClearCore Arduino wrapper

  (c)2021 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <Ethernet.h>       // Ethernet library v2 is required

#include <ModbusAPI.h>
#include <ModbusTCPTemplate.h>
#include <Clearcore.h>

#define motor ConnectorM0

class ModbusEthernet : public ModbusAPI<ModbusTCPTemplate<EthernetServer, EthernetClient>> {};

const uint16_t REG = 39999;               // Modbus Hreg Offset maybe this should be 40000?
const int32_t showDelay = 500;   // Show result every n'th mellisecond

bool usingDhcp = false;
byte mac[] = { 0x24, 0x15, 0x10, 0xB0, 0x34, 0x61 }; // MAC address for your controller updated to actual clearcore
IPAddress server(192, 168, 1, 99); // The IP address will be dependent on your local network
IPAddress client(192,168,1,101); // Ip address of the PLC 
ModbusEthernet mb;               // Declare ModbusTCP instance
int test = 0; 
uint16_t test1[15]; 
int velocityLimit = 10000; // pulses per sec
int accelerationLimit = 100000; // pulses per sec^2
int amplitude = 10; //in mm
int speed = 10; //mm/s?
int dwell = 1; //in miliseconds
bool waitFlag = 0;
bool connectedFlag = 0;
int movementTable[] = {0,0,0,0};
int movement = 0;
int tableLength = sizeof(movementTable) / sizeof(movementTable[0]);
bool direction ; //1 equals positive 0 equals negative
bool homing = 0;
bool homed = 0;
long oldTime;
long newTime;


enum i_motor {
  empty = 0b0000000000000000,
  reset = 0b0000000000000001,
  fault = 0b0000000000000010,
  warning = 0b0000000000000100,
  home = 0b0000000000001000
};
enum i_motor motor_mask;

// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress server) {
  Serial.print("cbConn");
  Serial.println(server);
  connectedFlag = true;
  return true;
}

uint16_t cbLed(TRegister* reg, uint16_t val) {
  //Attach ledPin to LED_COIL register
  //Serial.print("cb led called  ");
  //Serial.println(val);
  return val;
}

void setup() {
    Serial.begin(9600);
    uint32_t timeout = 5000;
    uint32_t startTime = millis();
    while (!Serial && millis() - startTime < timeout)
        continue;

    // Get the Ethernet module up and running.
    if (usingDhcp) {
        int dhcpSuccess = Ethernet.begin(mac);
        if (dhcpSuccess)
            Serial.println("DHCP configuration was successful.");
        else {
            Serial.println("DHCP configuration was unsuccessful!");
            Serial.println("Try again using a manual configuration...");
            while (true)
                continue;
        }
    }
    else {
        Ethernet.begin(mac, server);
    }

    // Make sure the physical link is up before continuing.
    while (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("The Ethernet cable is unplugged...");
        delay(1000);
    }
    /////////////////////////////////////// Motor Setup ///////////////////////////////////////
    MotorMgr.MotorInputClocking(MotorManager::CLOCK_RATE_NORMAL);

    // Sets all motor connectors into step and direction mode.
    MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL,
                          Connector::CPM_MODE_STEP_AND_DIR);

    // Set the motor's HLFB mode to bipolar PWM
    motor.HlfbMode(MotorDriver::HLFB_MODE_STATIC);
    // Set the HFLB carrier frequency to 482 Hz
    motor.HlfbCarrier(MotorDriver::HLFB_CARRIER_482_HZ);

    // Sets the maximum velocity for each move



    // Set the maximum acceleration for each move
    motor.AccelMax(accelerationLimit);



/////////////////////////////////////////////////////END - Motor Setup ///////////////////////////////////////////////////



  mb.server();           // Act as Modbus TCP server
  mb.onConnect(cbConn);
  mb.addHreg(0x098,0,100);          // Expose Holding Register #100
  mb.addHreg(0x046,0,100);
  mb.addCoil(0,0,2);
  mb.onGetHreg(0x4A,cbLed,10); // this works but I don't understand yet
  mb.pullHreg(server,0x4A,0x4A); // stop = 0  start = 9999
  mb.pullHreg(server,0x4B,0x4B);// amplitude (mm)
  mb.pullHreg(server,0x4C,0x4C); // speed mm/s
  mb.pullHreg(server,0x4D,0x4D); //dwell time (ms)
  mb.pullHreg(server,0x4E,0x4E); // motor control bitmask See i_motor ENUM above
  mb.pullCoil(server,0,0); // 0= stop 1 = start
  mb.pullCoil(server,1,1); 

  if( mb.isConnected(server) ){
    Serial.println("Server started") ;
  }
  motor.EnableRequest(false);
  //Serial.println("Motor Enabled");
  
  //bool connect(client);// do we need a second connection to the client?

}
 int i = 0;
void loop() {

 //mb.Hreg(0x098,1234);
if (i>=15){
  i= 0;
}
  mb.task();  

  //Serial.println(mb.Hreg(0x4A));
 // motor.Move(mb.Hreg(0x4A)*300, MotorDriver::MOVE_TARGET_ABSOLUTE);
  amplitude = mb.Hreg(0x4B);
  speed = mb.Hreg(0x4C);
  dwell = mb.Hreg(0x4D);
 // Serial.println(movementTable[3]);
    velocityLimit = speed * 6400; // This assumes a 1mm per rotation reduction
		movementTable[0] = 0;
		movementTable[3] = (amplitude*6400)/2;
		movementTable[1] = (amplitude*6400)/2;
    movementTable[2] = amplitude*6400;
    
        // Sets the maximum velocity for each move
    motor.VelMax(velocityLimit);
    // Set the maximum acceleration for each move
    motor.AccelMax(accelerationLimit);
  /*
  Serial.print(0x040+i );
  Serial.print (" :=  ");
  Serial.print(test);
  Serial.println(test1[i]);
  */
    if( i<15){
    //test = mb.readHreg(server,0x040,test1,10); // this doesn't work
    //Serial.print(0x040+i );
    //Serial.print (" :=  ");
    //Serial.print(test);    
    mb.Hreg(0x098,1234+i);
    i++;

  }   
  if(movement+1 >= tableLength){
			if(movementTable[movement]-movementTable[0] >0){
				direction =1;
			}else{
				direction = 0;
			}
		}else{
			if(movementTable[movement]-movementTable[movement+1] >0 ){
					direction =1;
			}else {
					direction =0;
			}

		}
 // Serial.println(mb.Coil(0));
  if (connectedFlag && (mb.Hreg(0x4A) > 200)){
    motor.EnableRequest(true);
		if(motor.StepGenerator::StepsComplete()){
      Serial.println("Step generator empty");
			if ((movement == 0 || movement == 2) && !waitFlag){
				waitFlag = true;
        delay(dwell);
			}
			movement = movement + 2;
			if (movement >= tableLength){
				movement = 0;				
			}
      newTime = millis();
      if (newTime-oldTime >= 1000){
          oldTime = newTime;
          waitFlag = 0;
        }
      if(!waitFlag){
        Serial.print("Move commanded to ");
        Serial.println((movementTable[movement]));
			motor.Move((movementTable[movement]),MotorDriver::MOVE_TARGET_ABSOLUTE);  
      } else {
        Serial.println("Waiting on dwell");
      }
      }
      }              // Common local Modbus task

      //Motor control enum
      /*
      Serial.print ( "Hreg = ");
      Serial.print (mb.Hreg(0x4E));
      Serial.print ("    mask = ");
      Serial.println(motor_mask = reset);

      */
      if (mb.Hreg(0x4E) & (motor_mask = reset)){
          Serial.println("Motor Reset Request");
          motor.EnableRequest(false);
      } else if(homed)
      {
        motor.EnableRequest(true);
      }

      if (mb.Hreg(0x4E)& (motor_mask = home)){
        motor.EnableRequest(false);
        homing = 1;
      }

      if(homing){
        Serial.println("starting Home");
        motor.EnableRequest(true);
        if (motor.HlfbHasRisen()){
          homing=0;
          homed = 1;
          Serial.println("home done");
        }
      }


 //delay(100);
}