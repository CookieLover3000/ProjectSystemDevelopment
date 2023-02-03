#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#define PORT 8080

#define I2C_SDL D1
#define I2C_SDA D2

#define PIN 14
#define NUMPIXELS 1

bool beweeg = false;
bool gestuurd = false;
bool test = false;
bool aan = false;
bool S1 = false;
bool S2 = false;
bool S3 = false;

bool beweging();
void initWiFi();
void leesbericht();
void verwerkData();
void connectToServer();

WiFiClient client;

/* voor het verbinden met de server */
bool id_accepted = false;
bool id_sent = false;
long startMillis = 0;
String ID = "LAMP_ID"; // verander socket voor je apparaat

/* voor het ontvangen van data */
bool newData = false;
const byte numChars = 32;
char receivedChars[numChars];

Adafruit_NeoPixel led(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup(void)
{
  pinMode(D5, OUTPUT);
  Wire.begin();
  Serial.begin(115200);
  led.setPixelColor(0, 0, 0, 0);
  led.show();

  initWiFi();
}

void loop(void)
{
  digitalWrite(D5, HIGH);
  delay(100);
  digitalWrite(D5, LOW);
  delay(100);

  connectToServer();
  leesbericht();
  verwerkData();

  // flash mosfet output

  Wire.beginTransmission(0x38);
  Wire.write(byte(0x00));
  Wire.endTransmission();

  // if (!test){
  //   led.setPixelColor(0, 0, 0, 0);
  //   led.show();
  //   test = true;
  // }
  aan = beweging();
  if (aan)
  {
    client.println("beweeg");
    aan = false;
  }
}

// Controleert of er de bewegingssensor een beweging meet
bool beweging()
{
  Wire.beginTransmission(0x38);
  Wire.write(byte(0x00));
  Wire.endTransmission();

  // request data van de slave, en schrijf de data uit de buffer naar een int
  Wire.requestFrom(0x38, 1);
  int data = Wire.read() & 0x01;

  if ((data == 1) && !beweeg)
  {
    beweeg = true;
    // client.println("beweging");
    // gestuurd = true;
    Serial.println(data);
    return true;
  }
  else if (data == 0 && beweeg)
  {
    client.println("S4");
    beweeg = false;
    return false;
  }
  return 0;
}

// Start de wifi verbinding.
void initWiFi()
{
  WiFi.begin("Lab003", "lab003WiFi");
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
  while (!client.connected()) // zorgt ervoor dat als de client verbonden is er niet nog een poging wordt gedaan
  {
    id_sent = false; // wordt gereset zodat als we verbinding met de server kwijt raken de identification opnieuw wordt gedaan
    id_accepted = false;
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
    if (strcmp(receivedChars, "uit") == 0)
    {
      led.setPixelColor(0, 0, 0, 0);
      led.show();
    }
    else if (strcmp(receivedChars, "aan") == 0)
    {
      led.setPixelColor(0, 127, 0, 0);
      led.show();
    }
    else if (strcmp(receivedChars, "hoog") == 0)
    {
      led.setPixelColor(0, 255, 0, 0);
      led.show();
    }

    newData = false;
  }
}
