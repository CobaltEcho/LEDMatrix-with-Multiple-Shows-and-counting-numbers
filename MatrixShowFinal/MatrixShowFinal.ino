//Nano Every - megaAVR - ATMega4809

#include <FastLED.h>
#include <OneButton.h>
#include <LEDMatrix.h>
//#include <SamFont.h>
#include <FontMatrise.h>
#include <LEDText.h>




// For Receiving IR Serial Code
char CodeIn;

// Board Pins
#define LED_PIN 9
#define NextShow_PIN   3
#define NumCountUp_PIN  4


//-------Helper macro for getting a macro definition as string---------
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
//-----------------------

uint8_t y = 0;
uint8_t OnOff = 1;

const uint8_t DisplayWidth = 14;
const uint8_t DisplayHeight = 10;

const uint16_t NumLeds = DisplayWidth * DisplayHeight;
const uint16_t NUM_LEDS = NumLeds;
uint8_t Brightness = 150;
uint8_t hue2 = 150;

//CRGB leds[NumLeds];


// For LEd Text Scrolling
cLEDMatrix < DisplayWidth, -DisplayHeight, HORIZONTAL_ZIGZAG_MATRIX > leds;

uint16_t Options;

const uint8_t MaxNum = 99;
int whichNum = 0;

const uint8_t MaxShow = 3;  //Change depending on how many different "shows"
uint8_t whichShow = 0;

OneButton NextShowBtn(NextShow_PIN, true, true);
OneButton NumCountUpBtn(NumCountUp_PIN, true, true);


//-----------Setting up number font stuff-------------
const uint8_t FontWidth = 7;
const uint8_t FontHeight = 10;
const uint8_t NumCharacters = 10;

const uint8_t FontTable[NumCharacters][FontHeight] = {
  { 0x7e, 0x7e, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7e, 0x7e },  // 0
  { 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60 },  // 1
  { 0x7e, 0x7e, 0x60, 0x60, 0x7e, 0x7e, 0x06, 0x06, 0x7e, 0x7e },  // 2
  { 0x7e, 0x7e, 0x60, 0x60, 0x7c, 0x7c, 0x60, 0x60, 0x7e, 0x7e },  // 3
  { 0x66, 0x66, 0x66, 0x66, 0x7e, 0x7e, 0x60, 0x60, 0x60, 0x60 },  // 4
  { 0x7e, 0x7e, 0x06, 0x06, 0x7e, 0x7e, 0x60, 0x60, 0x7e, 0x7e },  // 5
  { 0x06, 0x06, 0x06, 0x06, 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e },  // 6
  { 0x7e, 0x7e, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60 },  // 7
  { 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e },  // 8
  { 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e, 0x60, 0x60, 0x60, 0x60 }   // 9
};

const uint8_t NumDigits = DisplayWidth / FontWidth;



//---------------------(START)Number Code/Counting and stuff--------------------
// Calculate the index for a given LED in the matrix, given its X/Y coordinates
uint16_t xy(uint16_t x, uint16_t y) {
  const int RowBase = DisplayWidth * y;  // number of LEDs before the current row
  if (x >= DisplayWidth) x = DisplayWidth - 1;
  if (y >= DisplayHeight) y = DisplayHeight - 1;

  uint16_t output;
  if (y % 2 == 0) output = RowBase + x;  // normal on even rows
  else output = RowBase + (DisplayWidth - x - 1);  // reversed on odd rows (serpentine)

  if (output >= NumLeds) output = NumLeds - 1;

  return output;
}

void clearDigit(int pos, CRGB color = CRGB::Black);
void writeDigit(int pos, int num, CRGB color, CRGB background = CRGB::Black);
void writeNumber(int num, CRGB color, CRGB background = CRGB::Black);


void clearDigit(int pos, CRGB color) {
  if (pos < 0 || pos >= NumDigits) return;  // display index out of range

  const uint8_t ColumnOffset = FontWidth * pos;  // offset the column position per digit

  for (uint8_t row = 0; row < FontHeight; row++) {
    for (uint8_t col = 0; col < FontWidth; col++) {
      leds(xy(col + ColumnOffset, row)) = color;  // assign color to LED array
    }
  }
}

void writeDigit(int pos, int num, CRGB color, CRGB background) {
  if (num < 0 || num >= NumCharacters) return;  // number out of range
  if (pos < 0 || pos >= NumDigits) return;  // display index out of range

  const uint8_t* Character = FontTable[num];  // get the font array from the table
  const uint8_t ColumnOffset = FontWidth * pos;  // offset the column position per digit

  for (uint8_t row = 0; row < FontHeight; row++) {
    for (uint8_t col = 0; col < FontWidth; col++) {
      const bool lit = Character[row] & (1 << col);  // extract bit for this LED
      leds(xy(col + ColumnOffset, row)) = lit ? color : background;  // assign color to LED array
    }
  }
}

void writeNumber(int num, CRGB color, CRGB background) {
  num = abs(num);  // not supporting negative numbers yet

  for (uint8_t i = 0; i < NumDigits; i++) {
    uint8_t digit = num % 10;
    writeDigit(NumDigits - i - 1, digit, color, background);  // right to left
    num /= 10;
  }
}
//---------------------(END)Number Code and stuff--------------------




//---------------------Show Number on LEDs--------------------
CHSV NumColor = CHSV(150, 255, 255); //Default Blue
void ShowNumber() {
  writeNumber(whichNum, NumColor);
  printLEDs();
  FastLED.show();
}





//---------------------Button Counting--------------------
void IncrementNextShow() {
  whichShow++;
  if (whichShow >= MaxShow) whichShow = 0;
  ShowNumber();
  DebugStatus();
}

void IncrementNumberShow() {
  whichNum++;
  if (whichNum > MaxNum) whichNum = 0;
  DebugStatus();
  ShowNumber();
}
void DecrementNumberShow() {
  whichNum--;
  if (whichNum < 0) whichNum = MaxNum;
  DebugStatus();
  ShowNumber();
}
void ResetNumberShow() {
  whichNum = 0;
  DebugStatus();
  ShowNumber();
}


cLEDText ScrollingMsg;

// Test Text
const unsigned char TxtShow[] = {
  EFFECT_FRAME_RATE "\x02"
  EFFECT_HSV "\x64\xff\xff"
  EFFECT_SCROLL_LEFT "  CAT BAGS"
};

// Real Text Parameters
const unsigned char TxtLockClosed[] = {
  EFFECT_FRAME_RATE "\x01"    //How Fast
  EFFECT_SCROLL_LEFT          //Direction
  EFFECT_HSV_CV "\x00\xff\xff\x40\xff\xff" "  LOCK CLOSED"  //Each text Line a different color
  EFFECT_RGB_CV "\x00\xff\xf2\xff\x00\xff" "  WEIGH-IN 3PM"
  EFFECT_RGB_CV "\xfc\x5b\x5b\xff\x00\x00" "  TOYS FOR TOTS"
  EFFECT_RGB_CH "\x00\xff\xff\xff\xff\x00" "  BUY RAFFLE TICKETS"
};


// Actually show the text
void ScrollingTextShow() {
  int x = 0;
  while (x < 1) {
    FastLED.clear();
    x++;
  }
  if (ScrollingMsg.UpdateText() == -1)
    ScrollingMsg.SetText((unsigned char *)TxtLockClosed, sizeof(TxtLockClosed) - 1);
  else
    FastLED.show();
  FakeDelay(10);
  y = 0;
}




//---------------------SETUP--------------------
void setup() {
  FastLED.addLeds<WS2812B, 9, GRB>(leds[0], leds.Size()).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 10, GRB>(leds[0], leds.Size()).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(105);  //Set Default brightness of LEDs

  // Start serial debug output for remote control
  while (!Serial1);
  Serial.begin(9600);
  Serial1.begin(9600);
  delay(2000);
  Serial.println("LED Show v3.0");

  // Set the hard coded stuff for the text
  ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);

  //---------- Define Buttons----------
  NextShowBtn.attachClick(IncrementNextShow);
  NumCountUpBtn.attachClick(IncrementNumberShow);

  ShowNumber();
}


//---------------------Loop--------------------
void loop() {
  NextShowBtn.tick();
  NumCountUpBtn.tick();

  while (Serial1.available()) {  
    CodeIn = ((char)Serial1.read());
    //Serial.println(CodeIn);  //Debug Line for remote, comment out for actual use
    switch (CodeIn) {  // For Static Shows
      case 'A': LedPower(); break; //Button Power
      case 'B': Serial.println("Button: TV"); break; //Button TV
      case 'C': Serial.println("Button: AV"); break; //Button AV
      case 'D': Serial.println("Button: COMP"); break; //Button COMP
      case 'E': Serial.println("Button: HDMI/PC"); break; //Button HDMI/PC
      case 'F': FlagShow(); break; //Button 1
      //Animated      case 'G': FishShowGo(); break; //Button 2
      //Animated      case 'H': BlinkShow(); break; //Button 3
      //Animated      case 'I': Cylon(); break; //Button 4
      //Animated      case 'J': PacificaShow(); break; //Button 5
      case 'K': CountdownShowGo(); break; //Button 6
      //Animated      case 'L': ScrollingTextShow(); break; //Button 7
      case 'M': CountdownShowGo(); break; //Button 8
      case 'N': Serial.println("Button: 9"); break; //Button 9
      case 'O': Serial.println("Button: -"); break; //Button -
      case 'P': Serial.println("Button: 0"); break; //Button 0
      case 'Q': Serial.println("Button: Pre Ch"); break; //Button Pre Ch
      case 'R': MakeBrighter(); break; //Button Vol +
      case 'S': MakeDimmer(); break; //Button Vol -
      case 'T': ResetHue(); break; //Button Source
      case 'U': ResetBrightness(); break; //Button Mute
      case 'V': IncreaseHue(); break; //Button CH +
      case 'W': DecreaseHue(); break; //Button CH -
      case 'X': ResetNumberShow(); break; //Button Exit
      case 'Y': Serial.println("Button: Display"); break; //Button Display
      case '%': ShowNumber(); break; //Button OK
      case 'Z': IncrementNumberShow(); break; //Button Circle Up
      case '!': DecrementNumberShow(); break; //Button Circle Down
      case '@': Serial.println("Button: Circle Left"); break; //Button Circle Left
      case '$': Serial.println("Button: Circle Right"); break; //Button Circle Right
      case '&': Serial.println("Button: Fav"); break; //Button Fav
      case '*': Serial.println("Button: Menu"); break; //Button Menu
    }
  }

  switch (CodeIn) {   //For Animated Shows
    case 'G': Cylon(); break; //Button 2
    case 'H': FishShowGo(); break; //Button 3
    case 'I': ScrollingTextShow(); break; //Button 4
    case 'J': FullShow(); break; //Button 5
  }

  FastLED.show();
}


//---------------------FakeDelay--------------------
void FakeDelay(int DelayTime) {
  unsigned long time_now = millis();
  while (millis() < time_now + DelayTime) {
  }
}

//------------------------------------Debugging Code--------------------------------------
//-----------SerialOut for Button Pushes
void DebugStatus() {
  Serial.print("ShowNumber: ");
  Serial.print(whichShow);
  Serial.print(" WhichNumber: ");
  Serial.print(whichNum);
  Serial.print('\n');
}

//-----------Debug visualization for the LED on/off states
void printLEDs() {
  for (uint8_t row = 0; row < DisplayHeight; row++) {
    for (uint8_t col = 0; col < DisplayWidth; col++) {
      bool on = leds(xy(col, row)).getLuma() >= 16;  // arbitrary threshold
      Serial.print(on ? 'X' : '_');
      Serial.print(" ");
    }
    Serial.println();
  }
}


int PowerHoldArray[NumLeds];
CRGB color;

void LedPower() {
  if (OnOff = 1) {   // Turn Off
    for (int PowerHoldIndex = 0; PowerHoldIndex < NumLeds; PowerHoldIndex++) {
      color = leds(PowerHoldIndex);
      PowerHoldArray[PowerHoldIndex] = color;
      Serial.println(color);
    }
    FastLED.clear();
    OnOff = 0;

  }
  if (OnOff = 0) {   // Turn On
    for (int i = 0; i < NumLeds; i++) {
      leds(i) = PowerHoldArray[i];
    }
    FastLED.show();
    OnOff = 1;
  }
}

void FullShow() {
  FishShowGo();
  ScrollingTextShow();
}

void MakeBrighter() {
  Brightness = FastLED.getBrightness();
  Brightness = Brightness + 10;
  FastLED.setBrightness(Brightness);
}
void MakeDimmer() {
  Brightness = FastLED.getBrightness();
  Brightness = Brightness - 10;
  if (Brightness <= 0) Brightness = 15;
  FastLED.setBrightness(Brightness);
}

void ResetBrightness() {
  Brightness = 150;
  FastLED.setBrightness(Brightness);
}

void IncreaseHue() {
  hue2 = hue2 + 5;
  if (hue2 > 255) hue2 = 0;
  NumColor = CHSV(hue2, 255, 255);
  Serial.println(hue2);
  ShowNumber();
}

void DecreaseHue() {
  hue2 = hue2 - 5;
  if (hue2 < 0) hue2 = 255;
  NumColor = CHSV(hue2, 255, 255);
  Serial.println(hue2);
  ShowNumber();
}

void ResetHue() {
  hue2 = 150;
  NumColor = CHSV(hue2, 255, 255);
  Serial.println(hue2);
  ShowNumber();
}
int ArrayGet[41] = {28, 29, 30, 31, 33, 34, 35, 37, 38, 39, 40, 41, 44, 50, 55, 56, 61, 67, 72, 76, 77, 78, 80, 81, 83, 84, 87, 89, 95, 100, 106, 108, 111, 112, 113, 114, 115, 117, 118, 119, 123};
int ArraySet[40] = {28, 29, 30, 31, 33, 34, 35, 37, 38, 39, 40, 41, 44, 50, 55, 56, 61, 67, 72, 76, 77, 78, 80, 81, 82, 83, 87, 89, 95, 100, 106, 108, 112, 113, 114, 115, 117, 118, 119, 123};
int ArrayNum10[68] = {3, 4, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 23, 24, 31, 32, 36, 37, 40, 41, 42, 43, 46, 47, 51, 52, 59, 60, 64, 65, 68, 69, 70, 71, 74, 75, 79, 80, 87, 88, 92, 93, 96, 97, 98, 99, 102, 103, 107, 108, 115, 116, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 135, 136};
int ArrayNum9[40] = {4, 5, 6, 7, 8, 9, 18, 19, 20, 21, 22, 23, 32, 33, 36, 37, 46, 47, 50, 51, 60, 61, 62, 63, 64, 65, 74, 75, 76, 77, 78, 79, 92, 93, 102, 103, 120, 121, 130, 131};
int ArrayNum8[52] = {4, 5, 6, 7, 8, 9, 18, 19, 20, 21, 22, 23, 32, 33, 36, 37, 46, 47, 50, 51, 60, 61, 62, 63, 64, 65, 74, 75, 76, 77, 78, 79, 88, 89, 92, 93, 102, 103, 106, 107, 116, 117, 118, 119, 120, 121, 130, 131, 132, 133, 134, 135};
int ArrayNum7[28] = {4, 5, 6, 7, 8, 9, 18, 19, 20, 21, 22, 23, 36, 37, 46, 47, 64, 65, 74, 75, 92, 93, 102, 103, 120, 121, 130, 131};
int ArrayNum6[48] = {4, 5, 6, 7, 8, 9, 18, 19, 20, 21, 22, 23, 32, 33, 50, 51, 60, 61, 62, 63, 64, 65, 74, 75, 76, 77, 78, 79, 88, 89, 92, 93, 102, 103, 106, 107, 116, 117, 118, 119, 120, 121, 130, 131, 132, 133, 134, 135};
int ArrayNum5[44] = {4, 5, 6, 7, 8, 9, 18, 19, 20, 21, 22, 23, 32, 33, 50, 51, 60, 61, 62, 63, 64, 65, 74, 75, 76, 77, 78, 79, 92, 93, 102, 103, 116, 117, 118, 119, 120, 121, 130, 131, 132, 133, 134, 135};
int ArrayNum4[36] = {4, 5, 8, 9, 18, 19, 22, 23, 32, 33, 36, 37, 46, 47, 50, 51, 60, 61, 62, 63, 64, 65, 74, 75, 76, 77, 78, 79, 92, 93, 102, 103, 120, 121, 130, 131};
int ArrayNum3[44] = {4, 5, 6, 7, 8, 9, 18, 19, 20, 21, 22, 23, 36, 37, 46, 47, 60, 61, 62, 63, 64, 65, 74, 75, 76, 77, 78, 79, 92, 93, 102, 103, 116, 117, 118, 119, 120, 121, 130, 131, 132, 133, 134, 135};
int ArrayNum2[44] = {4, 5, 6, 7, 8, 9, 18, 19, 20, 21, 22, 23, 36, 37, 46, 47, 60, 61, 62, 63, 64, 65, 74, 75, 76, 77, 78, 79, 88, 89, 106, 107, 116, 117, 118, 119, 120, 121, 130, 131, 132, 133, 134, 135};
int ArrayNum1[20] = {6, 7, 20, 21, 34, 35, 48, 49, 62, 63, 76, 77, 90, 91, 104, 105, 118, 119, 132, 133};
int ArrayFish[47] = {28, 29, 30, 32, 34, 35, 36, 38, 41, 42, 45, 49, 51, 55, 56, 60, 62, 66, 69, 70, 71, 72, 73, 75, 76, 77, 79, 81, 82, 83, 84, 88, 92, 94, 97, 98, 101, 103, 107, 111, 112, 116, 118, 119, 120, 122, 125};


void GetArray() {
  for (byte i = 0; i < 41; i++) {
    leds(ArrayGet[i]) = CRGB(0, 255, 255); //Green
  } FastLED.show();
}
void SetArray() {
  for (byte i = 0; i < 40; i++) {
    leds(ArraySet[i]) = CRGB(0, 0, 255); //Green
  } FastLED.show();
}
void Num10Array() {
  for (byte i = 0; i < 68; i++) {
    leds(ArrayNum10[i]) = CRGB(255, 0, 0); //Red
  } FastLED.show();
}
void Num9Array() {
  for (byte i = 0; i < 40; i++) {
    leds(ArrayNum9[i]) = CRGB(255, 0, 0); //Red
  } FastLED.show();
}
void Num8Array() {
  for (byte i = 0; i < 52; i++) {
    leds(ArrayNum8[i]) = CRGB(255, 0, 0); //Red
  } FastLED.show();
}
void Num7Array() {
  for (byte i = 0; i < 28; i++) {
    leds(ArrayNum7[i]) = CRGB(255, 0, 0); //Red
  } FastLED.show();
}
void Num6Array() {
  for (byte i = 0; i < 48; i++) {
    leds(ArrayNum6[i]) = CRGB(255, 255, 0); //Yellow
  } FastLED.show();
}
void Num5Array() {
  for (byte i = 0; i < 44; i++) {
    leds(ArrayNum5[i]) = CRGB(255, 255, 0); //Yellow
  } FastLED.show();
}
void Num4Array() {
  for (byte i = 0; i < 36; i++) {
    leds(ArrayNum4[i]) = CRGB(255, 255, 0); //Yellow
  } FastLED.show();
}
void Num3Array() {
  for (byte i = 0; i < 44; i++) {
    leds(ArrayNum3[i]) = CRGB(0, 255, 0); //Green
  } FastLED.show();
}
void Num2Array() {
  for (byte i = 0; i < 44; i++) {
    leds(ArrayNum2[i]) = CRGB(0, 255, 0); //Green
  } FastLED.show();
}
void Num1Array() {
  for (byte i = 0; i < 20; i++) {
    leds(ArrayNum1[i]) = CRGB(0, 255, 0); //Green
  } FastLED.show();
}
void FishArray() {
  for (byte i = 0; i < 47; i++) {
    leds(ArrayFish[i]) = CRGB(0, 255, 0); //Green
  } FastLED.show();
}

void (*CountdownShow[13])(void) = {&GetArray, &SetArray, &Num10Array, &Num9Array, &Num8Array, &Num7Array, &Num6Array, &Num5Array, &Num4Array, &Num3Array, &Num2Array, &Num1Array, &FishArray};

void CountdownShowGo() {
  for (byte x = 0; x < 13; x++) {
    FastLED.clear();
    CountdownShow[x]();
    FakeDelay(1000);
  }
}

void fadeall() {
  for (int i = 0; i < NumLeds; i++) {
    leds(i).nscale8(250);
  }
}

void Cylon() {
  static uint8_t hue = 0;

  //Serial.print("x");
  // First slide the led in one direction
  for (int i = 0; i < NumLeds; i++) {
    // Set the i'th led to red
    leds(i) = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    FakeDelay(10);
  }
  //Serial.print("x");

  // Now go in the other direction.
  for (int i = (NumLeds) - 1; i >= 0; i--) {
    // Set the i'th led to red
    leds(i) = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    FakeDelay(10);
  }
}

int ArrayFish1[2] = {69, 70};
int ArrayFish2[6] = {42, 68, 69, 70, 71, 97};
int ArrayFish3[12] = {41, 42, 43, 67, 68, 69, 70, 71, 72, 96, 97, 98};
int ArrayFish4[19] = {14, 40, 41, 43, 44, 66, 67, 68, 69, 70, 71, 72, 73, 95, 96, 97, 98, 99, 125};
int ArrayFish5[27] = {14, 15, 39, 40, 41, 42, 44, 45, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 94, 95, 96, 97, 98, 99, 100, 124, 125};
int ArrayFish6[35] = {14, 15, 16, 38, 39, 40, 41, 42, 43, 45, 46, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 93, 94, 95, 96, 97, 98, 99, 100, 101, 123, 124, 125};
int ArrayFish7[43] = {14, 15, 16, 17, 37, 38, 39, 40, 41, 42, 43, 44, 46, 47, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 122, 123, 124, 125};
int ArrayFish8[49] = {15, 16, 17, 18, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 47, 48, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 121, 122, 123, 124};
int ArrayFish9[53] = {16, 17, 18, 19, 35, 36, 37, 38, 39, 40, 42, 43, 44, 45, 46, 48, 49, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 90, 91, 92, 93, 94, 95, 96, 97, 99, 100, 101, 102, 103, 104, 120, 121, 122, 123};
int ArrayFish10[55] = {17, 18, 19, 20, 34, 35, 36, 37, 38, 39, 43, 44, 45, 46, 47, 49, 50, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 89, 90, 91, 92, 93, 94, 95, 96, 100, 101, 102, 103, 104, 105, 119, 120, 121, 122};
int ArrayFish11[59] = {18, 19, 20, 21, 33, 34, 35, 36, 37, 38, 42, 44, 45, 46, 47, 48, 50, 51, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 88, 89, 90, 91, 92, 93, 94, 95, 97, 101, 102, 103, 104, 105, 106, 118, 119, 120, 121};
int ArrayFish12[65] = {19, 20, 21, 22, 32, 33, 34, 35, 36, 37, 41, 42, 43, 45, 46, 47, 48, 49, 51, 52, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 87, 88, 89, 90, 91, 92, 93, 94, 96, 97, 98, 102, 103, 104, 105, 106, 107, 117, 118, 119, 120};
int ArrayFish13[69] = {14, 20, 21, 22, 23, 31, 32, 33, 34, 35, 36, 40, 41, 43, 44, 46, 47, 48, 49, 50, 52, 53, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 86, 87, 88, 89, 90, 91, 92, 93, 95, 96, 98, 99, 103, 104, 105, 106, 107, 108, 116, 117, 118, 119, 125};
int ArrayFish14[71] = {14, 15, 21, 22, 23, 24, 30, 31, 32, 33, 34, 35, 39, 40, 44, 45, 47, 48, 49, 50, 51, 53, 54, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 85, 86, 87, 88, 89, 90, 91, 92, 94, 95, 99, 100, 104, 105, 106, 107, 108, 109, 115, 116, 117, 118, 124, 125};
int ArrayFish15[69] = {15, 16, 22, 23, 24, 25, 29, 30, 31, 32, 33, 34, 38, 39, 45, 46, 48, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 93, 94, 100, 101, 105, 106, 107, 108, 109, 110, 114, 115, 116, 117, 123, 124};
int ArrayFish16[65] = {16, 17, 23, 24, 25, 26, 28, 29, 30, 31, 32, 33, 37, 38, 46, 47, 49, 50, 51, 52, 53, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 92, 93, 101, 102, 106, 107, 108, 109, 110, 111, 113, 114, 115, 116, 122, 123};
int ArrayFish17[59] = {17, 18, 24, 25, 26, 27, 28, 29, 30, 31, 32, 36, 37, 47, 48, 50, 51, 52, 53, 54, 56, 57, 58, 59, 60, 61, 62, 63, 64, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 91, 92, 102, 103, 107, 108, 109, 110, 111, 112, 113, 114, 115, 121, 122};
int ArrayFish18[52] = {18, 19, 25, 26, 27, 28, 29, 30, 31, 35, 36, 48, 49, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 90, 91, 103, 104, 108, 109, 110, 111, 112, 113, 114, 120, 121};
int ArrayFish19[44] = {19, 20, 26, 27, 28, 29, 30, 34, 35, 49, 50, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 89, 90, 104, 105, 109, 110, 111, 112, 113, 119, 120};
int ArrayFish20[36] = {20, 21, 27, 28, 29, 33, 34, 50, 51, 53, 54, 55, 56, 57, 58, 59, 60, 61, 78, 79, 80, 81, 82, 83, 84, 85, 86, 88, 89, 105, 106, 110, 111, 112, 118, 119};
int ArrayFish21[28] = {21, 22, 28, 32, 33, 51, 52, 54, 55, 56, 57, 58, 59, 60, 79, 80, 81, 82, 83, 84, 85, 87, 88, 106, 107, 111, 117, 118};
int ArrayFish22[22] = {22, 23, 31, 32, 52, 53, 55, 56, 57, 58, 59, 80, 81, 82, 83, 84, 86, 87, 107, 108, 116, 117};
int ArrayFish23[18] = {23, 24, 30, 31, 53, 54, 56, 57, 58, 81, 82, 83, 85, 86, 108, 109, 115, 116};
int ArrayFish24[16] = {24, 25, 29, 30, 54, 55, 56, 57, 82, 83, 84, 85, 109, 110, 114, 115};
int ArrayFish25[12] = {25, 26, 28, 29, 55, 56, 83, 84, 110, 111, 113, 114};
int ArrayFish26[6] = {26, 27, 28, 111, 112, 113};
int ArrayFish27[2] = {27, 112};
int ArrayFish28[0] = {};

CRGBPalette16 currentPalette;

DEFINE_GRADIENT_PALETTE( RedGreen_gp ) {
  0,    225,  0,  0,   //red
  85,   255,  0,  0,   //red
  170,  0, 255,  0,  //green
  255,  0, 255, 0 //green
};

DEFINE_GRADIENT_PALETTE( MagentaYellow_gp ) {
  0, 255, 0, 255,  //Magenta
  85, 255, 0, 255,  //Magenta
  170, 255, 255, 0,  //Yellow
  255, 255, 255, 0  //Yellow
};

DEFINE_GRADIENT_PALETTE( BlueGreen_gp ) {
  0, 0, 0, 255,  //Blue
  51, 0, 110, 255,
  102, 0, 187, 255,
  153, 0, 255, 255,
  204, 0, 255, 145,
  255, 0, 255, 0  //Green
};


/*
DEFINE_GRADIENT_PALETTE( RandomPallet_gp ) {
  0, random8(), random8(), random8(),
  51, random8(), random8(), random8(),
  102, random8(), random8(), random8(),
  153, random8(), random8(), random8(),
  204, random8(), random8(), random8(),
  255, random8(), random8(), random8()
};
*/

void Fish1Array() {
  for (byte i = 0; i < 2; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish1[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish2Array() {
  for (byte i = 0; i < 6; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish2[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish3Array() {
  for (byte i = 0; i < 12; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish3[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish4Array() {
  for (byte i = 0; i < 19; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish4[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish5Array() {
  for (byte i = 0; i < 27; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish5[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish6Array() {
  for (byte i = 0; i < 35; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish6[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish7Array() {
  for (byte i = 0; i < 43; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish7[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish8Array() {
  for (byte i = 0; i < 49; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish8[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish9Array() {
  for (byte i = 0; i < 53; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish9[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish10Array() {
  for (byte i = 0; i < 55; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish10[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish11Array() {
  for (byte i = 0; i < 59; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish11[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish12Array() {
  for (byte i = 0; i < 65; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish12[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish13Array() {
  for (byte i = 0; i < 69; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish13[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish14Array() {
  for (byte i = 0; i < 71; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish14[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish15Array() {
  for (byte i = 0; i < 69; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish15[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish16Array() {
  for (byte i = 0; i < 65; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish16[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish17Array() {
  for (byte i = 0; i < 59; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish17[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish18Array() {
  for (byte i = 0; i < 52; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish18[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish19Array() {
  for (byte i = 0; i < 44; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish19[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish20Array() {
  for (byte i = 0; i < 36; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish20[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish21Array() {
  for (byte i = 0; i < 28; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish21[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish22Array() {
  for (byte i = 0; i < 22; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish22[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish23Array() {
  for (byte i = 0; i < 18; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish23[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish24Array() {
  for (byte i = 0; i < 16; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish24[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish25Array() {
  for (byte i = 0; i < 12; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish25[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish26Array() {
  for (byte i = 0; i < 6; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish26[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish27Array() {
  for (byte i = 0; i < 2; i++) {
    uint8_t paletteIndex = map(i, 0, NUM_LEDS - 1, 0, 240);
    leds(ArrayFish27[i]) = ColorFromPalette(currentPalette, paletteIndex, Brightness, LINEARBLEND);
  } FastLED.show();
}
void Fish28Array() {
  FastLED.clear();
}

void (*FishShow[28])(void) = {&Fish1Array, &Fish2Array, &Fish3Array, &Fish4Array, &Fish5Array, &Fish6Array, &Fish7Array, &Fish8Array, &Fish9Array, &Fish10Array, &Fish11Array, &Fish12Array, &Fish13Array, &Fish14Array, &Fish15Array, &Fish16Array, &Fish17Array, &Fish18Array, &Fish19Array, &Fish20Array, &Fish21Array, &Fish22Array, &Fish23Array, &Fish24Array, &Fish25Array, &Fish26Array, &Fish27Array, &Fish28Array};


int NumOfPallets = 4;
int PalletNum = 1;

void ChangePallet() {
  if (PalletNum <= NumOfPallets) {
    switch (PalletNum) {
      case 1:
        currentPalette = RedGreen_gp;
        break;
      case 2:
        currentPalette = MagentaYellow_gp;
        break;
      case 3:
        currentPalette = BlueGreen_gp;
        break;
/*      case 4:
        currentPalette = RandomPallet_gp;
        break;   */
    }
  }
  else {
    PalletNum = 1;
    currentPalette = RedGreen_gp;
  }
}


void FishShowGo() {
  for (byte x = 0; x < 28; x++) {
    FastLED.clear();
    FishShow[x]();
    FakeDelay(100);
  }
  ChangePallet();
}




//---------------------FlagShow--------------------
void FlagShow() {
  FastLED.clear();
  leds(0) = CRGB(0, 0, 255);
  leds(1) = CRGB(0, 0, 255);
  leds(2) = CRGB(0, 0, 255);
  leds(3) = CRGB(0, 0, 255);
  leds(4) = CRGB(0, 0, 255);
  leds(5) = CRGB(255, 0, 0);
  leds(6) = CRGB(255, 0, 0);
  leds(7) = CRGB(255, 0, 0);
  leds(8) = CRGB(255, 0, 0);
  leds(9) = CRGB(255, 0, 0);
  leds(10) = CRGB(255, 0, 0);
  leds(11) = CRGB(255, 0, 0);
  leds(12) = CRGB(255, 0, 0);
  leds(13) = CRGB(255, 0, 0);
  leds(14) = CRGB(255, 255, 255);
  leds(15) = CRGB(255, 255, 255);
  leds(16) = CRGB(255, 255, 255);
  leds(17) = CRGB(255, 255, 255);
  leds(18) = CRGB(255, 255, 255);
  leds(19) = CRGB(255, 255, 255);
  leds(20) = CRGB(255, 255, 255);
  leds(21) = CRGB(255, 255, 255);
  leds(22) = CRGB(255, 255, 255);
  leds(23) = CRGB(0, 0, 255);
  leds(24) = CRGB(255, 255, 255);
  leds(25) = CRGB(0, 0, 255);
  leds(26) = CRGB(255, 255, 255);
  leds(27) = CRGB(0, 0, 255);
  leds(28) = CRGB(0, 0, 255);
  leds(29) = CRGB(0, 0, 255);
  leds(30) = CRGB(255, 255, 255);
  leds(31) = CRGB(0, 0, 255);
  leds(32) = CRGB(0, 0, 255);
  leds(33) = CRGB(255, 0, 0);
  leds(34) = CRGB(255, 0, 0);
  leds(35) = CRGB(255, 0, 0);
  leds(36) = CRGB(255, 0, 0);
  leds(37) = CRGB(255, 0, 0);
  leds(38) = CRGB(255, 0, 0);
  leds(39) = CRGB(255, 0, 0);
  leds(40) = CRGB(255, 0, 0);
  leds(41) = CRGB(255, 0, 0);
  leds(42) = CRGB(255, 255, 255);
  leds(43) = CRGB(255, 255, 255);
  leds(44) = CRGB(255, 255, 255);
  leds(45) = CRGB(255, 255, 255);
  leds(46) = CRGB(255, 255, 255);
  leds(47) = CRGB(255, 255, 255);
  leds(48) = CRGB(255, 255, 255);
  leds(49) = CRGB(255, 255, 255);
  leds(50) = CRGB(255, 255, 255);
  leds(51) = CRGB(0, 0, 255);
  leds(52) = CRGB(255, 255, 255);
  leds(53) = CRGB(0, 0, 255);
  leds(54) = CRGB(255, 255, 255);
  leds(55) = CRGB(0, 0, 255);
  leds(56) = CRGB(0, 0, 255);
  leds(57) = CRGB(0, 0, 255);
  leds(58) = CRGB(0, 0, 255);
  leds(59) = CRGB(0, 0, 255);
  leds(60) = CRGB(0, 0, 255);
  leds(61) = CRGB(255, 0, 0);
  leds(62) = CRGB(255, 0, 0);
  leds(63) = CRGB(255, 0, 0);
  leds(64) = CRGB(255, 0, 0);
  leds(65) = CRGB(255, 0, 0);
  leds(66) = CRGB(255, 0, 0);
  leds(67) = CRGB(255, 0, 0);
  leds(68) = CRGB(255, 0, 0);
  leds(69) = CRGB(255, 0, 0);
  leds(70) = CRGB(255, 255, 255);
  leds(71) = CRGB(255, 255, 255);
  leds(72) = CRGB(255, 255, 255);
  leds(73) = CRGB(255, 255, 255);
  leds(74) = CRGB(255, 255, 255);
  leds(75) = CRGB(255, 255, 255);
  leds(76) = CRGB(255, 255, 255);
  leds(77) = CRGB(255, 255, 255);
  leds(78) = CRGB(255, 255, 255);
  leds(79) = CRGB(255, 255, 255);
  leds(80) = CRGB(255, 255, 255);
  leds(81) = CRGB(255, 255, 255);
  leds(82) = CRGB(255, 255, 255);
  leds(83) = CRGB(255, 255, 255);
  leds(84) = CRGB(255, 0, 0);
  leds(85) = CRGB(255, 0, 0);
  leds(86) = CRGB(255, 0, 0);
  leds(87) = CRGB(255, 0, 0);
  leds(88) = CRGB(255, 0, 0);
  leds(89) = CRGB(255, 0, 0);
  leds(90) = CRGB(255, 0, 0);
  leds(91) = CRGB(255, 0, 0);
  leds(92) = CRGB(255, 0, 0);
  leds(93) = CRGB(255, 0, 0);
  leds(94) = CRGB(255, 0, 0);
  leds(95) = CRGB(255, 0, 0);
  leds(96) = CRGB(255, 0, 0);
  leds(97) = CRGB(255, 0, 0);
  leds(98) = CRGB(255, 255, 255);
  leds(99) = CRGB(255, 255, 255);
  leds(100) = CRGB(255, 255, 255);
  leds(101) = CRGB(255, 255, 255);
  leds(102) = CRGB(255, 255, 255);
  leds(103) = CRGB(255, 255, 255);
  leds(104) = CRGB(255, 255, 255);
  leds(105) = CRGB(255, 255, 255);
  leds(106) = CRGB(255, 255, 255);
  leds(107) = CRGB(255, 255, 255);
  leds(108) = CRGB(255, 255, 255);
  leds(109) = CRGB(255, 255, 255);
  leds(110) = CRGB(255, 255, 255);
  leds(111) = CRGB(255, 255, 255);
  leds(112) = CRGB(255, 0, 0);
  leds(113) = CRGB(255, 0, 0);
  leds(114) = CRGB(255, 0, 0);
  leds(115) = CRGB(255, 0, 0);
  leds(116) = CRGB(255, 0, 0);
  leds(117) = CRGB(255, 0, 0);
  leds(118) = CRGB(255, 0, 0);
  leds(119) = CRGB(255, 0, 0);
  leds(120) = CRGB(255, 0, 0);
  leds(121) = CRGB(255, 0, 0);
  leds(122) = CRGB(255, 0, 0);
  leds(123) = CRGB(255, 0, 0);
  leds(124) = CRGB(255, 0, 0);
  leds(125) = CRGB(255, 0, 0);
}
