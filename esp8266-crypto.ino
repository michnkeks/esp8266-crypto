/*
 * ESP8266 Crypto
 * 
 * A program to display various cryptocurrency and stock 
 * prices on an OLED display using an ESP8266
 * 
 * Conor Walsh 2021-02-27
 * conor.engineer
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

/**** Setup ****/

// Uncomment the next line if you are using an ESP-01
// #define ESP01

// WIFI Network name and password
#define WIFI_SSID "enterwifi"
#define WIFI_PASS "enterwifipasswd"

// Setup time in seconds
#define SETUP_TIME 6

// Update screen every __ seconds
#define UPDATE_TIME 15

// Currency for crypto, tested: eur usd gbp
#define CURRENCY_CODE "eur"
// Currency symbol for crypto, tested: letters and $
#define CURRENCY_SYM ' '

// Fingerprint for api.cryptonator.com - expires 6 Feb 2022
const uint8_t fingerprint_crypto[20] = {0x10, 0x76, 0x19, 0x6B, 0xE9, 0xE5, 0x87, 0x5A, 0x26, 0x12, 0x15, 0xDE, 0x9F, 0x7D, 0x3B, 0x92, 0x9A, 0x7F, 0x30, 0x13};
// Fingerprint for finnhub.io - expires 27 Aug 2021
const uint8_t fingerprint_stock[20] = {0x2B, 0x34, 0xAD, 0x3F, 0x51, 0x8D, 0xA8, 0x5A, 0xD5, 0x53, 0xA3, 0x5D, 0x48, 0x13, 0x85, 0xC4, 0xA1, 0xA2, 0x60, 0x43};

// API Key for finnhub.io, get your free API key at finnhub.io
const String stocksApiKey = "XXXXXXXXXXXXXXXXXXXX";

/** End Setup **/

ESP8266WiFiMulti WiFiMulti;

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Screen Definitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// All asset logos must be 48x48
#define LOGO_WIDTH 48
#define LOGO_HEIGHT 48

// Struct to store an asset
struct asset { 
   bool isCrypto;
   String assetName;
   String url;
   const unsigned char logo [288];
};

// Declare all assests used
// To add a new asset
//  1: Add a new setup similar to one shown in the examples
//  2: Update the name and isCrypto flag for the asset
//  3: Find the api.cryptonator.com or finnhub.io api url for the asset
//  4: Create a 48x48 bitmap logo for the asset using http://javl.github.io/image2cpp/
asset assets[] = {
    // Doge setup
    {true, "Doge",
     "https://api.cryptonator.com/api/ticker/doge-"+String(CURRENCY_CODE),
      {0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 
       0x7f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 
       0x00, 0x00, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x7f, 
       0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x7f, 0x80, 
       0x00, 0x1f, 0xf0, 0x00, 0xff, 0xfc, 0x00, 0x0f, 0xf0, 0x00, 0xff, 0xff, 0x00, 0x0f, 0xe0, 0x00, 
       0xff, 0xff, 0xc0, 0x07, 0xe0, 0x00, 0xff, 0xff, 0xe0, 0x07, 0xc0, 0x00, 0xff, 0xff, 0xe0, 0x03, 
       0xc0, 0x00, 0xff, 0xff, 0xf0, 0x03, 0x80, 0x00, 0xfc, 0x0f, 0xf8, 0x01, 0x80, 0x00, 0xfc, 0x03, 
       0xf8, 0x01, 0x80, 0x00, 0xfc, 0x03, 0xf8, 0x01, 0x80, 0x00, 0xfc, 0x01, 0xf8, 0x01, 0x80, 0x00, 
       0xfc, 0x01, 0xfc, 0x01, 0x00, 0x07, 0xff, 0xe1, 0xfc, 0x00, 0x00, 0x07, 0xff, 0xe0, 0xfc, 0x00, 
       0x00, 0x07, 0xff, 0xe0, 0xfc, 0x00, 0x00, 0x07, 0xff, 0xe0, 0xfc, 0x00, 0x80, 0x00, 0xfc, 0x01, 
       0xfc, 0x01, 0x80, 0x00, 0xfc, 0x01, 0xf8, 0x01, 0x80, 0x00, 0xfc, 0x01, 0xf8, 0x01, 0x80, 0x00, 
       0xfc, 0x03, 0xf8, 0x01, 0x80, 0x00, 0xfc, 0x07, 0xf8, 0x01, 0xc0, 0x00, 0xff, 0xff, 0xf0, 0x03, 
       0xc0, 0x00, 0xff, 0xff, 0xf0, 0x03, 0xe0, 0x00, 0xff, 0xff, 0xe0, 0x07, 0xe0, 0x00, 0xff, 0xff, 
       0xc0, 0x07, 0xf0, 0x00, 0xff, 0xff, 0x80, 0x07, 0xf0, 0x00, 0xff, 0xfe, 0x00, 0x0f, 0xf8, 0x00, 
       0x7f, 0xc0, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 
       0xfe, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xc0, 0x00, 0x00, 
       0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xfe, 
       0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff}
    },
    // BTC setup
    {true, "BTC",
     "https://api.cryptonator.com/api/ticker/btc-"+String(CURRENCY_CODE),
      {0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 
      0xc0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x7f, 
      0xff, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 
      0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xfe, 0x3f, 0xff, 0xe0, 0x0f, 0xff, 0xfe, 0x73, 
      0xff, 0xf0, 0x0f, 0xff, 0xfc, 0x63, 0xff, 0xf0, 0x1f, 0xff, 0x0c, 0x63, 0xff, 0xf8, 0x1f, 0xff, 
      0x00, 0x63, 0xff, 0xfc, 0x3f, 0xff, 0x80, 0x07, 0xff, 0xfc, 0x3f, 0xff, 0xe0, 0x01, 0xff, 0xfc, 
      0x7f, 0xff, 0xe0, 0x80, 0x7f, 0xfe, 0x7f, 0xff, 0xe0, 0xf0, 0x3f, 0xfe, 0x7f, 0xff, 0xe1, 0xf8, 
      0x3f, 0xfe, 0xff, 0xff, 0xc1, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0xc1, 0xf8, 0x1f, 0xff, 0xff, 0xff, 
      0xc1, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xff, 
      0xff, 0xff, 0x82, 0x00, 0xff, 0xff, 0xff, 0xff, 0x83, 0xc0, 0x7f, 0xff, 0xff, 0xff, 0x87, 0xf0, 
      0x3f, 0xff, 0xff, 0xff, 0x07, 0xf8, 0x3f, 0xff, 0xff, 0xfb, 0x07, 0xf8, 0x3f, 0xff, 0x7f, 0xf8, 
      0x07, 0xf0, 0x3f, 0xfe, 0x7f, 0xf0, 0x03, 0xf0, 0x3f, 0xfe, 0x7f, 0xf8, 0x00, 0x00, 0x7f, 0xfe, 
      0x3f, 0xff, 0x80, 0x00, 0x7f, 0xfc, 0x3f, 0xff, 0x88, 0x00, 0xff, 0xfc, 0x3f, 0xff, 0x9c, 0x43, 
      0xff, 0xfc, 0x1f, 0xff, 0x18, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0x18, 0xff, 0xff, 0xf8, 0x0f, 0xff, 
      0xf8, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xfd, 0xff, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 
      0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xff, 0xff, 
      0xfe, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x07, 
      0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00}
    },
    // ETH setup
    {true, "ETH",
     "https://api.cryptonator.com/api/ticker/eth-"+String(CURRENCY_CODE),
      {0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 
      0xc0, 0x00, 0x00, 0x0f, 0xfe, 0xff, 0xf0, 0x00, 0x00, 0x1f, 0xfc, 0x7f, 0xfc, 0x00, 0x00, 0x7f, 
      0xfc, 0x7f, 0xfe, 0x00, 0x00, 0xff, 0xf8, 0x3f, 0xff, 0x00, 0x01, 0xff, 0xf0, 0x1f, 0xff, 0x80, 
      0x03, 0xff, 0xf0, 0x1f, 0xff, 0xc0, 0x07, 0xff, 0xe0, 0x0f, 0xff, 0xe0, 0x07, 0xff, 0xe0, 0x07, 
      0xff, 0xf0, 0x0f, 0xff, 0xc0, 0x07, 0xff, 0xf0, 0x1f, 0xff, 0x80, 0x03, 0xff, 0xf8, 0x1f, 0xff, 
      0x80, 0x03, 0xff, 0xf8, 0x3f, 0xff, 0x00, 0x01, 0xff, 0xfc, 0x3f, 0xff, 0x00, 0x00, 0xff, 0xfc, 
      0x7f, 0xfe, 0x00, 0x00, 0xff, 0xfe, 0x7f, 0xfc, 0x00, 0x00, 0x7f, 0xfe, 0x7f, 0xfc, 0x00, 0x00, 
      0x7f, 0xfe, 0x7f, 0xf8, 0x00, 0x00, 0x3f, 0xfe, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xf0, 
      0x00, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 
      0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfe, 0x00, 0x00, 
      0x7f, 0xff, 0x7f, 0xe7, 0x00, 0x01, 0xcf, 0xff, 0x7f, 0xf3, 0xc0, 0x07, 0x9f, 0xff, 0x7f, 0xf8, 
      0xf0, 0x0e, 0x3f, 0xfe, 0x7f, 0xf8, 0x38, 0x3c, 0x3f, 0xfe, 0x7f, 0xfc, 0x1e, 0xf0, 0x7f, 0xfe, 
      0x3f, 0xfe, 0x07, 0xc0, 0xff, 0xfe, 0x3f, 0xff, 0x03, 0x80, 0xff, 0xfc, 0x1f, 0xff, 0x00, 0x01, 
      0xff, 0xfc, 0x1f, 0xff, 0x80, 0x03, 0xff, 0xf8, 0x0f, 0xff, 0xc0, 0x07, 0xff, 0xf0, 0x0f, 0xff, 
      0xc0, 0x07, 0xff, 0xf0, 0x07, 0xff, 0xe0, 0x0f, 0xff, 0xe0, 0x03, 0xff, 0xf0, 0x1f, 0xff, 0xc0, 
      0x01, 0xff, 0xf0, 0x1f, 0xff, 0xc0, 0x00, 0xff, 0xf8, 0x3f, 0xff, 0x00, 0x00, 0x7f, 0xfc, 0x7f, 
      0xfe, 0x00, 0x00, 0x3f, 0xfe, 0x7f, 0xfc, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x07, 
      0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00}
    },
    // LTC setup .. add picture----
    {true, "LTC",
     "https://api.cryptonator.com/api/ticker/ltc-"+String(CURRENCY_CODE),
     {0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x07, 0xff, 0xff, 
      0xe0, 0x00, 0x00, 0x1f, 0xe0, 0x07, 0xf8, 0x00, 0x00, 0x3f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x7c, 
      0x00, 0x00, 0x3e, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x0f, 0x80, 0x03, 0xe0, 0x00, 0x00, 0x07, 0xc0, 
      0x03, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x07, 0x80, 0x00, 0x00, 0x01, 0xe0, 0x0f, 0x00, 0x0f, 0xc0, 
      0x00, 0xf0, 0x1e, 0x00, 0x0f, 0xc0, 0x00, 0x78, 0x1c, 0x00, 0x1f, 0xc0, 0x00, 0x38, 0x3c, 0x00, 
      0x1f, 0xc0, 0x00, 0x3c, 0x38, 0x00, 0x1f, 0xc0, 0x00, 0x1c, 0x78, 0x00, 0x1f, 0x80, 0x00, 0x1e, 
      0x70, 0x00, 0x1f, 0x82, 0x00, 0x0e, 0x70, 0x00, 0x1f, 0x8e, 0x00, 0x0e, 0x70, 0x00, 0x3f, 0x9e, 
      0x00, 0x0e, 0xe0, 0x00, 0x3f, 0xfe, 0x00, 0x07, 0xe0, 0x00, 0x3f, 0xfc, 0x00, 0x07, 0xe0, 0x00, 
      0x3f, 0xf0, 0x00, 0x07, 0xe0, 0x00, 0x3f, 0xe0, 0x00, 0x07, 0xe0, 0x00, 0x3f, 0x80, 0x00, 0x07, 
      0xe0, 0x00, 0x7f, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x7f, 0x00, 0x00, 0x07, 0xe0, 0x01, 0xff, 0x00, 
      0x00, 0x07, 0xe0, 0x03, 0xfe, 0x00, 0x00, 0x07, 0xe0, 0x07, 0xfe, 0x00, 0x00, 0x07, 0x70, 0x07, 
      0xfe, 0x00, 0x00, 0x0e, 0x70, 0x07, 0xfe, 0x00, 0x00, 0x0e, 0x70, 0x04, 0xfe, 0x00, 0x00, 0x0e, 
      0x78, 0x00, 0xff, 0x00, 0x00, 0x1e, 0x38, 0x00, 0xff, 0xff, 0xe0, 0x1c, 0x3c, 0x00, 0xff, 0xff, 
      0xe0, 0x3c, 0x1c, 0x01, 0xff, 0xff, 0xe0, 0x38, 0x1e, 0x01, 0xff, 0xff, 0xc0, 0x78, 0x0f, 0x00, 
      0xff, 0xff, 0xc0, 0xf0, 0x07, 0x80, 0x00, 0x00, 0x01, 0xe0, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xe0, 
      0x03, 0xe0, 0x00, 0x00, 0x07, 0xc0, 0x01, 0xf0, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x7c, 0x00, 0x00, 
      0x3f, 0x00, 0x00, 0x3f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xe0, 0x07, 0xf8, 0x00, 0x00, 0x07, 
      0xff, 0xff, 0xe0, 0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00}
      
    }
};

// Stonks and not stonks logos are both 70x30
#define STONKS_WIDTH 70
#define STONKS_HEIGHT 30

// Stonks symbol as bitmap
const unsigned char stonks [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 
  0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x01, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x00, 0x00, 
  0x07, 0x80, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0xff, 0xf8, 0x00, 
  0x00, 0x00, 0x3f, 0xc0, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x07, 0xff, 
  0xf0, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x1f, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xf0, 0x00, 
  0x3f, 0xff, 0xe0, 0x00, 0x00, 0x1f, 0xff, 0xf8, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x3f, 0xff, 
  0xf8, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xe7, 0xc0, 0x00, 0x03, 
  0xff, 0xff, 0xfe, 0x1f, 0xff, 0xc7, 0xc0, 0x00, 0x07, 0xff, 0xf3, 0xfe, 0x7f, 0xff, 0x03, 0x80, 
  0x00, 0x1f, 0xff, 0xc1, 0xff, 0xff, 0xfe, 0x03, 0x80, 0x00, 0x7f, 0xff, 0x01, 0xff, 0xff, 0xf8, 
  0x01, 0x80, 0x00, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0xff, 
  0xff, 0xc0, 0x00, 0x00, 0x0f, 0xff, 0xe0, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xc0, 
  0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0xff, 
  0xfc, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 
  0x00, 0x3f, 0xe0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x04, 0x00, 
  0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Not stonks symbol as bitmap
const unsigned char notstonks [] PROGMEM = {
  0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x0c, 
  0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0x00, 
  0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x7f, 
  0xff, 0x80, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xc0, 0x00, 0x7f, 0xfe, 0x00, 0x00, 
  0x00, 0x07, 0xff, 0xf0, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 0xff, 0xff, 
  0xc0, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xf0, 0x01, 0x80, 0x00, 0x3f, 0xff, 0x81, 
  0xff, 0xff, 0xf8, 0x01, 0x80, 0x00, 0x1f, 0xff, 0xe3, 0xff, 0xff, 0xfe, 0x03, 0x80, 0x00, 0x07, 
  0xff, 0xf3, 0xfe, 0x3f, 0xff, 0x83, 0xc0, 0x00, 0x01, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xc7, 0xc0, 
  0x00, 0x00, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x01, 0xff, 
  0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xf8, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xf0, 
  0x00, 0x3f, 0xff, 0xe0, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 0x00, 
  0x7f, 0xe0, 0x00, 0x07, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x01, 0xff, 0xf0, 0x00, 
  0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0xff, 
  0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x03, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00
};

// Variable to store what asset should be displayed
int currentAsset = 0;
// Const to store how many assets are configured
const int maxAssets = sizeof(assets)/sizeof(assets[0]);

void setup() {

  Serial.begin(115200);

  #ifdef ESP01
    // I2C for ESP-01 is different
    Wire.begin(2, 0);
  #endif

  // Try to start display if that doesn't work halt program and inform user
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("HALT: OLED allocation failed"));
    while(1);
  }

  // Immediately clear display to remove Adafruit logo
  display.clearDisplay();
  // Set the text color to white this will turn on pixels when set
  display.setTextColor(SSD1306_WHITE);
  // Set starting text size to 1 for connecting message
  display.setTextSize(1);

  // Allow esp wifi time to setup before attempting connection
  for (uint8_t t = SETUP_TIME; t > 0; t--) {
    // Print info to serial
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    // and print info to OLED
    display.clearDisplay();
    display.setCursor(0, display.height()/4);
    display.print(F("Setup please wait . . . "));
    display.print(t);
    display.display();
    delay(1000);
  }

  // Use font size 2 for the rest of the program
  display.setTextSize(2);

  // Add Wifi network to ESP
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
}

void loop() {
  // Wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    // Set the correct fingerprint
    uint8_t fingerprint[20];
    if(assets[currentAsset].isCrypto)
      // In C a memcopy needs to be used as an array can't be directly assigned to another array
      memcpy(fingerprint, fingerprint_crypto, 20);
    else
      memcpy(fingerprint, fingerprint_stock, 20);

    // Setup a https client
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(fingerprint);
    HTTPClient https;

    // Connect to API
    Serial.print("HTTPS begin...\n");
    if (https.begin(*client, assets[currentAsset].url)) {  // HTTPS

      // start connection and send HTTP header
      Serial.print("HTTPS GET...\n");
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("HTTPS GET... code: %d\n", httpCode);

        // If the HTTP code is valid process and display the data
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          // Store the API payload in a string
          String payload = https.getString();
          // Calculate the price, change and percentage change
          float price;
          float change;
          if(assets[currentAsset].isCrypto){
            price = payload.substring(payload.indexOf("price")+8,payload.indexOf("volume")-3).toFloat();
            change = payload.substring(payload.indexOf("change")+9,payload.indexOf("timestamp")-4).toFloat();
          }
          else{
            price = payload.substring(payload.indexOf("c")+3,payload.indexOf("h")-2).toFloat();
            change = price - payload.substring(payload.indexOf("pc")+4,payload.indexOf("timestamp")-2).toFloat();
          }
          float changePercent = change/price*100;
          // Print payload to serial
          Serial.println(assets[currentAsset].assetName);
          Serial.println(payload);
          // Print info to display
          display.clearDisplay();
          drawLogo(assets[currentAsset].logo);
          drawStonks(change);
          printPrice(price);
          printChange(changePercent);

          // Move to the next coin
          currentAsset++;
          if(currentAsset>=maxAssets){
            currentAsset = 0;
          }
        }
      } else {
        // Warn user using serial and OLED that call failed
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, display.height()/4);
        display.println(F("HTTPS GET failed"));
        display.print(F("Will retry in a few seconds"));
        display.setTextSize(2);
      }

      // Must close the https connection
      https.end();
    }
    else {
      // Warn user using serial and OLED that call failed
      Serial.println("Unable to connect to API");
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, display.height()/4);
      display.println(F("Unable to connect"));
      display.print(F("Will retry in a few seconds"));
      display.setTextSize(2);
    }
  }

  Serial.print("Wait a few seconds before next round...\n\n");
  delay(UPDATE_TIME*1000);
}

// Draw the asset logo on the OLED
void drawLogo(const unsigned char logo []) {
  display.drawBitmap(0,
    (display.height() + (display.height()/4) - LOGO_HEIGHT) / 2,
    logo, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
}

// Draw the stonks or not stonks logo on the OLED
void drawStonks(float change) {
  if(change>=0){
    display.drawBitmap(display.width()-STONKS_WIDTH, display.height()-STONKS_HEIGHT, stonks, STONKS_WIDTH, STONKS_HEIGHT, 1);
  }
  else{
    display.drawBitmap(display.width()-STONKS_WIDTH, display.height()-STONKS_HEIGHT, notstonks, STONKS_WIDTH, STONKS_HEIGHT, 1);
  }  
  display.display();
}

// Print the price on the OLED
void printPrice(float price){
    char priceBuf[20];
    // Get correct currency symbol
    char currencySymbol = '$';
    if(assets[currentAsset].isCrypto)
      currencySymbol = CURRENCY_SYM;
    // Construct the price string
    if(price<10){
      sprintf(priceBuf, "%c%.5f", currencySymbol, price);
    }
    else {
      sprintf(priceBuf, "%c%.3f", currencySymbol, price);
    }
    // To center the price we need to get the size it will be on the OLED
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(priceBuf, 0, 0, &x1, &y1, &w, &h);
    // Set the cursor to the starting location that will center the text
    display.setCursor((display.width() - w) / 2, 0);
    // Print the string to the OLED
    display.print(priceBuf);
    display.display();
}

// Print the price change percentage to the OLED
void printChange(float change) {
  display.setCursor(display.width()-STONKS_WIDTH, display.height()/4);
  // Manually print a + for positive changes
  if(change>=0){
    display.print(F("+"));
  }
  // If the change is less than 10 use a decimal point if not dont
  if(abs(change)<10){
    display.print(change,1);
  }
  else{
    display.print(change,0);
  }
  display.print(F("%"));
  display.display();
}
