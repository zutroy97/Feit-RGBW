#define DEFAULT_HANGTIME 500
#define DEFAULT_HANGSTEP 32
#define DEFAULT_HANGSPEED 25
#define DEFAULT_PATTERN 4
#define DEFAULT_BRIGHTNESS 255

#define FW_NAME "RGBWController"
#define FW_VERSION "0.0.4"

#include <Arduino.h>
#include <Cmd.h>
#include "FIET72031.h"
#define REMOTE_RF_PIN       3 // Input from RF module output
extern volatile struct RFBuilder_t rfBuilder;
extern volatile RemoteCommand_t receivedCommand;

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

extern CRGBPalette16 myFlashPrimaryColorPalette;
extern const TProgmemHSVPalette16 myFlashPrimaryColorPalette_p PROGMEM;

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

#define NUMBER_PATTERNS 9
const PatternInfo_t Patterns [NUMBER_PATTERNS] PROGMEM = 
{
    {"Rainbow"}, //0
    {"Rainbow Stripe"}, //1
    {"Heat"},   // 2
    {"Cloud"},  // 3
    {"Party"},  // 4
    {"Lava"},   // 5
    {"Red White Blue"}, // 6
    {"Primary Color"}, // 7
    {"Ocean"}   //8
};

#define NUMBER_COLOR_NAMES 5
const char colorName_red[] PROGMEM = "red";
const char colorName_green[] PROGMEM = "green";
const char colorName_blue[] PROGMEM = "blue";
const char colorName_white[] PROGMEM = "white";
const char colorName_black[] PROGMEM = "black";
const char* const colorNames[NUMBER_COLOR_NAMES] PROGMEM = {colorName_red, colorName_green, colorName_blue, colorName_white, colorName_black};


CRGBPalette16 palettes[NUMBER_PATTERNS] = {
    RainbowColors_p,  //0
    RainbowStripesColors_p, // 1
    HeatColors_p,   // 2
    CloudColors_p,  // 3
    PartyColors_p,  // 4
    LavaColors_p,   // 5
    myRedWhiteBluePalette_p, // 6
    myFlashPrimaryColorPalette_p,   // 7
    OceanColors_p // 8
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
  CRGB* rgb = FastLED.leds();
  analogWrite(REDPIN,   rgb->r );
  analogWrite(GREENPIN, rgb->g );
  analogWrite(BLUEPIN,  rgb->b );
}

void setup() {

    
    pinMode(REDPIN, OUTPUT); // RED
    pinMode(GREENPIN, OUTPUT); // GREEN
    pinMode(BLUEPIN, OUTPUT); // BLUE
    pinMode(WHITEPIN, OUTPUT); // WHITE
    digitalWrite(REDPIN, LOW);
    digitalWrite(GREENPIN, LOW);
    digitalWrite(BLUEPIN, LOW);
    digitalWrite(WHITEPIN, LOW);
    delay(1000);
    Serial.begin(115200);
    cmdInit(&Serial);
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

    attachInterrupt(digitalPinToInterrupt(REMOTE_RF_PIN), handleRfInterrupt, CHANGE);
    analogWrite(WHITEPIN, machineState.whiteValue);
    Serial.println(F("READY:"));

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
                switch(i)
                {
                    case 0:
                        setSolidColor(CRGB::Red); break;
                    case 1:
                        setSolidColor(CRGB::Green); break;
                    case 2:
                        setSolidColor(CRGB::Blue); break;
                    case 3:
                        setSolidColor(CRGB::White); break;
                    case 4:
                        setSolidColor(CRGB::Black); break;
                }
                show();
            }
        }
    }
}

void cycleSolidColors()
{
    static byte colorIndex = 0;
    switch(colorIndex)
    {
        case 0:
            setSolidColor(CRGB::Red); break;
        case 1:
            setSolidColor(CRGB::Green); break;
        case 2:
            setSolidColor(CRGB::Blue); break;     
        case 3:
            setSolidColor(CRGB::White); break;   
        case 4:
            setSolidColor(CRGB::Purple); break;
        case 5:
            setSolidColor(CRGB::GhostWhite);break;
        case 6:
            setSolidColor(CRGB::Honeydew); break;
    }
    colorIndex++;
    if (colorIndex > 6) colorIndex = 0;
}
void setSolidColor(struct CRGB color)
{
    machineState.isRunning = false;
    leds[0] = CRGB::Black;
    analogWrite(WHITEPIN, 0);
    if (color == CRGB(CRGB::White))
    {
        analogWrite(WHITEPIN, 255);
        machineState.whiteValue = 255;
    }else{
        leds[0] = color;
    }
    show();
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
    setIsRunning(true);
}
void cmd_off(int arg_cnt, char **args)
{
    setIsRunning(false);
}

void setIsRunning(bool value)
{
    if (value)
    {
        machineState.isRunning = true;
        return;
    }else{
        setSolidColor(CRGB::Black);
    }
}
void cmd_jump(int arg_cnt, char **args)
{
    if (arg_cnt == 2){
        FillLEDsFromPaletteColors(constrain(cmdStr2Num(args[1], 10), 0, 255));
        show();
    }
}
void doRemoteCommand(){
    Stream *s = cmdGetStream();
    if (receivedCommand.isReady == 1)
    {
        //Serial.print("REMOTE ID 0x"); Serial.println(receivedCommand.packet.id.remote, HEX);
        //s->print(" Count: "); s->println(receivedCommand.count);        
        //if (receivedCommand.packet.id.remote != 0x2686) return;
        //if (receivedCommand.count != 0) return;
        if (receivedCommand.count == 0)
        {
            // These commands only when count = 0...no holding down the button.
            switch(receivedCommand.packet.id.command)
            {
                case BUTTON_POWER_ON:
                    setIsRunning(true); break;
                case BUTTON_POWER_OFF:
                    setIsRunning(false); break;
                case BUTTON_WHITE:
                    if (machineState.isRunning)
                    {
                        machineState.isRunning = false;
                        setSolidColor(CRGB::White);
                    }else{
                        setWhiteValue(0);
                        machineState.isRunning = true;
                    }
                    break;
                case BUTTON_DOUBLE_ARROW:
                    setWhiteValue(0);goToNextPattern(); break;
                case BUTTON_RED:
                    //changePattern(4);   // Goto Party Pattern
                    //machineState.isRunning = true;
                    //break;
                    cycleSolidColors();
                    break;
                case BUTTON_GREEN:
                    changePattern(4);   // Instant Party pinMode
                    machineState.isRunning = true;
                    break;
            }
        }
        else if ((receivedCommand.count % 5) == 0)
        {
            // These command every 5th count
            switch(receivedCommand.packet.id.command)
            {
                case BUTTON_PLUS:
                    //increaseHangTime();
                    break;
                case BUTTON_MINUS:
                    //decreaseHangTime();
                    break;
                case BUTTON_UP:
                    //decrease hangSpeed
                    break;
                case BUTTON_DOWN:
                    // increase hangSpeed
                    break;
                default:
                    Serial.print("RECEIVED COMMAND: 0x"); Serial.println(receivedCommand.packet.id.command, HEX);
                    Serial.print(" Count: "); Serial.println(receivedCommand.count);   
                    break;
            }
        }
        receivedCommand.isReady = 0;
    }
}
// If not running, resume current pattern. Otherwise switch to next pattern.
void goToNextPattern()
{
    if (!machineState.isRunning)
    {
        machineState.isRunning = true;
        return;
    }
    machineState.currentPattern++;
    if (machineState.currentPattern >= NUMBER_PATTERNS)
        machineState.currentPattern = 0;
    changePattern(machineState.currentPattern);
}

void changePattern(byte patternIndex)
{
    Stream *s = cmdGetStream();
    if (patternIndex >= NUMBER_PATTERNS) return;
    machineState.currentPattern = patternIndex;
    currentPalette = palettes[machineState.currentPattern];
    printPatternName(machineState.currentPattern);
    s->println();
}
void loop() {
    static uint8_t gradientIndex = 0; // Where in the gradient are we?
    static uint8_t transistionTimer = machineState.hangSpeed; // Counts # of loops
    static uint16_t hangTimeTimer = machineState.hangTime;
    RfLoop();
    doRemoteCommand();
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
void cmd_pattern(int arg_cnt, char **args)
{
    Stream *s = cmdGetStream();
    switch(arg_cnt)
    {
        case 1:
            printPatternName(machineState.currentPattern);
            s->println();
            break;
        case 2:
            ChangePalette(args[1][0]);
            break;
    }
}
void ChangePalette(char c)
{
    for (byte i = 0; i< NUMBER_PATTERNS; i++)
    {
        if (c == ('a' + i))
        {
            changePattern(i);
            //printPatternName(i);
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
            s->println(map(machineState.whiteValue, 0, 255, 0, 100));
            break;
        case 2:
            machineState.whiteValue = constrain(cmdStr2Num(args[1], 10), 0, 100);
            setWhiteValue(map(machineState.whiteValue, 0, 100, 0, 255));
            break;
    }
}

// void toggle_white()
// {
//     if (machineState.whiteValue != 0){
//         setWhiteValue(0);return;
//     }
//     setWhiteValue(255);
// }
void setWhiteValue(uint8_t rawValue)
{
    machineState.whiteValue = rawValue;
    analogWrite(WHITEPIN, machineState.whiteValue);
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
        //s->print(i);s->print(F(" - "));
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

const TProgmemPalette16 myFlashPrimaryColorPalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Red, // 'white' is too bright compared to red and blue
    CRGB::Red,
    CRGB::Black,
    
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Green,
    CRGB::Green,
    CRGB::Green,
    CRGB::Black,

    CRGB::Purple,
    CRGB::Purple,
    CRGB::Purple,
    CRGB::Black
};