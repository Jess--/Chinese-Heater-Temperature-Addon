#include <SoftwareSerialWithHalfDuplex.h>
SoftwareSerialWithHalfDuplex sOne(2, 2); // NOTE TX & RX are set to same pin for half duplex operation
int inhibswitch = 8;
unsigned long lasttime;
String heaterstate[] = {"Off","Starting","Pre-Heat","Failed Start - Retrying","Ignition - Now heating up","Running Normally","Stop Command Received","Stopping","Cooldown"};
int heaterstatenum = 0;
int heatererror = 0;
String controlenable = "False";
String inhibseenactive = "False";
float currtemp = 0;
float settemp = 0;
float command = 0;
void setup()
{
 pinMode(inhibswitch,INPUT_PULLUP);
// initialize listening serial port
 // 25000 baud, Tx and Rx channels of Chinese heater comms interface:
 // Tx/Rx data to/from heater, special baud rate for Chinese heater controllers
 sOne.begin(25000);
 // initialise serial monitor on serial port 0
 Serial.begin(115200);
 // prepare for detecting a long delay
 lasttime = millis();

}

void loop()
{
 static byte Data[48];
 static bool RxActive = false;
 static int count = 0;


 
 // read from serial on D2
 if (sOne.available()) {

 // calc elapsed time since last rx’d byte to detect start of frame sequence
 unsigned long timenow = millis();
 unsigned long diff = timenow - lasttime;
 lasttime = timenow;

 if(diff > 100) { // this indicates the start of a new frame sequence
 RxActive = true;
 }
 int inByte = sOne.read(); // read hex byte
 if(RxActive) {
 Data[count++] = inByte;
 if(count == 48) {
 RxActive = false;
 }
 }
 }

 if(count == 48) { // filled both frames – dump
 count = 0;
command = int(Data[2]);
currtemp = Data[3];
settemp = Data[4];
heaterstatenum = int(Data[26]);
heatererror = int(Data[41]);

if (digitalRead(inhibswitch) == HIGH) {
  inhibseenactive = "True";
}

if (digitalRead(inhibswitch) == LOW && (inhibseenactive == "True")) {
  inhibseenactive = "False";
  Serial.println("Inhibit Switch Toggled disable-enable - Enabling Auto");
  controlenable = "True";
}

 Serial.println();
 Serial.print("Command ");
 Serial.println(int(command));
 Serial.println(heaterstate[heaterstatenum]);
 Serial.print("Error Code ");
 Serial.println(heatererror);
 Serial.print("Current Temp ");
 Serial.println(int(currtemp));
 Serial.print("Set Temp ");
 Serial.println(int(settemp));
 Serial.print("System Enabled : ");
 Serial.println(controlenable);

if (int(command) == 160) {
  Serial.println("Start command seen from controller - Enabling Auto");
  controlenable = "True";
}
if (int(command) == 5) {
  Serial.println("Stop command seen from controller - Disabling Auto");
  controlenable = "False";
}



  // Standard Range     Temp +2 to Temp -1 (3 degrees)

if (controlenable == "True") {
if (int(settemp) > int(currtemp) && (digitalRead(inhibswitch) == LOW) && (heatererror <= 1))  {
  Serial.println("Temperature Below Lower Limit");
  if (heaterstatenum == 0) {
uint8_t data1[24] = {0x78,0x16,0xa0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x32,0x08,0x23,0x05,0x00,0x01,0x2C,0x0D,0xAC,0x8D,0x82};
delay(50);
sOne.write(data1, 24);
  }
}

 if (int(settemp) < (int(currtemp) - 1) && (digitalRead(inhibswitch) == LOW)) {
  Serial.println("Temperature Above Upper Limit");
  if (heaterstatenum == 5) {
uint8_t data1[24] = {0x78,0x16,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x32,0x08,0x23,0x05,0x00,0x01,0x2C,0x0D,0xAC,0x61,0xD6};
delay(50);
sOne.write(data1, 24);
  }
}
}




 

 }
} // loop
