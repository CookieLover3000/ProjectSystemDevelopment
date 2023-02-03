#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define I2C_SDL D1
#define I2C_SDA D2

#define PIN 14 //LED pin
#define NUMPIXELS 3 // Aantal pixels in de LED-strip

#define PORT 8080

void initWiFi();
void leesbericht();
void verwerkData();
void connectToServer();

void knop();
void ledUit();
void ledAan();
void donkerRaam();
void lichtRaam();
void potentiometer(unsigned int);
void LDR(unsigned int);
String licht = "aan";
String lichtUit = "uit";
// boolLicht is zodat het licht niet de hele tijd snel aan en uit gaat
bool boolLicht = false;
// automatisch checkt of de potentiemeter helemaal naar links of rechts staat
bool automatisch = true;

WiFiClient client;

/* voor het verbinden met de server */
bool id_sent = false;
bool id_accepted = false;
long startMillis = 0;
String ID = "MUUR_ID";

/* voor het ontvangen van data */
bool newData = false;
const byte numChars = 32;
char receivedChars[numChars];

/* initialisatie van de adafruit lib (voor LED-strip)
Neo_GRB geeft aan dat het en GRB bitstream is (voor de meeste neopixel producten)
NEO_KHZ800 is voor een 800 KHz bitstream is (voor meeste neopixel producten)
*/
Adafruit_NeoPixel led(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned int anin0;
unsigned int anin1;

void setup()
{
  pinMode(D5, OUTPUT);
  Wire.begin();
  Serial.begin(115200);
  led.begin();
  for (int i = 0; i < 3; i++)
    led.setPixelColor(i, 0, 0, 0);
  led.show();
  ledUit();

  Wire.begin();
  Serial.begin(115200); // Set serial baud rate to 115200 bps
  Wire.beginTransmission(0x38);
  Wire.write(0x03);
  Wire.write(0b00001111);
  Wire.endTransmission();
  lichtRaam();
  Wire.begin();

  initWiFi();
}
void loop()
{
  connectToServer();
  leesbericht();
  verwerkData();

  // Config MAX11647
  Wire.beginTransmission(0x36);
  Wire.write(byte(0xA2));
  Wire.write(byte(0x03));
  Wire.endTransmission();

  // Read analog 10bit inputs 0&1
  Wire.requestFrom(0x36, 4);
  anin0 = Wire.read() & 0x03;
  anin0 = anin0 << 8;
  anin0 = anin0 | Wire.read();
  anin1 = Wire.read() & 0x03;
  anin1 = anin1 << 8;
  anin1 = anin1 | Wire.read();
  LDR(anin0);
  client.println(anin1);
  Serial.println(anin0);

  delay(100);

  digitalWrite(D5, HIGH);
  delay(100);
  digitalWrite(D5, LOW);

  Wire.beginTransmission(0x38);
  Wire.write(byte(0x00));
  Wire.endTransmission();
}

void initWiFi()
{
  // Zet hier de naam + wachtwoord voor het WiFi netwerk
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
  while (!client.connected()) // zorgt ervoor dat als de client verbonden is er niet nog een poging wordt gedaan
  {
    id_sent = false;
    /* NODIG VOOR FIX */
    id_accepted = false; // wordt gereset zodat als we verbinding met de server kwijt raken de identification opnieuw wordt gedaan
    Serial.println("No Connection");
    if (client.connect("voer hier ip van pi in", PORT))
      Serial.println("Server found");
    delay(100);
  }
  if (id_sent && millis() - startMillis > 2000 && !id_accepted)
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
// Deze functie is om te vergelijken welke data is ontvangen met wat er nodig is 
// zoals de kleur van de LED-strip (met potentiometer), stand van het raam en of het op de automatische modus staat
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
        startMillis = millis();
      }
      else if (strcmp(receivedChars, "id_accept") == 0)
      {
        Serial.println("ID accepted");
        id_accepted = true;
      }
    }
    if (strcmp(receivedChars, "ga dicht") == 0)
      donkerRaam();
    else if (strcmp(receivedChars, "ga open") == 0)
      lichtRaam();
    else if (strcmp(receivedChars, "aan") == 0)
      ledAan();
    else if (strcmp(receivedChars, "uit") == 0)
    {
      ledUit();
      automatisch = false;
    }
    else if (strcmp(receivedChars, "autoUit") == 0)
    {
      ledUit();
    }
    else if (strcmp(receivedChars, "geel") == 0)
    {
      ledAan();
      potentiometer(1);
      automatisch = false;
    }
    else if (strcmp(receivedChars, "blauw") == 0)
    {
      ledAan();
      potentiometer(2);
      automatisch = false;
    }
    else if (strcmp(receivedChars, "paars") == 0)
    {
      ledAan();
      potentiometer(3);
      automatisch = false;
    }
    else if (strcmp(receivedChars, "rood") == 0)
    {
      ledAan();
      potentiometer(4);
      automatisch = false;
    }
    else if (strcmp(receivedChars, "auto") == 0)
    {
      automatisch = true;
    }
    newData = false;
  }
}

void ledUit() // Om LED-strip uit te zetten
{
  led.begin();
  for (int i = 0; i < 3; i++)
    led.setPixelColor(i, 0, 0, 0);
  led.show();
}
void ledAan() // Om LED-strip aan te zetten
{
  for (int i = 0; i < 3; i++)
    led.setPixelColor(i, 255, 255, 0);
  led.show();
}

void donkerRaam() 
{
  Wire.beginTransmission(0x38);
  Wire.write(0x01);
  Wire.write(0b00010000);
  Wire.endTransmission();
}
void lichtRaam()
{
  Wire.beginTransmission(0x38);
  Wire.write(0x01);
  Wire.write(0b00000000);
  Wire.endTransmission();
}

void potentiometer(unsigned int a1) // Als de server een waarde geeft wordt het in deze functie gestopt en vergelijkt het voor een kleur
{
  for (int i = 0; i < 3; i++)
  {
    if (a1 == 1) // geel
    {
      led.setPixelColor(i, 255, 255, 0);
      led.show();
    }
    if (a1 == 2) // blauw
    {
      led.setPixelColor(i, 0, 255, 255);
      led.show();
    }
    if (a1 == 3) // paars
    {
      led.setPixelColor(i, 255, 0, 255);
      led.show();
    }
    if (a1 == 4) // rood
    {
      led.setPixelColor(i, 255, 0, 0);
      led.show();
    }
  }
}

void LDR(unsigned int a0)
{
  if (automatisch) // Checkt of de muur in de automatische stand staat
  {
    if (a0 < 150 && boolLicht == false) // Stuurt naar de server dat de LED-strip aan moet gaan als de LED-strip uit is en minder meet dan 150 lumen
    {
      boolLicht = true;
      client.println(licht);
    }
    else if (a0 > 500 && boolLicht) // Stuurt naar de server dat de LED-strip uit moet gaan als de LED-strip aan is en meer meet dan 500 lumen
    {
      boolLicht = false;
      client.println("autoUit");
    }
  }
}
