

#define FW_NAME "RGBWController"
#define FW_VERSION "0.0.4"

#include <Arduino.h>
#include <Cmd.h>

/*
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
*/
#include <FastLED.h>
#define INITIAL_WHITE_VALUE 0
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

byte transistionDelay = 10; // MS to wait before setting next gradient
uint16_t hangDelay = 0;  // MS to wait at each "hang stop"
byte hangStepsLimit = 16; // Number of gradients to skip before waiting at next hangTime 
char buffer[30];

// Custom Patterns
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

typedef struct
{
    //char* name;
    char shortcut;
} Pattern;

// Names of Patterns
const char STR_RAINBOW[] PROGMEM = "Rainbow";
const char STR_RAINBOW_STRIPE[] PROGMEM = "Rainbow Stripe";
const char STR_HEAT[] PROGMEM = "Heat";
const char STR_CLOUD[] PROGMEM = "Cloud";
const char STR_PARTY[] PROGMEM = "Party";
const char STR_LAVA[] PROGMEM = "Lava";
const char STR_RED_WHITE_BLUE[] PROGMEM = "Red White Blue";

#define NUMBER_PATTERNS 7
Pattern patterns[NUMBER_PATTERNS] = {
    { 'a'},
    {'b'},
    {'c'},
    {'d'},
    {'f'},
    {'g'},
    {'h'}
};

CRGBPalette16 palettes[NUMBER_PATTERNS] = {
    RainbowColors_p, 
    RainbowStripesColors_p,
    HeatColors_p,
    CloudColors_p,
    PartyColors_p,
    LavaColors_p,
    myRedWhiteBluePalette_p
};

const char* const patternNames[NUMBER_PATTERNS] PROGMEM = {
    STR_RAINBOW,
    STR_RAINBOW_STRIPE,
    STR_HEAT,
    STR_CLOUD,
    STR_PARTY,
    STR_LAVA,
    STR_RED_WHITE_BLUE
};

/*
// LED  ESP-8266
#define REDPIN 13
#define GREENPIN 12
#define BLUEPIN 14
#define DATA_PIN 4
*/
// LED  Arduino Nano 
#define REDPIN 9 //13 // D9
#define GREENPIN 10 //14 // D10 
#define BLUEPIN 11 //15 // D11
#define WHITEPIN 6 // D6
#define DATA_PIN 4

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

void show()
{
  //FastLED.show();
  CRGB* rgb = FastLED.leds();
  analogWrite(REDPIN,   rgb->r );
  analogWrite(GREENPIN, rgb->g );
  analogWrite(BLUEPIN,  rgb->b );
}

void setup() {

    delay(100);
    Serial.begin(115200);
    cmdInit(&Serial);
    
    pinMode(REDPIN, OUTPUT); // RED
    pinMode(GREENPIN, OUTPUT); // GREEN
    pinMode(BLUEPIN, OUTPUT); // BLUE
    pinMode(WHITEPIN, OUTPUT); // WHITE
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

    cmdAdd( "speed", speed);
    cmdAdd("pattern", pattern);
    cmdAdd("help", help);
    cmdAdd("hangtime", hangTime);
    cmdAdd("white", white);
    Serial.println(F("READY:"));
    ChangePalette('a');
    analogWrite(WHITEPIN, INITIAL_WHITE_VALUE);
}

void white(int arg_cnt, char **args)
{
    static byte whiteValue = INITIAL_WHITE_VALUE;
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            s->println(whiteValue);
            break;
        case 2:
            whiteValue = constrain(cmdStr2Num(args[1], 10), 0, 100);
            whiteValue = map(whiteValue, 0, 100, 0, 255);
            analogWrite(WHITEPIN, whiteValue);
            break;
    }
}
void hangTime(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            s->println(hangDelay);
            break;
        case 2:
            hangDelay = cmdStr2Num(args[1], 10);
            break;
    }
}
void help(int arg_cnt, char **args)
{
    for (byte i = 0; i< NUMBER_PATTERNS; i++)
    {
        Serial.print(i);Serial.print(F(" - "));
        Serial.print(patterns[i].shortcut);Serial.print(F(" "));
        strcpy_P(buffer, (char*) pgm_read_word(&( patternNames[i] )));
        Serial.println(buffer);
    }
}
void pattern(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        //case 1:
        //s->println(patterns)
        case 2:
            ChangePalette(args[1][0]);
            break;
    }
}

void speed(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            s->println(transistionDelay);
            break;
        case 2:
            transistionDelay = cmdStr2Num(args[1], 10);
            break;
    }
}

//int bytes2Read = 0;
void loop() {
    static uint8_t gradientIndex = 0; // Where in the gradient are we?
    static uint8_t transistionTimer = transistionDelay; // Counts # of loops
    static uint16_t hangTimeTimer = hangDelay;
    //static uint8_t hangStepIndex = 0;

    cmdPoll();
    EVERY_N_MILLISECONDS(1)
    {
        //Serial.print("HTT:"); Serial.println(hangTimeTimer);
        if (hangTimeTimer == 0)
        {
            //Serial.print("TT:"); Serial.println(transistionTimer);
            if (transistionTimer == 0)
            {
                //Serial.print("GI:"); Serial.println(gradientIndex);
                FillLEDsFromPaletteColors(gradientIndex++); 
                show();
                transistionTimer = transistionDelay;
                if (gradientIndex % hangStepsLimit == 0)
                {
                    hangTimeTimer = hangDelay;
                    transistionTimer = transistionDelay;
                }
            }else{
                transistionTimer--;
            }
        }else{
            hangTimeTimer--;
        }
    }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    leds[0] = ColorFromPalette(currentPalette, colorIndex, 255, currentBlending);
}

void ChangePalette(char c)
{
    for (byte i = 0; i< NUMBER_PATTERNS; i++)
    {
        if (patterns[i].shortcut == c)
        {
            currentPalette = palettes[i];
            return;
        }
    }
    switch(c)
    {
        case '-':
            //speedLimit--;
            //Serial.print("SpeedLimit = ");Serial.println(speedLimit);
            break; 
        case '+':
            //speedLimit++;
            //Serial.print("SpeedLimit = ");Serial.println(speedLimit);
            break;                          
        case ' ':
            //Serial.println("Next Color");
            //speedLimit += 16;
            //Serial.print("SpeedLimit = ");Serial.println(speedLimit);
            break;
        default:
            Serial.println("Unknown Command");
            break;
    }
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