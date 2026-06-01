/*
  ModbusTCP Server for ClearCore Arduino wrapper

  (c)2021 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <Ethernet.h>       // Ethernet library v2 is required

#include <ModbusAPI.h>
#include <ModbusTCPTemplate.h>

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


// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress server) {
  Serial.print("cbConn");
  Serial.println(server);
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



  mb.server();           // Act as Modbus TCP server
  mb.onConnect(cbConn);
  mb.addHreg(0x098,0,100);          // Expose Holding Register #100
  mb.addHreg(0x046,1,100);
  mb.onGetHreg(0x4A,cbLed,10); // this works but I don't understand yet
  mb.pullHreg(server,0x4A,0x4A);
  if( mb.isConnected(server) ){
    Serial.println("Server started") ;
  }
  //bool connect(client);// do we need a second connection to the client?

}
 int i = 0;
void loop() {

 //mb.Hreg(0x098,1234);
if (i>=15){
  i= 0;
}
  mb.task();  
  
  Serial.println(mb.Hreg(0x4A));
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

  }                    // Common local Modbus task
 delay(100);
}