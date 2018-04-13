#define DEFAULT_HANGTIME 500
#define DEFAULT_HANGSTEP 32
#define DEFAULT_HANGSPEED 25
#define DEFAULT_PATTERN 4
#define DEFAULT_BRIGHTNESS 255

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

// Custom Patterns
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

typedef struct
{
    char name[15];
    //uint8_t  numberOfHolds;
    //uint8_t holds[16];
    
} PatternInfo_t;

PatternInfo_t tempPatternInfo;

typedef struct
{
    uint8_t currentPattern;
    uint8_t whiteValue;
    uint16_t hangTime;
    uint8_t hangSpeed;
    bool isRunning;
    uint8_t brightness;
    uint8_t hangSteps;
} PatternState_t;
PatternState_t machineState = {
    DEFAULT_PATTERN,
    INITIAL_WHITE_VALUE,
    DEFAULT_HANGTIME,
    DEFAULT_HANGSPEED,
    true,
    DEFAULT_BRIGHTNESS,
    DEFAULT_HANGSTEP};

#define NUMBER_PATTERNS 7
const PatternInfo_t Patterns [NUMBER_PATTERNS] PROGMEM = 
{
    {"Rainbow"},
    {"Rainbow Stripe"},
    {"Heat"},
    {"Cloud"},
    {"Party"},
    {"Lava"},
    {"Red White Blue"}
};

#define NUMBER_COLOR_NAMES 4
const char colorName_red[] PROGMEM = "red";
const char colorName_green[] PROGMEM = "green";
const char colorName_blue[] PROGMEM = "blue";
const char colorName_white[] PROGMEM = "white";
const char* const colorNames[NUMBER_COLOR_NAMES] PROGMEM = {colorName_red, colorName_green, colorName_blue, colorName_white};


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
    currentPalette = PartyColors_p;
    currentBlending = LINEARBLEND;

    cmdAdd( "speed", cmd_hangSpeed);
    cmdAdd("pattern", cmd_pattern);
    cmdAdd("help", cmd_help);
    cmdAdd("hangtime", cmd_hangTime);
    cmdAdd("white", cmd_white);
    cmdAdd("bright", cmd_bright);
    cmdAdd("off", cmd_off);
    cmdAdd("on", cmd_on);
    cmdAdd("jump", cmd_jump);
    cmdAdd("hangstep", cmd_hangstep);
    cmdAdd("color", cmd_color);
    Serial.println(F("READY:"));
    analogWrite(WHITEPIN, machineState.whiteValue);
}

void cmd_color(int arg_cnt, char **args)
{
    char buffer[6];
    Stream *s = cmdGetStream();
    if (arg_cnt == 2)
    {
        for (int i=0; i < NUMBER_COLOR_NAMES; i++)
        {
            if (isColorName(i, args[1]))
            {
                machineState.isRunning = false;
                leds[0] = CRGB::Black;
                analogWrite(WHITEPIN, 0);
                switch(i)
                {
                    case 0:
                        leds[0] = CRGB::Red; break;
                    case 1:
                        leds[0] = CRGB::Green; break;
                    case 2:
                        leds[0] = CRGB::Blue; break;
                    case 3:
                        analogWrite(WHITEPIN, 255);
                        machineState.whiteValue = 255;
                        break;
                }
                show();
            }
        }
    }
}

bool isColorName(byte i, const char* testString)
{
    char buffer[10];
    strcpy_P(buffer, (char*) (pgm_read_word(&(colorNames[i]))));
    return strcmp(testString, buffer) == 0;
}

void cmd_hangstep(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            s->println(machineState.hangSteps);
            break;
        case 2:
            machineState.hangSteps = constrain( cmdStr2Num(args[1],10), 1, 255);
            break;
    }
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
    static uint8_t transistionTimer = machineState.hangSpeed; // Counts # of loops
    static uint16_t hangTimeTimer = machineState.hangTime;

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
                    transistionTimer = machineState.hangSpeed;
                    if (gradientIndex % machineState.hangSteps == 0)
                    {
                        hangTimeTimer = machineState.hangTime;
                        transistionTimer = machineState.hangSpeed;
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
            machineState.isRunning = true;
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
            s->println(machineState.hangTime);
            break;
        case 2:
            machineState.hangTime = constrain( cmdStr2Num(args[1], 10), 1, 65535);
            break;
    }
}
void cmd_help(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    s->println(F("speed-MS between next gradient value 1-255"));
    s->println(F("pattern-Which pattern letter to run"));
    s->println(F("hangtime-MS to wait between hangsteps 1-65535"));
    s->println(F("white-Brightness of the white light 0-100"));
    s->println(F("bright-overall brightness of the color LEDs (0-100)"));
    s->println(F("off/on-Change color lights state."));
    s->println(F("hangstep-Number of steps between hangtime"));
    s->println(F("Patterns:"));
    for (byte i = 0; i< NUMBER_PATTERNS; i++)
    {
        s->print(i);s->print(F(" - "));
        s->print( (char)('a' + i));s->print(F(" "));
        printPatternName(i);
        s->println();
    }
}

void printPatternName(int patternNumber)
{
    Stream *s = cmdGetStream();
    memcpy_P (&tempPatternInfo, &Patterns[patternNumber], sizeof tempPatternInfo);
    s->print(tempPatternInfo.name);
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

void cmd_hangSpeed(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            s->println(machineState.hangSpeed);
            break;
        case 2:
            machineState.hangSpeed = constrain( cmdStr2Num(args[1], 10), 1, 255);
            break;
    }
}