#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Servo.h>

#define PORT 8080

unsigned int totaalai0;
unsigned int totaalai1;
unsigned int totaalai0oud;
unsigned int totaalai1oud;
bool button;
bool buttonoud;
bool inputsoud;
bool ingedrukt;
bool nietIngedrukt;

void initWiFi();
void leesbericht();
void verwerkData();
void connectToServer();
void led();
void knop();
void leesdruksens();

WiFiClient client;

/* voor het verbinden met de server */
bool id_accepted = false;
bool id_sent = false;
long startMillis = 0;
String ID = "BED_ID"; // verander socket voor je apparaat

/* voor het ontvangen van data */
bool newData = false;
const byte numChars = 32;
char receivedChars[numChars];

void setup()
{
  Wire.begin();
  Serial.begin(115200); // Set serial baud rate to 9600 bps
  Serial.println("Starting I2C setup");
  Wire.beginTransmission(0x38); // data direction
  Wire.write(0x03);
  Wire.write(0b00001111);
  Wire.endTransmission();
  Wire.beginTransmission(0x36); // send setup and config byte
  Wire.write(byte(0xA2));
  Wire.write(byte(0x03));
  Wire.endTransmission();
  Serial.println("Finished I2C setup, starting Wifi connection...");

  initWiFi();
}

void loop()
{

  connectToServer();
  leesbericht();
  verwerkData();
  leesdruksens();

  knop();
  if (buttonoud != button)
  { // verandere door het lezen van de server
    client.println("<knopBed>");
    buttonoud = button;
  }
  delay(100);
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
  while (!client.connected()) // zorgt ervoor dat als de client verbonden is er niet nog een poging wordt gedaan
  {
    id_accepted = false; // wordt gereset zodat als we verbinding met de server kwijt raken de identification opnieuw wordt gedaan
    id_sent = false;
    Serial.println("No Connection");
    if (client.connect("voer hier ip van pi in", PORT))
      Serial.println("Server found");
    delay(100);
  }
  if (id_sent && (millis() - startMillis > 1000) && !id_accepted)
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
        id_sent = true;
        startMillis = millis();
      }
      else if (strcmp(receivedChars, "id_accept") == 0)
      {
        Serial.println("ID accepted");
        id_accepted = true;
      }
    }
    if (strcmp(receivedChars, "veranderKnop") == 0)
      led();

    newData = false;
  }
}

// stuurt de led aan.
void led()
{
  Wire.requestFrom(0x38, 1);
  int waardeLed = Wire.read();
  waardeLed ^= 0b00010000;
  Wire.beginTransmission(0x38);
  Wire.write(0x01);
  Wire.write(waardeLed);
  Wire.endTransmission();
  Serial.println("Led");
}

// Leest of de knop is ingedrukt.
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
    // Serial.println(button);
    if (inputsoud != inputs)
    {
      button = !button;
      // client.println("Bed knop");
      inputsoud = inputs;
    }
  }
  else
    inputsoud = 0;
}

// Leest of de druksensor is ingedrukt.
void leesdruksens()
{
  Wire.requestFrom(0x36, 4);
  unsigned int anin0 = Wire.read() & 0x03;
  anin0 = anin0 << 8;
  anin0 = anin0 | Wire.read();

  if (anin0 > 500 && !ingedrukt)
  {
    ingedrukt = true;
    client.println("Persoon ligt op bed");
  }
  else if (anin0 <= 500 && ingedrukt)
  {
    ingedrukt = false;
    client.println("Persoon ligt niet op bed");
  }
}
