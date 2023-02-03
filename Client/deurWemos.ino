#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Servo.h>

#define PORT 8080
#define I2C_SDL D1
#define I2C_SDA D2

void initDeur();
void initWiFi();
void leesKnop();
void leesbericht();
void verwerkData();
void connectToServer();

Servo myservo;
WiFiClient client;

/* voor het verbinden met de server */
bool id_accepted = false;
bool id_sent = false;
long startMillis = 0;
String ID = "DEUR_ID";

/* voor het ontvangen van data */
bool newData = false;
const byte numChars = 32;
char receivedChars[numChars];
unsigned int inputsoud = 0;
unsigned int inputsoud2 = 0;

void setup()
{
  initDeur();
  initWiFi();

  myservo.attach(D5);
  myservo.write(70);

  Wire.begin();

}
void loop(void)
{
  connectToServer();
  leesbericht();
  verwerkData();
// client.println("<knopDeur>");
  leesKnop();
  delay(100);
}

/* -------- */
/* Functies */
/* -------- */

void initDeur()
{
  pinMode(D5, OUTPUT);
  Wire.begin();
  Serial.begin(115200);
  Serial.println();

  // flash mosfet output
  digitalWrite(D5, HIGH);
  delay(1000);
  digitalWrite(D5, LOW);
  delay(1000);
}

// Start de wifi verbinding.
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

// Maakt verbinding met de server.
void connectToServer()
{
  while (!client.connected()) // zorgt ervoor dat als de client verbonden is er niet nog een poging wordt gedaan.
  {
    id_accepted = false;
    id_sent = false;
    Serial.println("No Connection");
    if (client.connect("voer hier ip van pi in", PORT))
      Serial.println("Server found");
    delay(100);
  }
  if (id_sent && millis() - startMillis > 1000 && !id_accepted)
    client.stop();
}

// leest de data die verstuurd wordt door de server. Start met lezen als hij '<' ziet en stopt wanneer hij '>' ziet.
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

// Verwerkt de data die ontvangen is van de server.
void verwerkData()
{
  if (newData == true)
  {
    Serial.println(receivedChars);
    if (!id_accepted) // nodig zodat de client niet verbinding met de server blijft maken
    {
      if (strcmp(receivedChars, "id?") == 0)
      {
        client.println(ID);
        Serial.println("ID Sent");
        startMillis = millis();
        id_sent = true;
      }
      else if (strcmp(receivedChars, "id_accept") == 0)
      {
        Serial.println("ID accepted");
        id_accepted = true;
        myservo.write(70);
      }
    }
    if (strcmp(receivedChars, "open") == 0)
    {
      //  Serial.println("OPEN");
      myservo.write(180);
      delay(10);
    }
    else if (strcmp(receivedChars, "dicht") == 0) // kijken of !open werkt, zo niet verander naar open == false
    {
      // Serial.println("DICHT");
      myservo.write(70);
      delay(10);
    }
    if (strcmp(receivedChars, "hi") == 0)
      Serial.println("something funny");

    newData = false;
  }
}

void leesKnop() // afblijven sloopt wemos anders
{
  Wire.beginTransmission(0x38);
  Wire.write(byte(0x03));          
  Wire.write(byte(0x0F));         
  Wire.endTransmission();

  Wire.beginTransmission(0x36);     //Kies MAX11647
  Wire.write(byte(0xA2));               //Zet byte
  Wire.write(byte(0x03));               //Configureer byte
  Wire.endTransmission();               //Stopt I2C connectie

  unsigned int inputs = 0;
  Wire.beginTransmission(0x38); 
  Wire.write(byte(0x00));      
  Wire.endTransmission();
  Wire.requestFrom(0x38, 1);   
  inputs = Wire.read();
  //inputs&0x0F;
  if (inputs & 0x02)
  {
    // Serial.println(button);
    if (inputsoud != inputs)
    {
      client.println("<knopDeur>");
      inputsoud = 2;
      // client.println("Bed knop");
      inputsoud = inputs;
    }
  }
  else if(inputs & 0x01)
  {
    if(inputsoud2 != inputs)
    {
      client.println("<deurbel>");
      inputsoud2 = inputs;
      Serial.println("deurbel");
    }
  }
  else
  {
    inputsoud = 0;
    inputsoud2 = 0;
  }
}
