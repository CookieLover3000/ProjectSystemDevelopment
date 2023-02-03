/*libraries*/
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <math.h>
#include <Arduino.h>
/*port socket connection*/
#define PORT 8080
/*i2c pins*/
#define I2C_SDL D1
#define I2C_SDA D2
/*server communicatie*/
bool id_sent = false;
bool id_accepted = false;
long serverMillisStart = 0;
String ID = "KOELKAST_ID";
/*cooler open mechanisme*/
bool xopen = false;
bool xlangopen = false;
bool xkortopen = false;
unsigned long startMillis;
const unsigned long period = 10000;
const unsigned long longperiod = 15000;
/*Steinhart-Hart and Hart Coefficients*/
const float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
/*krijgt data van de server*/
bool newData = false;
const byte numChars = 32;
char receivedChars[numChars];
bool fanserver;
/*om de temperatuur naar de server te sturen elke 2s*/
long celciusTimer = 0;
/*huidige tijd*/
unsigned long currentMillis;
/*knop*/
bool switchbutton;

WiFiClient client;

/*koelkast control en data*/
void knop();
void leesTMPsensoren();
void fanAanOfUit();
void vertaler(int Anin0);
void koelkastTeLangOpen();
/*server client functions*/
void initWiFi();
void leesbericht();
void verwerkData();
void connectToServer();

void setup()
{
  Wire.begin();
  Serial.begin(115200); 
  Serial.println("Starting I2C setup");
  /*data direction register*/
  Wire.beginTransmission(0x38); 
  Wire.write(0x03);
  Wire.write(0b00001111);
  Wire.endTransmission();
  
  pinMode(D5, OUTPUT);
  /*stuurt setup en config byte*/
  Wire.beginTransmission(0x36); 
  Wire.write(byte(0xA2));
  Wire.write(byte(0x03));
  Wire.endTransmission();
  Serial.println("Finished I2C setup, starting Wifi connection...");
  
  initWiFi();
  /*zet timers*/
  celciusTimer = millis();
  currentMillis = millis() +3000; //to immediately give 
  
}
void loop()
{ /*maakt en onderhoudt de connectie met de server*/
  connectToServer();
  /*leest berichten verzonden door de server*/
  leesbericht();
  /*interpreteerd berichten van de server*/
  verwerkData();
  /*stuurt de temperatuur in C naar de server*/
  if(currentMillis - celciusTimer > 2000){
  leesTMPsensoren();
  celciusTimer = millis();}
  /*leest de knop wat de koelkast deur is*/
  knop();
  /*stuurt de fan aan*/
  fanAanOfUit();
  /*zet een variabel gelijk aan de huidige tijd*/
  currentMillis = millis();
  /*Stuurt bericht naar de server als die te lang open is*/
  koelkastTeLangOpen();
  /*overheat prevention*/
  delay(100);
}

void knop()
{//gebruikt I2C om de knop uit te lezen
  Wire.beginTransmission(0x38);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x38, 1);
  int inputs = Wire.read();
  inputs &= 0x01;
  //if de knop is ingedrukt
  if (inputs)
  {//if zorgt dat er maar één bericht wordt gestuurd als de koelkast open is
    if (xopen && xlangopen)//en if de koelkast open was for 15s of meer
    {
      client.println("Koelkast dicht");
      client.println("Balie uit");
      xopen = false;
      xlangopen = false;
    }
    else if (xopen && !xlangopen) // en if de koelkast open was voor 10s tot 15s
    {
      client.println("Koelkast dicht");
      xopen = false;
    }
    else if (!xopen && xkortopen) //en als de koelkast maar even open was tot 10s
    {
      client.println("Koelkast dicht");
      xkortopen = false;
    }
    startMillis = millis(); //if de koelkast gesloten is wordt deze tijd gelijk gesteld aan de huidige
  }
  else
  {//if de knop is niet ingedrukt wat betekent dat de koelkast open is
    if (!xkortopen)//if zorgt voor maar één bericht naar de server
    {
      client.println("Koelkast open");
      xkortopen = true;
    }
  }
}

void leesTMPsensoren()
{
    // Leest analoge 10bit inputs over i2c
  Wire.requestFrom(0x36, 4);
  unsigned int anin0 = Wire.read() & 0x03;
  anin0 = anin0 << 8;
  anin0 = anin0 | Wire.read(); // inside
  unsigned int anin1 = Wire.read() & 0x03;
  anin1 = anin1 << 8;
  anin1 = anin1 | Wire.read(); // outside
  Serial.println(anin1);
  Serial.println(anin0);
  client.print("Buiten: ");
  vertaler(anin1); //vertaald de waarde van analoog naar c
  client.print("Binnen: ");
  vertaler(anin0); //vertaald de waarde van analoog naar c
}

void fanAanOfUit()
{//wanneer de server bericht heeft gestuurd om de fan koelelement te besturen
  if (fanserver) //zet de fan en cooler aan
  {
    Wire.beginTransmission(0x38); //i2c
    Wire.write(0x01);
    Wire.write(0b00010000);
    Wire.endTransmission();
    digitalWrite(D5, HIGH); //zet de pin direct aan
  }
  else if (!fanserver) //zet de fan en cooler uit
  {
    Wire.beginTransmission(0x38); //i2c
    Wire.write(0x01);
    Wire.write(0b00000000);
    Wire.endTransmission();
    digitalWrite(D5, LOW); //zet de pin direct uit
  }
}

void vertaler(int Anin0)   
{//vertaald lezingen van de sensors naar c
  float getal = 10000 * (1023 / (float)Anin0 - 1); //rekent de weerstand in ohm uit
  float loggetal = log(getal); //neemt het logarite hiervan
  float T = (1.0 / (c1 + c2 * loggetal + c3 * loggetal * loggetal * loggetal));//rekent de temperatuur uit met behulp van de hart formule
  T -= 273.15; //tempratuur van k naar c
  String temp = String(T); //zet float om in string
  client.println(temp); //stuurt het op naar de server
}

void koelkastTeLangOpen() //stuurt bericht naar de server als de koelkast te lang open is 
{//stuurt bericht 1x na 10s
  if (currentMillis - startMillis >= period && !xopen)
  {
    client.println("10s open");
    xopen = true;
  }
  if (currentMillis - startMillis >= (longperiod) && xopen && !xlangopen)
  {//stuurt bericht 1x na 15s
    xlangopen = true;
    client.println("15s open");
  }
}

void initWiFi()
{
  WiFi.begin("Lab003", "Lab003WiFi");
  Serial.print("Connecting to wifi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(PORT);
}

void connectToServer()
{
  while (!client.connected()) //makes conenction and makes sure not to try again if there is already connection with the server
  {
    id_sent = false; // wordt gereset zodat als we verbinding met de server kwijt raken de identification opnieuw wordt gedaan
    id_accepted = false;
    Serial.println("No Connection");
    if (client.connect("voer hier ip van pi in", PORT))
      Serial.println("Server found");
    delay(100);
  }
  if (id_sent && millis() - serverMillisStart> 1000 && !id_accepted)
    client.stop();
}

void leesbericht()
{
  static boolean recvInProgress = false;
  static byte ndx;
  char rc;
  char startMarker = '<';
  char endMarker = '>';
  while (client.available() > 0 && newData == false)
  {
    rc = client.read();
    if (recvInProgress)
    {
      if (rc != endMarker)
      {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars)
          ndx = numChars - 1;
      }
      else
      {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }
    else if (rc == startMarker)
    {
      recvInProgress = true;
    }
  }
}

void verwerkData()
{
  if (newData == true)
  {
    Serial.println(receivedChars);
    if (!id_accepted)
    {
      if (strcmp(receivedChars, "id?") == 0)
      {
        client.println(ID);
        Serial.println("ID Sent");
        id_sent = true;
        serverMillisStart = millis();
      }
      else if (strcmp(receivedChars, "id_accept") == 0)
      {
        Serial.println("ID accepted");
        id_accepted = true;
        fanserver = true;
        digitalWrite(D5, HIGH);
    Serial.println("Fan start");
      }
    }//bericht van de server dat de fan en koelelement aan moeten
    if (strcmp(receivedChars, "Fan aan") == 0)
      fanserver = true;
     //bericht van de server dat de fan en koelelement aan moeten
    if (strcmp(receivedChars, "Fan uit")==0)
      fanserver = false;
    newData = false;
  }
}
