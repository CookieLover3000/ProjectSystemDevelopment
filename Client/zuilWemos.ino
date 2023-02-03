#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

#define I2C_SDL D1
#define I2C_SDA D2
#define PORT 8080

void initWiFi();
void leesbericht();
void verwerkData();
void connectToServer();
void brandalarm();
void knop();
void deurbel();
void koelkastAlarm();

WiFiClient client;

/* voor het verbinden met de server */
bool id_sent = false;
bool id_accepted = false;
long startMillis = 0;
String ID = "ZUIL_ID";

/* voor het ontvangen van data */
bool newData = false;
const byte numChars = 32;
char receivedChars[numChars];

unsigned int totaal;
bool pushbutton;
String alarm = "<ALARM>";
int deel = 0;

/* Voor de deurbel */
bool deurbelbool = false;
bool rookAlarm = false;
unsigned long startTime;
const unsigned long timePeriod = 1000;

/* Voor de koelkast */
bool koelkastBool = false;

void setup()
{
  Wire.begin();
  Serial.begin(115200); // Set serial baud rate to 9600 bps
  Wire.beginTransmission(0x38);
  Wire.write(0x03);
  Wire.write(0b00001111);
  Wire.endTransmission();

  Wire.beginTransmission(0x36); // send setup and config byte
  Wire.write(byte(0xA2));
  Wire.write(byte(0x03));
  Wire.endTransmission();

  // zet de buzzer uit voor de wifi init zodat we niet gek worden
  Wire.beginTransmission(0x38);
  Wire.write(0x01);
  Wire.write(0b00000000);
  Wire.endTransmission();
  initWiFi();
}
void loop()
{
  connectToServer();
  leesbericht();
  verwerkData();
  brandalarm();
  // Serial.println(totaal);
  knop();
  koelkastAlarm();
  if (deurbelbool && millis() - startTime > timePeriod)
  {
    Serial.println("start");
    Wire.beginTransmission(0x38);
    Wire.write(0x01);
    Wire.write(0b00000000);
    Wire.endTransmission();
    deurbelbool = false;
  }
}
// leest of de knop is ingedrukt of niet.
void knop()
{
  Wire.beginTransmission(0x38);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(0x38, 1);
  int inputs = Wire.read();
  inputs &= 0x01;
  // Serial.println(inputs);
  if (inputs)
  {
    pushbutton = true;
    // delay(5000);
  }
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
  if (!client.connected()) // zorgt ervoor dat als de client verbonden is er niet nog een poging wordt gedaan
  {
    id_sent = false; // wordt gereset zodat als we verbinding met de server kwijt raken de identification opnieuw wordt gedaan
    id_accepted = false;
    Serial.println("No Connection");
    if (client.connect("VincePi.local", PORT))
      Serial.println("Server found");
    delay(100);
  }
  if (id_sent && millis() - startMillis > 2000 && !id_accepted)
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
    if (!id_accepted)
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
      }
    }
    else if ((strcmp(receivedChars, "deurbel") == 0) && (!rookAlarm))
    {
      deurbel();
    }
    else if (strcmp(receivedChars, "koelkast") == 0)
    {
      koelkastBool = true;
    }
    else if (strcmp(receivedChars, "koelkast dicht") == 0)
    {
      koelkastBool = false;
    }
    else if (strcmp(receivedChars, "hi") == 0)
      Serial.println("something funny");

    newData = false;
  }
}

// bestuurt het brandalarm
void brandalarm()
{
  for (int i = 0; i < 5; i++)
  {
    Wire.requestFrom(0x36, 2);
    unsigned int anin0 = Wire.read() & 0x03;
    anin0 = anin0 << 8;
    pinMode(23, OUTPUT);
    digitalWrite(23, 0);
    anin0 = anin0 | Wire.read();
    // Serial.print("analog in 0: ");
    // Serial.println(anin0);
    delay(100); // test delay
    totaal += anin0;
  }
  if (deel > 500 && !pushbutton)
  {
    Wire.beginTransmission(0x38);
    Wire.write(0x01);
    Wire.write(0b00010000);
    Wire.endTransmission();
    client.println(alarm);
    rookAlarm = true;
  }
  else if (!deurbelbool && !koelkastBool)
  {
    Wire.beginTransmission(0x38);
    Wire.write(0x01);
    Wire.write(0b00000000);
    Wire.endTransmission();
  }
  deel = totaal / 5;
  Serial.println(deel);
  totaal = 0;
  if (deel <= 500)
  {
    rookAlarm = false;
  }
  if (pushbutton)
  {
    client.println("<STOP>");
    rookAlarm = false;
    if (deel <= 500)
      pushbutton = false;
  }
}

// zorgt ervoor dat de deurbel afgaat op een andere pitch dan het brandalarm
void deurbel()
{
  deurbelbool = true;
  startTime = millis();
  unsigned long endtTime = millis() + 1000;

  while (millis() < endtTime)
  {
    static uint8_t data = 0;
    Wire.beginTransmission(0x38);
    Wire.write(0x01);
    Wire.write(data ^= 0b00010000); // doe hier een xor voor dutycycle van 50% (ipv 95%)
    Wire.endTransmission();
    delay(1);
  }
  Wire.beginTransmission(0x38);
  Wire.write(0x01);
  Wire.write(0b00000000); // doe hier een xor voor dutycycle van 50% (ipv 95%)
  Wire.endTransmission();
}

// bestuurt het alarm van de koelkast
void koelkastAlarm()
{
  if (koelkastBool && !rookAlarm)
  {
    static uint8_t data = 0;
    Wire.beginTransmission(0x38);
    Wire.write(0x01);
    Wire.write(data ^= 0b00010000); // doe hier een xor voor dutycycle van 50% (ipv 95%)
    Wire.endTransmission();
    delay(2);
  }
}
