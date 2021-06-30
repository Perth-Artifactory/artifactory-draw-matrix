////////////////////////////////////////////////////////////////////////////////////
/// Artifactory Wifi LED Drawing
///
/// Based on code from Rui (Ray) Wang's WiFi-enabled LEDWorkshop
/// https://rayshobby.net/wordpress/wifi-color-led-matrix/
///

// #define FASTLED_ALLOW_INTERRUPTS 1
// #define INTERRUPT_THRESHOLD 1
// #define FASTLED_INTERRUPT_RETRY_COUNT 0

#include <FastLED.h>

FASTLED_USING_NAMESPACE

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include "pixart.h"

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif


// Pins
#define PIN_BUTTON  D2
#define DATA_PIN    D1

// LED Matrix
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    256
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          20
#define FRAMES_PER_SECOND  120


#define MODE_COUNT   2
#define MODE_DRAW    0
#define MODE_OFFLINE 1

// https://intrinsically-sublime.github.io/FastLED-XY-Map-Generator/
// XY mapping function preserving all pixel data.
// Requires 771 Bytes's of SRAM and 7.71 ms/frame for WS2811 based LEDs.
// You COULD save 0 Bytes's of SRAM and 0.00 ms/frame for WS2811 based LEDs.
// Maximum frame rate for WS2811 based LEDs = 129 FPS using 1 output.
// Wired in horizontal serpentine layout starting at the top left corner.

uint16_t XY (uint8_t x, uint8_t y) {
  // map anything outside of the matrix to the extra hidden pixel
  if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT) { return NUM_LEDS; }

  const uint16_t XYTable[] = {
       0,  31,  32,  63,  64,  95,  96, 127, 128, 159, 160, 191, 192, 223, 224, 255,
       1,  30,  33,  62,  65,  94,  97, 126, 129, 158, 161, 190, 193, 222, 225, 254,
       2,  29,  34,  61,  66,  93,  98, 125, 130, 157, 162, 189, 194, 221, 226, 253,
       3,  28,  35,  60,  67,  92,  99, 124, 131, 156, 163, 188, 195, 220, 227, 252,
       4,  27,  36,  59,  68,  91, 100, 123, 132, 155, 164, 187, 196, 219, 228, 251,
       5,  26,  37,  58,  69,  90, 101, 122, 133, 154, 165, 186, 197, 218, 229, 250,
       6,  25,  38,  57,  70,  89, 102, 121, 134, 153, 166, 185, 198, 217, 230, 249,
       7,  24,  39,  56,  71,  88, 103, 120, 135, 152, 167, 184, 199, 216, 231, 248,
       8,  23,  40,  55,  72,  87, 104, 119, 136, 151, 168, 183, 200, 215, 232, 247,
       9,  22,  41,  54,  73,  86, 105, 118, 137, 150, 169, 182, 201, 214, 233, 246,
      10,  21,  42,  53,  74,  85, 106, 117, 138, 149, 170, 181, 202, 213, 234, 245,
      11,  20,  43,  52,  75,  84, 107, 116, 139, 148, 171, 180, 203, 212, 235, 244,
      12,  19,  44,  51,  76,  83, 108, 115, 140, 147, 172, 179, 204, 211, 236, 243,
      13,  18,  45,  50,  77,  82, 109, 114, 141, 146, 173, 178, 205, 210, 237, 242,
      14,  17,  46,  49,  78,  81, 110, 113, 142, 145, 174, 177, 206, 209, 238, 241,
      15,  16,  47,  48,  79,  80, 111, 112, 143, 144, 175, 176, 207, 208, 239, 240
  };

  return XYTable[(y * MATRIX_WIDTH) + x];
}


int mode_index = 0;

unsigned long button_last = 0;
bool initial_demo = true;
bool enableLEDs = false;

// Wifi Setup
// Based on stuff from https://www.hackster.io/rayburne/esp8266-captive-portal-5798ff
const bool apMode = true;
const char *      ap_name = "ArtifactoryDraw";
const byte        DNS_PORT = 53;          // Capture DNS requests on port 53
IPAddress         apIP(1, 3, 3, 7);    // Private network for server
// DNSServer         dnsServer;              // Create the DNS object
// ESP8266WebServer  server(80);          // HTTP server

// Wifi setup for connecting to an access point
char* ssid = "";
char* password = "";
bool staticIP = false;
IPAddress ip(10, 42, 0, 200); // where xx is the desired IP Address
IPAddress gateway(10, 42, 0, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your



// Forward declare functions
// Need 'ICACHE_RAM_ATTR' to prevent crash with newer esp8266 lib versions
void ICACHE_RAM_ATTR button_handler();
void on_status();
void on_change_color();
void on_homepage();



void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Led Drawing 16x16");

  // Set pin mode
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // If button is pressed when starting. Dont show LEDs.
  // Useful for flashing by USB where you dont want to connect 5V
  enableLEDs = digitalRead(PIN_BUTTON); // PULLUP: Normally High

  if (enableLEDs) {
    Serial.println("LED output is enabled");
  } else {
    Serial.println("Button pressed! LED output is disabled");
  }

  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  FastLED.show();  
  
  Serial.println("Ready!");

  // Set WiFi
  if (apMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ap_name);
  
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    // dnsServer.start(DNS_PORT, "*", apIP);
  } else {
    if (staticIP) {
      Serial.print(F("Setting static ip to: "));
      Serial.println(ip);
      WiFi.config(ip, gateway, subnet);
    }
    
    WiFi.mode(WIFI_STA);
    Serial.printf("Connecting to %s\n", ssid);
    if (String(WiFi.SSID()) != String(ssid)) {
      WiFi.begin(ssid, password);
    }

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.print("Connected! Open http://");
    Serial.print(WiFi.localIP());
    Serial.println(" in your browser");
  }
  

  // Set server callback functions
  // server.onNotFound(on_homepage);
  // server.on("/",   on_homepage);
  // // server.on("/js", on_status);
  // // server.on("/cc", on_change_color);
  // server.begin();

  // Set button handler
  attachInterrupt(PIN_BUTTON, button_handler, FALLING);
}

// The variable below is modified by interrupt service routine
// so declare it as volatile
volatile boolean button_clicked = false;

void loop() {
  // if (apMode) {
  //   dnsServer.processNextRequest();
  // }
  
  // server.handleClient();

  if (initial_demo) {
    //display_artifactory_logo();
    hexcolour_snake(ledarray_mask_img_sm_area);
    initial_demo = false;
  }

  // if (initial_demo || mode_index == MODE_OFFLINE) {
  //   static int delayer = 100;
  //   delayer -= 1;
  //   if (delayer == 0) {
  //     display_artifactory_logo();
  //     delayer = 100;
  //   }
  //   delayMicroseconds(100);
  // }

  // if (button_clicked) {
  //   if (millis() - button_last > 1000) {
  //     mode_index += 1;
  //     mode_index %= MODE_COUNT;
  //     Serial.print("Mode changed to ");
  //     Serial.println(mode_index);
  //     button_last = millis();
  //   }
  //   button_clicked = false;
  // }

//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND);
}









// Take an index for a normal c++ array layout and 
// flip it to match led connection order
// (from Artifactory LED Matrx)
int remap_led_index(int pos) {
  int x = pos % MATRIX_WIDTH;
  int y = pos / MATRIX_WIDTH;
  int index = XY(x, y);
  return index;
}

// Setting up hexcolour
// (assumeing LED matrix wired in snake)
//void hexcolour_snake(const uint32_t *arr) {
//  for (uint16_t t = 0; t < NUM_LEDS; t++) {
//    int new_index = remap_led_index(t);
//    if (new_index < 0 || new_index >= NUM_LEDS) {
//      Serial.printf("hexcolour_snake OUT OF BOUNDS - t=%d, new_index=%d\n", t, new_index);
//    }
//    leds.setPixelColor(t, arr[new_index]);
//  }
//  leds.show();
//}
void hexcolour_snake(const uint32_t *arr) {
  for (uint16_t t = 0; t < NUM_LEDS; t++) {
    int new_index = remap_led_index(t);
    leds[new_index] = arr[t];
  }
  FastLED.show();  
}





/* ----------------
    Artifactory Logo
   ---------------- */

const byte artifactory_logo[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
  0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0,
  0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
  0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


// uint32_t wheel(int WheelPos) {
//   WheelPos = 255 - WheelPos;
//   if (WheelPos < 85) {
//     return leds.Color(255 - WheelPos * 3, 0, WheelPos * 3);
//   }
//   if (WheelPos < 170) {
//     WheelPos -= 85;
//     return leds.Color(0, WheelPos * 3, 255 - WheelPos * 3);
//   }
//   WheelPos -= 170;
//   return leds.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
// }

//// Take an index for a normal c++ array layout and 
//// flip it to match led connection order
//int remap_led_index(int input_index) {
//  // Alternate rows are connected in different direction
//  // (snake scheme)
//  int c = 15 - input_index / 16, r = input_index % 16;
//  int index = 0;
//  if (r % 2 == 1) {
//    index = c + r * 16;
//  } else {
//    index = (15 - c) + r * 16;
//  }
//  return index;
//}

// void display_artifactory_logo() {
//   static byte idx = 0;

//   for (int i = 0; i < NUM_LEDS; i++) {
//     int index = remap_led_index(i);
    
//     // every pixel color is 6 bytes storing the hex value

//     pixels[index] = wheel((i + idx) & 0xFF) * artifactory_logo[i];
//   }

//   idx += 1;
//   idx %= 256;

//   show_leds();
// }

/* ----------------
    WebServer for Drawing
   ---------------- */

#include "html.h"
void on_homepage() {
  String html = FPSTR(index_html);
  if (mode_index == MODE_DRAW) {
    // server.send(200, "text/html", html);
  } else if (mode_index == MODE_OFFLINE) {
    // server.send(200, "text/html", "Artifactory Draw is Offline");
  }
}

// // this returns device variables in JSON, e.g.
// // {"pixels":xxxx,"blink":1}
// void on_status() {
//   String html = "";
//   html += "{\"brightness\":";
//   html += brightness;
//   html += "}";
//   server.send(200, "text/html", html);
// }

// void on_change_color() {

//   uint16_t i;
//   if (server.hasArg("pixels")) {
//     String val = server.arg("pixels");
//     for (i = 0; i < NUM_LEDS; i++) {
//       // every pixel color is 6 bytes storing the hex value
//       // pixels are specified in row-major order
//       // here we need to flip it to column-major order to
//       // match the physical connection of the leds

//       /*int r = i / 16, c = i % 16;
//       //pixels[c*16+r] = strtol(val.substring(i*6, i*6+6).c_str(), NULL, 16);
//       int index = 0;
//       if (r % 2 == 1) {
//         index = c + r * 16;
//       } else {
//         index = (15 - c) + r * 16;
//       }*/

//       int index = remap_led_index(i);

//       pixels[index] = strtol(val.substring(i * 6, i * 6 + 6).c_str(), NULL, 16);
//     }
//   }
//   if (server.hasArg("clear")) {
//     for (i = 0; i < NUM_LEDS; i++) {
//       pixels[i] = 0;
//     }
//   }
//   /*if(server.hasArg("brightness")) {
//     brightness = server.arg("brightness").toInt();
//     }*/

//   initial_demo = false;
//   show_leds();

//   server.send(200, "text/html", "{\"result\":1}");
// }

char dec2hex(byte dec) {
  if (dec < 10) return '0' + dec;
  else return 'A' + (dec - 10);
}

void button_handler() {
  button_clicked = true;
}
