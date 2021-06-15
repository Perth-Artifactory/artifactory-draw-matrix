////////////////////////////////////////////////////////////////////////////////////
/// Artifactory Wifi LED Drawing
///
/// Based on code from Rui (Ray) Wang's WiFi-enabled LEDWorkshop
/// https://rayshobby.net/wordpress/wifi-color-led-matrix/
///

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#define PIN_BUTTON  D2
#define PIN_LED     D1
#define NUM_LEDS    256
#define ROWS        16
#define COLS        16

#define MODE_COUNT   2
#define MODE_DRAW    0
#define MODE_OFFLINE 1

// Neopixel object
Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

int mode_index = 0;

byte brightness = 80;       // default brightness: 48
uint32_t pixels[NUM_LEDS];  // pixel buffer. this buffer allows you to set arbitrary
// brightness without destroying the original color values

unsigned long button_last = 0;
bool initial_demo = true;
bool enableLEDs = false;

// Wifi Setup
// Based on stuff from https://www.hackster.io/rayburne/esp8266-captive-portal-5798ff
const bool apMode = true;
const char *      ap_name = "ArtifactoryDraw";
const byte        DNS_PORT = 53;          // Capture DNS requests on port 53
IPAddress         apIP(1, 3, 3, 7);    // Private network for server
DNSServer         dnsServer;              // Create the DNS object
ESP8266WebServer  server(80);          // HTTP server

// Wifi setup for connecting to an access point
char* ssid = "";
char* password = "";
bool staticIP = false;
IPAddress ip(10, 42, 0, 200); // where xx is the desired IP Address
IPAddress gateway(10, 42, 0, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your



// Forward declare functions
void ICACHE_RAM_ATTR button_handler();
//void button_handler();  // causes error with NodeMCU board firmware ver: 2.5.2
void on_status();
void on_change_color();
void on_homepage();
void show_leds();



void show_leds() {
  uint32_t r, g, b;
  uint32_t c;
  for (int i = 0; i < NUM_LEDS; i++) {
    r = (pixels[i] >> 16) & 0xFF;
    g = (pixels[i] >> 8) & 0xFF;
    b = (pixels[i]) & 0xFF;
    r = r * brightness / 255;
    g = g * brightness / 255;
    b = b * brightness / 255;
    c = (r << 16) + (g << 8) + b;
    leds.setPixelColor(i, c);
  }
  if (enableLEDs) {
    leds.show();
  }
  else
  {
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Led Drawing 16x16");

  // Set pin mode
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED,    OUTPUT);

  // If button is pressed when starting. Dont show LEDs.
  // Useful for flashing by USB where you dont want to connect 5V
  enableLEDs = digitalRead(PIN_BUTTON); // PULLUP: Normally High

  if (enableLEDs) {
    Serial.println("LED output is enabled");
  } else {
    Serial.println("Button pressed! LED output is disabled");
  }

  // Delay for recovery
  delay(3000);
  
  Serial.println("Ready!");

  // Initialize LEDs
  leds.begin();
  leds.show();

  // Set WiFi
  if (apMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ap_name);
  
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.start(DNS_PORT, "*", apIP);
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
  server.onNotFound(on_homepage);
  server.on("/",   on_homepage);
  server.on("/js", on_status);
  server.on("/cc", on_change_color);
  server.begin();

  // Set button handler
  attachInterrupt(PIN_BUTTON, button_handler, FALLING);

}

// The variable below is modified by interrupt service routine
// so declare it as volatile
volatile boolean button_clicked = false;

void loop() {
  if (apMode) {
    dnsServer.processNextRequest();
  }
  
  server.handleClient();

  if (initial_demo) {
    display_artifactory_logo();
    initial_demo = false;
  }

  if (initial_demo || mode_index == MODE_OFFLINE) {
    static int delayer = 100;
    delayer -= 1;
    if (delayer == 0) {
      display_artifactory_logo();
      delayer = 100;
    }
    delayMicroseconds(100);
  }

  if (button_clicked) {
    if (millis() - button_last > 1000) {
      mode_index += 1;
      mode_index %= MODE_COUNT;
      Serial.print("Mode changed to ");
      Serial.println(mode_index);
      button_last = millis();
    }
    button_clicked = false;
  }
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

uint32_t wheel(int WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return leds.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return leds.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return leds.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Take an index for a normal c++ array layout and 
// flip it to match led connection order
int remap_led_index(int input_index) {
  // Alternate rows are connected in different direction
  // (snake scheme)
  int c = 15 - input_index / 16, r = input_index % 16;
  int index = 0;
  if (r % 2 == 1) {
    index = c + r * 16;
  } else {
    index = (15 - c) + r * 16;
  }
  return index;
}

void display_artifactory_logo() {
  static byte idx = 0;

  for (int i = 0; i < NUM_LEDS; i++) {
    int index = remap_led_index(i);
    
    // every pixel color is 6 bytes storing the hex value

    pixels[index] = wheel((i + idx) & 0xFF) * artifactory_logo[i];
  }

  idx += 1;
  idx %= 256;

  show_leds();
}

/* ----------------
    WebServer for Drawing
   ---------------- */

#include "html.h"
void on_homepage() {
  String html = FPSTR(index_html);
  if (mode_index == MODE_DRAW) {
    server.send(200, "text/html", html);
  } else if (mode_index == MODE_OFFLINE) {
    server.send(200, "text/html", "Artifactory Draw is Offline");
  }
}

// this returns device variables in JSON, e.g.
// {"pixels":xxxx,"blink":1}
void on_status() {
  String html = "";
  html += "{\"brightness\":";
  html += brightness;
  html += "}";
  server.send(200, "text/html", html);
}

void on_change_color() {

  uint16_t i;
  if (server.hasArg("pixels")) {
    String val = server.arg("pixels");
    for (i = 0; i < NUM_LEDS; i++) {
      // every pixel color is 6 bytes storing the hex value
      // pixels are specified in row-major order
      // here we need to flip it to column-major order to
      // match the physical connection of the leds

      /*int r = i / 16, c = i % 16;
      //pixels[c*16+r] = strtol(val.substring(i*6, i*6+6).c_str(), NULL, 16);
      int index = 0;
      if (r % 2 == 1) {
        index = c + r * 16;
      } else {
        index = (15 - c) + r * 16;
      }*/

      int index = remap_led_index(i);

      pixels[index] = strtol(val.substring(i * 6, i * 6 + 6).c_str(), NULL, 16);
    }
  }
  if (server.hasArg("clear")) {
    for (i = 0; i < NUM_LEDS; i++) {
      pixels[i] = 0;
    }
  }
  /*if(server.hasArg("brightness")) {
    brightness = server.arg("brightness").toInt();
    }*/

  initial_demo = false;
  show_leds();

  server.send(200, "text/html", "{\"result\":1}");
}

char dec2hex(byte dec) {
  if (dec < 10) return '0' + dec;
  else return 'A' + (dec - 10);
}

void button_handler() {
  button_clicked = true;
}
