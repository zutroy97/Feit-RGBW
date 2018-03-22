

#define FW_NAME "RGBWController"
#define FW_VERSION "0.0.4"

#include <Arduino.h>
/*#include <CmdBuffer.hpp>
#include <CmdCallback.hpp>
#include <CmdParser.hpp>
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
*/
#include <FastLED.h>

#define UPDATES_PER_SECOND 100

#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

byte patternCount = 0;

/*
// LED  ESP-8266
#define REDPIN 13
#define GREENPIN 12
#define BLUEPIN 14
#define DATA_PIN 4
*/
// LED  Nano
#define REDPIN 9 //13 // D9
#define GREENPIN 10 //14 // D10 
#define BLUEPIN 11 //15 // D11
#define DATA_PIN 4
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

//CmdCallback_P<3> cmdCallback;

void show()
{
  //FastLED.show();
  CRGB* rgb = FastLED.leds();
  analogWrite(REDPIN,   rgb->r );
  analogWrite(GREENPIN, rgb->g );
  analogWrite(BLUEPIN,  rgb->b );
}
/*
void functNext(CmdParser *myParser)
{
    patternCount++;
    ChangePaletteOnKeypress(patternCount);
}
*/
void setup() {

  delay(1000);
  Serial.begin(115200);
  
  pinMode(REDPIN, OUTPUT); // RED
  pinMode(GREENPIN, OUTPUT); // GREEN
  pinMode(BLUEPIN, OUTPUT); // BLUE
//  pinMode(D3, OUTPUT); // WHITE
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

  //cmdCallback.addCmd(PSTR("n"), &functNext);
  Serial.println("Setup Complete");
}



int bytes2Read = 0;
void loop() {
    static uint8_t startIndex = 0;

    //cmdCallback.loopCmdProcessing(&myParser, &myBuffer, &Serial);
    
    bytes2Read = Serial.available();
    if (bytes2Read > 0)
    {
        while (bytes2Read-- > 0)
        {
            Serial.read();
        }
        patternCount++;
        ChangePaletteOnKeypress(patternCount);
    }
    //ChangePalettePeriodically();
    
    EVERY_N_MILLISECONDS(100)
    {
        FillLEDsFromPaletteColors(++startIndex); /* motion speed */
        show();
    }
    

    //FastLED.delay(1000 / UPDATES_PER_SECOND);  
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void ChangePaletteOnKeypress(byte pattern)
{
    byte oldPattern = 255;
    if (pattern == oldPattern) return;
    oldPattern = pattern;
    Serial.print("pattern % 10 = ");
    Serial.println(oldPattern % 11);

    switch(oldPattern % 11)
    {
        case 0:
            Serial.println("RainbowColors Blend");
            currentPalette = RainbowColors_p;
            currentBlending = LINEARBLEND;
            break;
        case 1:
            Serial.println("RainbowStripeColors NoBlend");
            currentPalette = RainbowStripeColors_p;
            currentBlending = NOBLEND;
            break;    
        case 2:
            Serial.println("RainbowStripeColors Blend");
            currentPalette = RainbowStripeColors_p;
            currentBlending = LINEARBLEND;
            break;         
        case 3:
            Serial.println("SetupPurpleAndGreenPalette Blend");
            SetupPurpleAndGreenPalette();
            currentBlending = LINEARBLEND;
            break;
        case 4:
            Serial.println("SetupTotallyRandomPalette Blend");
            SetupTotallyRandomPalette();
            currentBlending = LINEARBLEND;
            break;
        case 5:
            Serial.println("SetupBlackAndWhiteStripedPalette NoBlend");
            SetupBlackAndWhiteStripedPalette();
            currentBlending = NOBLEND;
            break;
        case 6:
            Serial.println("SetupBlackAndWhiteStripedPalette Blend");
            SetupBlackAndWhiteStripedPalette();
            currentBlending = LINEARBLEND;
            break;   
        case 7:
            Serial.println("CloudColors Blend");
            currentPalette = CloudColors_p;
            currentBlending = LINEARBLEND;
            break;  
        case 8:
            Serial.println("PartyColors Blend");
            currentPalette = PartyColors_p;
            currentBlending = LINEARBLEND;
            break;     
        case 9:
            Serial.println("myRedWhiteBluePalette NoBlend");
            currentPalette = myRedWhiteBluePalette_p;
            currentBlending = NOBLEND;
            break;  
        case 10:
            Serial.println("myRedWhiteBluePalette Blend");
            currentPalette = myRedWhiteBluePalette_p;
            currentBlending = LINEARBLEND;
            break;                                                           
    }
}
// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};
