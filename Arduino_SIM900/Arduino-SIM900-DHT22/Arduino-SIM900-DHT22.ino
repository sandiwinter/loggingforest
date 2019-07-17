#include <avr/wdt.h> //should be in any adruino IDE
#include <SoftwareSerial.h>
#include "DHT.h"

SoftwareSerial GPRS(2, 3); // RX, TX

#define GSM_ON              7 // connect GSM Module turn ON to pin 77 7 on my device
#define GSM_RESET           8 // connect GSM Module RESET to pin 35
#define DEBUG_EN            true // enable debugging
#define DHTPIN 10           // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// HTTPRequestInterval
unsigned long interval_1=60000; // the time we need to wait
unsigned long delayMillis_1=0; // millis() returns an unsigned long.
unsigned long delayMillisSuccess_1 = 0;

unsigned long resetMilis = 100000; //2147483647;

String apn = "gprs0.vipnet.hr"; // get device_key and 
const char* device_key = "YOUR_DEVICE_KEY"; // get device_key and 
const char* secret_key = "YOUR_DEVICE_PASSWORD"; // secret_key at http://loggingforest.com
String host = "loggingforest.com";

void setup()
{
  GPRS.begin(9600);
  if(DEBUG_EN)Serial.begin(9600);
  if(DEBUG_EN)Serial.println("PROGRAM START ::::::::::");

  blink(4);
  
  // Power on SIM900 module
  pinMode(GSM_ON, OUTPUT);
  digitalWrite(GSM_ON, HIGH);
  delay(500);
  digitalWrite(GSM_ON, LOW);  
  delay(20000);  
  // end

  //GPRS.write( 0x1a );
  //delay(2000);
  ifReceived("Call Ready", 20);  

  if(DEBUG_EN)Serial.println("TEST AT");
  GPRS.write("AT\r");
  if(ifReceived("OK", 30) == false)
    softReset();

// [START] select mobile network manually

  if(DEBUG_EN)Serial.println("SELECT NETWORK MANUALLY");

//print available networks
  if(DEBUG_EN)
  {
    GPRS.write("AT+COPS=?\r");
    ifReceived("OK", 20);
  }
  
  //GPRS.write("AT+COPS=1,2,\"21901\"\r"); // VIP=21910, BONBON=21901
  //if(ifReceived("OK", 20) == false)
  //  softReset();

  if(DEBUG_EN)Serial.println("NETWORK SELECTED");

// [END] select mobile network manually

/* VIP Mobile network PODACI
  GPRS.write("AT+SAPBR=3,1,\"APN\",\"gprs0.vipnet.hr\"\r");
  ifReceived("OK", 10);
  GPRS.write("AT+SAPBR=3,1,\"USER\",\"38591\"\r");
  ifReceived("OK", 10);
  GPRS.write("AT+SAPBR=3,1,\"PWD\",\"38591\"\r");
  ifReceived("OK", 10);
*/

// [START] t-mobile network data
  //GPRS.write("AT+SAPBR=3,1,\"APN\",\"web.htgprs\"\r");
  //GPRS.write("AT+SAPBR=3,1,\"APN\",\"gprs0.vipnet.hr\"\r");

  String call_apn = "AT+SAPBR=3,1,\"APN\",\""+apn+"\"\r";
  GPRS.print(call_apn);
  
  ifReceived("OK", 10);
  GPRS.write("AT+SAPBR=3,1,\"USER\",\"\"\r");
  ifReceived("OK", 10);
  GPRS.write("AT+SAPBR=3,1,\"PWD\",\"\"\r");
  ifReceived("OK", 10);
// [END] t-mobile network data

  GPRS.write("AT+SAPBR=0,1\r"); // ovo zna LOOP FAILED vratiti
  ifReceived("OK", 10);
  
  GPRS.write("AT+SAPBR=1,1\r"); // ovo zna LOOP FAILED vratiti
  ifReceived("OK", 10);

// [START] DEL all sms messages

  GPRS.write("AT+CMGDA=\"DEL ALL\"\r");
  ifReceived("OK", 10);

// [END] DEL all sms messages

  dht.begin();
}

void loop()
{
  blink(1);
  if(DEBUG_EN)Serial.println("LOOP START");

  unsigned long currentMillis = millis(); // grab current time

// HANG CALL IF AVAILABLE
  GPRS.write("ATH\r");
  ifReceived("OK", 10);

// [SEND DATA INTERVAL]
  if (millis() > delayMillis_1 )
  {
    sendHTTPData();
    //if(DEBUG_EN)Serial.println("sendHTTPData()");
    delayMillis_1 = millis()+interval_1;
  }
  else
  {
    if(DEBUG_EN)Serial.println((delayMillis_1-millis())
                              /1000);
  }
// [/SEND DATA INTERVAL]

// RESET if 3 HTTP requests failed
  if(delayMillisSuccess_1 < millis() && 
     delayMillisSuccess_1 != 0)
     softReset();

// RESET after half of millis() lifetime
  if(millis() > resetMilis)
    softReset();

  if(DEBUG_EN)Serial.println(delayMillisSuccess_1/1000);

// Dump remaining data to clear buffers
  dumpData();
  
  if(DEBUG_EN)Serial.println("LOOP END");
  blink(1);
  delay(1000);
}

void sendHTTPData()
{
  // [READ SENSOR]
  float humd = dht.readHumidity();
  float temp = dht.readTemperature();
  long tmp_s = (long) millis() / 1000;
  
  if(humd > 100 || temp > 100 || isnan(humd) || isnan(temp)){
    if(DEBUG_EN)Serial.println("SENSOR FAILED");
  }
  else
  {
    if(DEBUG_EN)Serial.println(String(humd)+"% "+String(temp)+"C");
  }
  // [/READ SENSOR]

  // [SEND DATA]

  // We now create a URI for the request
  String url = "/index.php/api";
  url += "?device_key=";
  url += device_key;
  url += "&secret_key=";
  url += secret_key;
  url += "&milis=";
  url += tmp_s;
  url += "&temp=";
  url += temp;
  url += "&hum=";
  url += humd; 
  
  //GET /api.php?data=dev:329;rt:10;h:10;t:10
  String call = "AT+HTTPPARA=\"URL\",\""+host+url+"\"\r";
  
  // send data
  GPRS.write("AT+HTTPINIT\r");
  ifReceived("OK", 10);

  GPRS.write("AT+HTTPPARA=\"CID\",1\r");
  ifReceived("OK", 10);

  GPRS.print(call.substring(0, 50) );
  GPRS.println(call.substring(50) );  
  ifReceived("OK", 10);
  
  GPRS.write("AT+HTTPACTION=0 \r");
  ifReceived("200", 130);

  //read results
  GPRS.write("AT+HTTPREAD=0,100 \r");
  if(ifReceived("RESULT", 20) == true)
  {
      delayMillisSuccess_1 = millis()+interval_1*3;
      blink(3);
  }
  
  //term connection
  GPRS.write("AT+HTTPTERM\r");
  ifReceived("OK", 10);

  // [/SEND DATA]
}

void dumpData()
{
    if(DEBUG_EN)Serial.println("DATA DUMP:");
    while(GPRS.available())
    {
      char c = GPRS.read();
      if(DEBUG_EN)Serial.print(c);
    }  
}

boolean ifReceived(String check, unsigned long timeout)
{
  String received = String("");

  boolean av=false;
  
  timeout=1000*timeout;
  if(timeout<10000)timeout=10000;
  unsigned long dmilis=millis()+timeout;
  
  while(dmilis > millis())
  {
    while(GPRS.available())
    {
      char c = GPRS.read();
      received+=c;
      av=true;
    }

    // ON WANTED OR ERROR brean immediately
    if(received.indexOf(check) > 0 || 
       received.indexOf("ERROR") > 0)break;
  }

  if(DEBUG_EN)Serial.println(received);
  
  if(received.indexOf(check) > 0)
    return true;

  if(received.indexOf("ERROR") > 0 || 
     received.indexOf("OK") > 0)
    return false;

  // serial buffer size fix
  if(received.length() > 60)
  {
    if(DEBUG_EN)Serial.println("BUFFER OVERFLOW");
    return false;
  }
    
  if(DEBUG_EN && false)
  {
    Serial.println("LOOP, FAILED");
    softReset();
    /*
    while(true){
      delay(500);
    }
    */
  }
  
  return false;  
}

void softReset(){
  delay(5000);

  GPRS.write("AT+CPOWD=1\r");
  ifReceived("POWER DOWN", 20);
  delay(5000);
  
  if(DEBUG_EN)Serial.println("softReset");
  wdt_enable(WDTO_1S); //enable it, and set it to 8s
  while(1){}
  //asm volatile ("  jmp 0");
}

void blink(int n)
{
  pinMode(13, OUTPUT);
  
  for(int i=0; i<n; i++)
  {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200); 
  }
}

/*
// If you want to replace asterisks with ctrl+Z you can
// replace the second part above with this instead

  while (Serial.available()) {
    byte b = Serial.read();
    if ( b == '*')
      GPRS.write( 0x1a );         // replace * with ctrl+z
    else
      GPRS.write(b);
  }

*/
