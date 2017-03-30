  
  /*
  blinkytest
  for blinkofant

  wizard23
  TomK32

  1 modul consumes 1.2A if fully lit
  If your LED drivers are the 5V compatible LS3654LS,
  you can remove the 7805 and 7808 regulators, and connect the
  5V and 8V with a 5V source. Saves space

  Useful information:
  http://metalab.at/wiki/Blinkofant
  https://www.binary-kitchen.de/wiki/doku.php?id=projekte:ledmatrix:start
  https://devlol.org/wiki/Blinkofant

*/


#include "SPI.h"
#ifdef ESP8266_WEMOS_D1MINI
  #define CLEAR_PANEL_PIN D8 // pin #10 on the blinkofant panel
  #define DATA_PIN D7 // pin #8
  #define CLOCK_PIN D5 // pin #4
#else
  #define CLEAR_PANEL_PIN 10 // this goes on panel to pin #10 on the blinkofant panel
  #define DATA_PIN 11 // pin #8
  #define CLOCK_PIN 13 // pin #4
#endif

#define PANELS 3
#define PANELDATA_SIZE (10*PANELS)

uint8_t panelData[PANELDATA_SIZE];

void setBlink(int value) {
  panelData[0] = value;
}

// set pixels on a specific panel
void setPixel(int x, int y, int panel, int value) {
  if (panel >= PANELS) {
    // out of bounds
    return;
  }
  setPixel(x + (panel * 8), y, value);
}

void setPixel(int x, int y, int value)
{
  int index = (y + 1) + x * 10; // y+1 because 1st bit controlls blinking
  int byteNr = index >> 3; // division by 8
  int bitNr = index & 0x7; // rest bei div durch 8

  if (value)
  {
    panelData[byteNr] |= 1 << bitNr;
  }
  else
  {
    panelData[byteNr] &= ~(1 << bitNr);
  }
}

void setAllPixel(uint8_t value)
{
  for (int i = 0; i < PANELDATA_SIZE; i++)
  {
    panelData[i] = value;
  }
}


void shiftPixelData() {
  screen_off();

  for (int i = 0; i < PANELDATA_SIZE; i++) {
    uint8_t value = panelData[i];

    SPDR = panelData[i];
    while (!(SPSR & (1 << SPIF)));
  }
  screen_on();
}

void setup()
{
  Serial.begin(9600);
  pinMode(CLEAR_PANEL_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  digitalWrite(CLEAR_PANEL_PIN, LOW);
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);

  SPI.begin();
  SPI.setBitOrder(LSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV128); // biggest divider there is.
}


void screen_off()
{
  digitalWrite(CLEAR_PANEL_PIN, LOW);
}

void screen_on()
{
  digitalWrite(CLEAR_PANEL_PIN, HIGH);
}


int state = 0;

int fps = 1000 / 30;
void snake() {
  static unsigned int pos = 0;
  static unsigned int length = 0;

  int new_pos = (millis() / fps) % (8 * PANELS * 9);
  if (pos == new_pos) {
    return;
  } else if (pos > new_pos) {
    length++;
    Serial.println(length);
    if (length > 8 * PANELS * 9) {
      length = 0;
    }
  }
  pos = new_pos;

  setAllPixel(0);
  int y = (pos / (8 * PANELS));
  for (int i = 0; i <= length; i++) {
    if (y < (pos + i) / (8 * PANELS)) {
      y++;
    }
    setPixel((pos + i) % (8 * PANELS), y % 9, 0, 1);
  }
}

void scanner(int panel) {
  static int col = 0;
  static int row = 0;
  static bool is_row = true;
  if (is_row) {
    for(int c=0; c < 8; c++) {
      setPixel(c, row, 1); 
    }
    row++;
    if (row > 8) { row = 0; is_row = !is_row; }
  } else {
    for(int r=0; r < 9; r++) {
      setPixel(col, r, 1);
    }
    col++;
    if (col > 9) { col = 0; is_row = !is_row; }
  }
  
  //setPixel(col, row, 1);
}

void flashPanels() {
  fps = 1000;
  static unsigned int panel = -1;

  int new_pos = (millis() / fps) % PANELS;
  if (panel == new_pos) {
    return;
  }
  panel = new_pos;

  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 9; y++) {
      setPixel(x, y, panel, 1);
    }
  }
}

void soulmates(x1, y1, x2, y2) {
  static bool state = true;
  state = !state;
  setPixel(state ? x1 : x2, state ? y1 y2, 1);
}

void stars(int num, bool value) {
  for (int i=0; i < num; i++) {
    setPixel(random(0, 7), random(0, 8), random(0, 3), value);
  }
}

void loop ()
{
  
  setAllPixel(0);

  // uncomment any of these demo modes
  //snake();
  //flashPanels();
  //stars(3, 1);
  soulmates();

  shiftPixelData();

  delay(300);

}
