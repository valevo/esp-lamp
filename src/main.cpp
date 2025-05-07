#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

#include <algorithm>

// Replace with your network credentials
const char *ssid = "Vale";
const char *password = "13061993";

const int ESP_BUILTIN_LED = 2;
#define NUM_LEDS 144
#define DATA_PIN 5 // this is D5 (on the board/diagram)
#define BRIGHTNESS 120

int brightness = BRIGHTNESS;
int direction = -1;

CRGB leds[NUM_LEDS];

bool gReverseDirection = false;
void Fire2012();

// TODO
// ------
// setup:
//  - won't run until WiFi connected -> try a few times, then continue with the loop -> use lamp to indicate
//  -
//  -
// loop:
//  - fire more orange -> make fire for larger section, cut out middle part and show
//  - modulate brightness -> random up-and-down motion
// -- reverse direction?

void setup()
{
  // Serial.begin(115200);
  // Serial.println("Booting");
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password);

  // digitalWrite(ESP_BUILTIN_LED, HIGH);
  // for (int i = 0; i < 10; i++) {
  //   WiFi.waitForConnectResult();
  //   Serial.println("Connection Failed! Rebooting...");
  //   digitalWrite(ESP_BUILTIN_LED, LOW);
  //   delay(5000);
  //   digitalWrite(ESP_BUILTIN_LED, HIGH);
  //   ESP.restart();
  // }

  // digitalWrite(ESP_BUILTIN_LED, LOW);
  // delay(1000);
  // digitalWrite(ESP_BUILTIN_LED, HIGH);
  // digitalWrite(ESP_BUILTIN_LED, LOW);
  // delay(500);
  // digitalWrite(ESP_BUILTIN_LED, HIGH);
  // digitalWrite(ESP_BUILTIN_LED, LOW);
  // delay(250);
  // digitalWrite(ESP_BUILTIN_LED, HIGH);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]()
                     { Serial.println("Start"); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(ESP_BUILTIN_LED, OUTPUT);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop()
{
  // HIGH means OFF
  digitalWrite(ESP_BUILTIN_LED, HIGH);

  if (WiFi.isConnected())
  {
    ArduinoOTA.handle();
  }

  try
  {

    Fire2012(); // run simulation frame

    FastLED.show(); // display this frame
    FastLED.delay(1000 / 20);

    // if (brightness == 0 || brightness >= 160) { direction = direction*-1;}
    // brightness = brightness + direction;
    // FastLED.setBrightness(brightness);
  }
  catch (...)
  {
    FastLED.setBrightness(50);
    for (int j = 0; j < NUM_LEDS; j++)
    {
      leds[j] = CRGB::Red;
    }
    FastLED.show();
  }
}

// --------------------------------- ORIGINAL FIRE ----------------------------------------------
// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
// #define COOLING 35

// // SPARKING: What chance (out of 255) is there that a new spark will be lit?
// // Higher chance = more roaring fire.  Lower chance = more flickery fire.
// // Default 120, suggested range 50-200.
// #define SPARKING 180

// void Fire2012()
// {
//   // Array of temperature readings at each simulation cell
//   static uint8_t heat[NUM_LEDS];

//   // Step 1.  Cool down every cell a little
//   for (int i = 0; i < NUM_LEDS; i++)
//   {
//     heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
//   }

//   // Step 2.  Heat from each cell drifts 'up' and diffuses a little
//   for (int k = NUM_LEDS - 1; k >= 2; k--)
//   {
//     heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
//   }

//   // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
//   if (random8() < SPARKING)
//   {
//     int y = random8(7);
//     heat[y] = qadd8(heat[y], random8(160, 255));
//     heat[y] = std::min((uint8_t)180, std::max((uint8_t)200, heat[y]));
//   }

//   // Step 4.  Map from heat cells to LED colors
//   for (int j = 0; j < NUM_LEDS; j++)
//   {
//     CRGB color = HeatColor(heat[j]);
//     int pixelnumber;
//     if (gReverseDirection)
//     {
//       pixelnumber = (NUM_LEDS - 1) - j;
//     }
//     else
//     {
//       pixelnumber = j;
//     }
//     leds[pixelnumber] = color;
//   }
// }

// --------------------------------- ORIGINAL FIRE ----------------------------------------------




#define COOLING 35

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 180

void Fire2012()
{
  // Array of temperature readings at each simulation cell
  int N = NUM_LEDS * 3;
  int8_t heat[N];

  // Step 1.  Cool down every cell a little
  for (int i = 0; i < N; i++)
  {
    heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = N - 1; k >= 2; k--)
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if (random8() < SPARKING)
  {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Step 4.  Map from heat cells to LED colors
  for (int j = NUM_LEDS; j < NUM_LEDS * 2; j++)
  {
    // heat[j] = std::min((uint8_t)180, std::max((uint8_t)200, heat[j]));

    CRGB color = HeatColor(heat[j]);
    int pixelnumber;
    if (gReverseDirection)
    {
      pixelnumber = (NUM_LEDS * 2 - 1) - j;
    }
    else
    {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}