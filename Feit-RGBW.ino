

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

//byte transistionDelay = 10; // MS to wait before setting next gradient
//uint16_t hangDelay = 0;  // MS to wait at each "hang stop"
byte hangStepsLimit = 16; // Number of gradients to skip before waiting at next hangTime 
//char buffer[30];

// Custom Patterns
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

typedef struct
{
    char name[15];
    uint8_t  numberOfHolds;
    uint8_t holds[16];
    
} PatternInfo_t;

PatternInfo_t tempPatternInfo;

typedef struct
{
    uint8_t currentPattern;
    uint8_t whiteValue;
    uint16_t hangDelay;
    uint8_t transistionDelay;
    bool isRunning;
    uint8_t brightness;
} PatternState_t;
PatternState_t machineState = {0, INITIAL_WHITE_VALUE, 0, 10, true, 255};

#define NUMBER_PATTERNS 7
const PatternInfo_t Patterns [NUMBER_PATTERNS] PROGMEM = 
{
    {"Rainbow", 0, {0}},
    {"Rainbow Stripe", 0, {0}},
    {"Heat", 0, {0}},
    {"Cloud", 0, {0}},
    {"Party", 0, {0}},
    {"Lava", 0, {0}},
    {"Red White Blue", 0, {0}}
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

    cmdAdd( "speed", cmd_speed);
    cmdAdd("pattern", cmd_pattern);
    cmdAdd("help", cmd_help);
    cmdAdd("hangtime", cmd_hangTime);
    cmdAdd("white", cmd_white);
    cmdAdd("bright", cmd_bright);
    cmdAdd("off", cmd_off);
    cmdAdd("on", cmd_on);
    cmdAdd("jump", cmd_jump);
    Serial.println(F("READY:"));
    analogWrite(WHITEPIN, machineState.whiteValue);
}

void cmd_unknown(int arg_cnt, char **args)
{
    Serial.print(F("Unknown Command: "));
    Serial.println(args[0]);
}
void cmd_on(int arg_cnt, char **args)
{
    machineState.isRunning = true;
}
void cmd_off(int arg_cnt, char **args)
{
    machineState.isRunning = false;
    leds[0] = CRGB::Black;
    show();
}

void cmd_jump(int arg_cnt, char **args)
{
    if (arg_cnt == 2){
        FillLEDsFromPaletteColors(constrain(cmdStr2Num(args[1], 10), 0, 255));
        show();
    }
}

void loop() {
    static uint8_t gradientIndex = 0; // Where in the gradient are we?
    static uint8_t transistionTimer = machineState.transistionDelay; // Counts # of loops
    static uint16_t hangTimeTimer = machineState.hangDelay;

    cmdPoll();
    EVERY_N_MILLISECONDS(1)
    {
        if (machineState.isRunning){
            //Serial.print("HTT:"); Serial.println(hangTimeTimer);
            if (hangTimeTimer == 0)
            {
                //Serial.print("TT:"); Serial.println(transistionTimer);
                if (transistionTimer == 0)
                {
                    //Serial.print("GI:"); Serial.println(gradientIndex);
                    FillLEDsFromPaletteColors(gradientIndex++); 
                    show();
                    transistionTimer = machineState.transistionDelay;
                    if (gradientIndex % hangStepsLimit == 0)
                    {
                        hangTimeTimer = machineState.hangDelay;
                        transistionTimer = machineState.transistionDelay;
                    }
                }else{
                    transistionTimer--;
                }
            }else{
                hangTimeTimer--;
            }
        }
    }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    leds[0] = ColorFromPalette(currentPalette, colorIndex, machineState.brightness, currentBlending);
}

void ChangePalette(char c)
{
    for (byte i = 0; i< NUMBER_PATTERNS; i++)
    {
        if (c == ('a' + i))
        {
            currentPalette = palettes[i];
            printPatternName(i);
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
void cmd_bright(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            s->println(machineState.brightness);
            break;
        case 2:
            machineState.brightness = constrain(cmdStr2Num(args[1], 10), 0, 100);
            machineState.brightness = map(machineState.brightness, 0, 100, 0, 255);
            break;
    }
}
void cmd_white(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            s->println(machineState.whiteValue);
            break;
        case 2:
            machineState.whiteValue = constrain(cmdStr2Num(args[1], 10), 0, 100);
            machineState.whiteValue = map(machineState.whiteValue, 0, 100, 0, 255);
            analogWrite(WHITEPIN, machineState.whiteValue);
            break;
    }
}
void cmd_hangTime(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            s->println(machineState.hangDelay);
            break;
        case 2:
            machineState.hangDelay = cmdStr2Num(args[1], 10);
            break;
    }
}
void cmd_help(int arg_cnt, char **args)
{
    for (byte i = 0; i< NUMBER_PATTERNS; i++)
    {
        //memcpy_P (&tempPatternInfo, &Patterns[i], sizeof tempPatternInfo);
        Serial.print(i);Serial.print(F(" - "));
        Serial.print( (char)('a' + i));Serial.print(F(" "));
        printPatternName(i);
        Serial.println();
    }
}

void printPatternName(int patternNumber)
{
    memcpy_P (&tempPatternInfo, &Patterns[patternNumber], sizeof tempPatternInfo);
    Serial.print(tempPatternInfo.name);
}
void cmd_pattern(int arg_cnt, char **args)
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

void cmd_speed(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            s->println(machineState.transistionDelay);
            break;
        case 2:
            machineState.transistionDelay = cmdStr2Num(args[1], 10);
            break;
    }
}