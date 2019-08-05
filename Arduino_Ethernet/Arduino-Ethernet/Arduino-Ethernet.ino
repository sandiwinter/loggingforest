
#include<stdlib.h>
#include<avr/wdt.h>

#include "DHT.h"
#define DHTPIN 3     // what pin we're connected to
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301, AM2305)
DHT dht(DHTPIN, DHTTYPE);

#include <SPI.h>
#include <Ethernet.h>
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
  
//IPAddress ip(192,168,1,14);

IPAddress ip();
EthernetClient client;
const long requestInterval = 40000;  // delay between requests
char serverName[] = "loggingforest.com";  // API URL
char device_key[] = "YOUR_DEVICE_KEY";  // API URL
char secret_key[] = "YOUR_DEVICE_PASSWORD";  // API URL
boolean requested;                   // whether you've made a request since connecting
long lastAttemptTime = 0;            // last time you connected to the server, in milliseconds
long lastSuccessTime = 0;
boolean lastConnected = false;

//String temp_data[100];
//int tmp_data_index = 0;

boolean serialEnable = true;
boolean wdtEnable = false;

void setup() {
  if(serialEnable){
    Serial.begin(9600); 
    //while (! Serial); // Wait until Serial is ready - Leonardo
    Serial.println("Logger start...");
  }

  /* DHT sensor start */
  dht.begin();
  
  /* Ethernet start */
  pinMode(2, OUTPUT);
  // attempt a DHCP connection:
  if ( !Ethernet.begin(mac) ) {
    // if DHCP fails, start with a hard-coded address:
    //Ethernet.begin(mac, ip);
    if(serialEnable)Serial.println("DHCP fails, wait 20sec...");
    delay(20000);
    softReset();
    //softReset();
  }
  if(wdtEnable)wdt_enable(WDTO_8S);
  
  // print your local IP address:
  if(serialEnable)Serial.println(Ethernet.localIP());
  
  lastAttemptTime = millis()-requestInterval; 
  lastSuccessTime = millis()/1000;
}

void loop() {
  //wdt_reset();
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
    if(serialEnable)Serial.println("Failed to read from DHT");
  } else {
    if(serialEnable)Serial.print("Humidity: "); 
    if(serialEnable)Serial.print(h);
    if(serialEnable)Serial.print(" %\t");
    if(serialEnable)Serial.print("Temperature: "); 
    if(serialEnable)Serial.print(t);
    if(serialEnable)Serial.println(" *C");
    if(serialEnable)Serial.println(millis() - lastAttemptTime);

    if (millis() - lastAttemptTime > requestInterval) {
      sendDataToServer(h,t);
    }
  }
  
  //wdt_reset(); 
  delay(3000);
  //wdt_reset(); 
  delay(3000);
  //wdt_reset(); 
  delay(1000);
  //wdt_reset();
  
  if(millis()/1000 - lastSuccessTime > 300
    || millis()/1000 - lastSuccessTime < 0)
  {
      softReset();
  }
}

//void zero_fillchar *tmp, int len)
//{
//  int j=0;
//  for(j=0;j<len;j++)
//  {
//    if(tmp[j]==' ')tmp[j]='0';
//  }
//}

void zero_fill(char *tmp, int len)
{
  int j=0;
  int space = 0;
  for(j=0;j<len;j++)
  {
    if(tmp[j]==' ')space++;
  }
  
  for(j=0;j<len;j++)
  {
    if(j+space < len)
      tmp[j] = tmp[j+space];
  }
  
  tmp[j] = '\0';
}

void sendDataToServer(float h, float t) 
{

  char tmp_t[20];
  dtostrf(t,5,2,tmp_t);
  zero_fill(tmp_t, 6);
  
  char tmp_h[20];
  dtostrf(h,5,2,tmp_h);
  zero_fill(tmp_h, 6);
  
  long tmp_s = (long) millis() / 1000;
  
  IPAddress ipadr = Ethernet.localIP();
  //char *tmp_ip;
  //ipadr.printTo(tmp_ip);

  char strGet[300];
  sprintf(strGet, "GET /index.php/api?device_key=%s&secret_key=%s&milis=100&temp=%s&hum=%s; HTTP/1.1", device_key, secret_key, tmp_t, tmp_h);  
  //wdt_reset(); 
  if (client.connect(serverName, 80)) { // ovo prijede WDT vrijeme i restarta sve
    //wdt_reset(); 
    // attempt to connect, and wait a millisecond:
    if(serialEnable)Serial.println("connecting to server...");
    if(serialEnable)Serial.println("making HTTP request...");
    if(serialEnable)Serial.println(strGet);
    // make HTTP GET request
    client.println(strGet);
    client.println("HOST: loggingforest.com");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();
    
    //wdt_reset(); 
    delay(1000);
    //wdt_reset(); 
    
    String page = String("test");
    while(client.available()) {
      char c = client.read();
      page+=String(c);
    }
    if(page.indexOf(String("RESULT")) > 0)
    {
      if(serialEnable)Serial.println(page);
      if(serialEnable)Serial.println("Result: true");
      lastSuccessTime = millis()/1000;
    }
    else
    {
      if(serialEnable)Serial.println(page);
      if(serialEnable)Serial.println("Result: false");
    }
    
    if(serialEnable)Serial.println("disconnecting.");
    client.stop();
  }
  else {
    // if you couldn't make a connection:
    if(serialEnable)Serial.println("connection failed");
    if(serialEnable)Serial.println("disconnecting.");
    client.stop();
  }
   
  // note the time of this connect attempt:
  lastAttemptTime = millis();
}

void softReset(){
  int pin=8;
  pinMode(pin, OUTPUT);      // sets the digital pin as output
  delay(2000);
  digitalWrite(pin, LOW);    // sets the LED off
  delay(2000);
  digitalWrite(pin, HIGH);    // sets the LED off
  
  wdt_enable(WDTO_2S);
  if(serialEnable)Serial.println("reseting");
  while(1){}
  //asm volatile ("  jmp 0");
}

void software_Reboot()
{
  //wdt_enable(WDTO_15MS);
  //while(1)
  //{
  //}
}

// you can use a wire to connect, via a 1K resistor,
// a Digital Pin (for example pin12) to the Reset Pin.
// Then you can use the following code: 
void swhwReset()
{
  //int pin=12;
  //pinMode(pin, OUTPUT);      // sets the digital pin as output
  //digitalWrite(ledPin, LOW);    // sets the LED off
}



