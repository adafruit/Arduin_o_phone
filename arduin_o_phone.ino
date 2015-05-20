/* todo 

 * everything
 * status notifications
 * dim screen when no touches in 1 minute
 * make calls
 * receive calls
 * receive texts?
 
*/


#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_STMPE610.h>
#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);


// For the Adafruit TFT shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

/******************* UI details */
#define BUTTON_X 40
#define BUTTON_Y 100
#define BUTTON_W 60
#define BUTTON_H 30
#define BUTTON_SPACING_X 20
#define BUTTON_SPACING_Y 20
#define BUTTON_TEXTSIZE 2

#define TEXT_X 10
#define TEXT_Y 10
#define TEXT_W 220
#define TEXT_H 50
#define TEXT_TSIZE 3
#define TEXT_TCOLOR ILI9341_MAGENTA

#define TEXT_LEN 12
char textfield[TEXT_LEN+1] = "";
uint8_t textfield_i=0;

#define STATUS_X 10
#define STATUS_Y 65

char buttonlabels[15][5] = {"Send", "Clr", "End", "1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#" };
uint16_t buttoncolors[15] = {ILI9341_DARKGREEN, ILI9341_DARKGREY, ILI9341_RED, 
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE, 
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE, 
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE, 
                             ILI9341_ORANGE, ILI9341_BLUE, ILI9341_ORANGE};
Adafruit_GFX_Button buttons[15];


void status(const __FlashStringHelper *msg) {
  tft.fillRect(STATUS_X, STATUS_Y, 240, 8, ILI9341_BLACK);
  tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print(msg);
}
void status(char *msg) {
  tft.fillRect(STATUS_X, STATUS_Y, 240, 8, ILI9341_BLACK);
  tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.print(msg);
}

void setup() {
  Serial.begin(9600);
  Serial.println("ILI9341 Test!"); 
 
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  
  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");

  // create buttons
  for (uint8_t row=0; row<5; row++) {
    for (uint8_t col=0; col<3; col++) {
      buttons[col + row*3].initButton(&tft, BUTTON_X+col*(BUTTON_W+BUTTON_SPACING_X), 
                 BUTTON_Y+row*(BUTTON_H+BUTTON_SPACING_Y),    // x, y, w, h, outline, fill, text
                  BUTTON_W, BUTTON_H, ILI9341_WHITE, buttoncolors[col+row*3], ILI9341_WHITE,
                  buttonlabels[col + row*3], BUTTON_TEXTSIZE); 
      buttons[col + row*3].drawButton();
    }
  }
  
  // create 'text field'
  tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, ILI9341_WHITE);
  
  status(F("Checking for FONA..."));
  // Check FONA is there
  fonaSS.begin(4800); // if you're using software serial

  // See if the FONA is responding
  if (! fona.begin(fonaSS)) {           // can also try fona.begin(Serial1) 
    status(F("Couldn't find FONA :("));
    while (1);
  }
  status(F("FONA is OK!"));
  
  // Check we connect to the network
  while (fona.getNetworkStatus() != 1) {
    status(F("Looking for service..."));
    delay(10);
  }
  status(F("Connected to network!"));
 
  // set to external mic & headphone
  fona.setAudio(FONA_EXTAUDIO);
   
}

boolean touchedflag;
TS_Point last_p;
void loop(void) {
  
  
  
  
  TS_Point p;
  
  if (ts.bufferSize()) {
    p = ts.getPoint(); 
  } else {
    p.x = p.y = p.z = -1;
  }
  
  // Scale from ~0->4000 to tft.width using the calibration #'s
  if (p.z != -1) {
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    Serial.print("("); Serial.print(p.x); Serial.print(", "); Serial.print(p.y); Serial.print(", "); Serial.print(p.z); Serial.println(") ");
  }
  
  for (uint8_t b=0; b<15; b++) {
    if (buttons[b].contains(p.x, p.y)) {
      //Serial.print("Pressing: "); Serial.println(b);
      buttons[b].press(true);  // tell the button it is pressed
    } else {
      buttons[b].press(false);
    }
  }
  for (uint8_t b=0; b<15; b++) {
    if (buttons[b].justReleased()) {
      // Serial.print("Released: "); Serial.println(b);
      buttons[b].drawButton();  // draw normal
    }
    
    if (buttons[b].justPressed()) {
        buttons[b].drawButton(true);  // draw invert!
        
        // if a numberpad button
        if (b >= 3) {
          if (textfield_i < TEXT_LEN) {
            textfield[textfield_i] = buttonlabels[b][0];
            textfield_i++;
          }
        }
        if (b == 1) {
          // clr! delete char
          textfield[textfield_i] = 0;
          if (textfield > 0) {
            textfield_i--;
            textfield[textfield_i] = ' ';
          }
        }

        Serial.println(textfield);
        tft.setCursor(TEXT_X + 2, TEXT_Y+10);
        tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
        tft.setTextSize(TEXT_TSIZE);
        tft.print(textfield);

        if (b == 2) {
          status(F("Hanging up"));
          fona.hangUp();
        }
        
        if (b == 0) {
          status(F("Calling"));
          Serial.print("Calling "); Serial.print(textfield);
          
          fona.callPhone(textfield);
        }
        
      delay(100); // debouncing
    }
  }
}
