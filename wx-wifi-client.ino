//  Created by Alex Lelievre on 7/22/20.
//

#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "wifi_password.h"


#define pascal2millibar  0.01
#define millibar2inchHg  0.02953    // should be like the others but isn't
#define c2f( a )            (((a) * 1.8000) + 32)
#define ms2mph( a )         ((a) * 2.23694)
#define inHg2millibars( a ) ((a) * 33.8639)
#define kLocalTempErrorC    2.033333333333333


static float s_localOffsetInHg = 0.50f;

static const char*    ssid     = STASSID;
static const char*    password = STAPSK;
static const char*    host     = "10.0.1.87"; // this is xserve.local (which has a fixed IP address)
static const uint16_t port     = 5555;


Adafruit_BME280 bme; // I2C


void Blink(byte PIN, int DELAY_MS, byte loops)
{
    for (byte i=0; i<loops; i++)
    {
        digitalWrite(PIN,LOW);
        delay(DELAY_MS);
        digitalWrite(PIN,HIGH);
        delay(DELAY_MS);
    }
}


void setup() {
  Serial.begin(115200);
  while(!Serial);    // time to get serial running

  pinMode(LED_BUILTIN, OUTPUT);

  // start up sensor
  unsigned status = bme.begin( BME280_ADDRESS_ALTERNATE );  
  if (!status) 
    Serial.println( "Failed to startup BME280" );
    
  // connect to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print( "Connecting to " );
  Serial.println( ssid );

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode( WIFI_STA );
  WiFi.begin( ssid, password );

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  Serial.print( "pressure offset: " );
  Serial.print( s_localOffsetInHg );
  Serial.println( " InHg" );
}

void loop() {

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  // This will send a string to the server
//  Serial.println("sending data to server");
  if (client.connected()) 
  {
    char outBuffer[128];
    sprintf( outBuffer, "%0.2f, %0.2f, %0.2f", c2f( bme.readTemperature() ), bme.readHumidity(), ((bme.readPressure() * pascal2millibar) * millibar2inchHg) + s_localOffsetInHg );
    client.write( outBuffer, strlen( outBuffer ) );
    Blink( LED_BUILTIN, 100, 1 );
    printValues();
  }

  // close the connection
  client.stop();

  delay(5000);  // every five seconds take a reading and send it to our service
}


void printValues() 
{
    Serial.print( "Temp: " );
    Serial.print( c2f( bme.readTemperature() ) );
    Serial.print("*F");

    Serial.print(", InHg: ");
    Serial.print( ((bme.readPressure() * pascal2millibar) * millibar2inchHg) + s_localOffsetInHg);

    Serial.print( ", Humi: " );
    Serial.print( bme.readHumidity() );
    Serial.println( "%" );
}

// EOF
