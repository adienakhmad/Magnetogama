#include <SdFat.h>
#include <LCD5110_Graph.h>
#include <MagnetogamaUI.h>
#include <Keypad.h>
#include <ADS1256.h>
#include <SPI.h>
#include <MagnetogamaData.h>
#include <TinyGPS++.h>

#define GPS_WAIT_CHECK 2000
#define FILENAME "LOGGER01.CSV"

#define DEBUG // comment this to disable debug

#ifdef DEBUG
#define DEBUG_PRINT(x)     Serial.print (x)
#define DEBUG_PRINT2(x,y)     Serial.print (x,y)
#define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#define DEBUG_PRINTLN2(x,y)  Serial.println (x,y)
#define DEBUG_START(x) Serial.begin(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINT2(x,y)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTLN2(x,y)
#define DEBUG_START(x)
#endif

// Struct to store error value of hardware such as SD Card, GPS and ADC
struct HW_ERROR
{
  bool adc;
  bool gps;
  bool sdcard;
} hw_error;

// SdFat software SPI template
SdFatSoftSpi<12, 11, 13> sd;
//SdFatSoftSpi<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> sd;
unsigned int nrecord;
unsigned int _measureDuration = 3000;


// Nokia 5110 PIN setup
MagnetogamaUI ui(46, 44, 42, 38, 40, 36); // SCK, MOSI, DC, RST, CS, Backlight PIN

// Keypad Pin Setup
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] =
{
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {33, 47, 45, 41};
byte colPins[COLS] = {39, 31, 43};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// End of keypad pin setup
// ADS1256 PIN Setup
ADS1256 adc(7.68, 49, 53, 48, 2.5); //clockspeed, DRDY pin, CS PIN, Reset PIN, VREF

// GPS Setup
TinyGPSPlus gps;

// Class to hold all reading
MagnetogamaData data;

void setup()
{
  ui.begin();
  ui.setLogoMsg("SETTING UP..");
  Serial.begin(19200);
  Serial2.begin(9600);

  if (!sd.begin(10, SPI_HALF_SPEED))
  {
    sd.initErrorHalt(); // SD CS PIN
    hw_error.sdcard = true;
  }

  else
  {
    SdFile infile;
    infile.open(FILENAME, O_READ);
    unsigned long size = infile.fileSize();
    nrecord = size / 79;
    ui.setNumOfRecord(nrecord);
    DEBUG_PRINT("File size: ");
    DEBUG_PRINTLN(size);
    DEBUG_PRINT("Number of lines: ");
    DEBUG_PRINTLN(nrecord);
  }

  keypad.setHoldTime(3000);
  keypad.addEventListener(keypadEvent);

  DEBUG_PRINTLN("Starting ADC");
// start the ADS1256 with data rate of 15 SPS
  adc.setConversionFactor(10000); // conversion from V to nT
  adc.start(ADS1256_DRATE_15SPS);
  DEBUG_PRINTLN("ADC Started");

// Set MUX Register to AIN2 and AIN3
  adc.switchChannel(2, 3);

  // Check if ADC multiplexer is set
  if (adc.readRegister(MUX) != (ADS1256_MUXP_AIN2 | ADS1256_MUXN_AIN3)) hw_error.adc = true;

// Check if GPS connected
  unsigned long start = millis();

  ui.setLogoMsg("GPS CHECK..");

  while (millis() - start < GPS_WAIT_CHECK)
  {
    gpsfeed();
  }

  if (gps.charsProcessed() < 10) hw_error.gps = true;

  ui.setLogoMsg("PRESS ANY KEY");
}

void loop()
{
  keypad.getKey();
  gpsfeed();
}

// Control all the keypad event

void keypadEvent(KeypadEvent key)
{
  
  if (keypad.getState() == PRESSED)
  {
    checkBattLvl();
    if (key == '7') ui.toggleBackLight();
  }

  switch (ui.getCurrentPage())
  {
  // In case the UI is on the boot page user need to press any button to go to main menu.
  case MagnetogamaUI::BOOT_PAGE:
    if (keypad.getState() != PRESSED) return;
    if (key == '7') return;
    ui.gotoPage(MagnetogamaUI::MENU_PAGE);
    break;

  // In case the UI is on menu page and a button is pressed.
  case MagnetogamaUI::MENU_PAGE:
    if ((keypad.getState() == HOLD) && (key == '9'))
    {
      if (_measureDuration == 3000)
      {
        _measureDuration = 5000;
        ui.printMessage("ACQ DURATION", "5000 ms", 1000);
        ui.gotoPage(MagnetogamaUI::MENU_PAGE);
      }
      else if (_measureDuration == 5000)
      {
        _measureDuration = 10000;
        ui.printMessage("ACQ DURATION", "10000 ms", 1000);
        ui.gotoPage(MagnetogamaUI::MENU_PAGE);
      }
      else
      {
        _measureDuration = 3000;
        ui.printMessage("ACQ DURATION", "3000 ms", 1000);
        ui.gotoPage(MagnetogamaUI::MENU_PAGE);
      }

      return;
    }

    if (keypad.getState() != PRESSED) return;
    switch (key)
    {
    case '1':
      ui.selectMenu(MagnetogamaUI::READING_MENU);
      break;
    case '2':
      ui.selectMenu(MagnetogamaUI::GPS_SEARCH_MENU);
      break;
    case '3':
      ui.selectMenu(MagnetogamaUI::RECALL_MENU);
      break;
    case '4':
      ui.selectMenu(MagnetogamaUI::DUMP_MENU);
      break;
    case '5':
      ui.selectMenu(MagnetogamaUI::CLEAR_MENU);
      break;
    case '6':
      monitor();
      break;
    case '*':
      // Execute selected menu
      switch (ui.getSelectedMenu())
      {
      case MagnetogamaUI::READING_MENU:
        ui.gotoPage(MagnetogamaUI::RECT_PAGE);
        break;

      case MagnetogamaUI::GPS_SEARCH_MENU:
        DEBUG_PRINTLN("Inside GPS Search Page");
        ui.gotoPage(MagnetogamaUI::GPS_SEARCH_PAGE);
        if (gpsWaitForFix())
        {
          ui.printMessage("SUCCESS", "Location fix", 1000);
          ui.gotoPage(MagnetogamaUI::MENU_PAGE);
          DEBUG_PRINTLN("GPS Location Fix");
        }
        else
        {
          DEBUG_PRINTLN("GPS Search cancelled");
          ui.gotoPage(MagnetogamaUI::MENU_PAGE);
        }
        break;

      case MagnetogamaUI::RECALL_MENU:
        ui.gotoPage(MagnetogamaUI::RECALL_PAGE);
        ui.printMessage("SORRY", "MAYBE LATER", 1000);
        ui.gotoPage(MagnetogamaUI::MENU_PAGE);
        break;

      case MagnetogamaUI::DUMP_MENU:
        ui.gotoPage(MagnetogamaUI::DUMP_PAGE);
        break;

      case MagnetogamaUI::CLEAR_MENU:
        ui.gotoPage(MagnetogamaUI::CLEAR_PAGE);
        break;

      }
      break;
    case '#':
      measure();
      break;
    }
    break;

  // In case the UI is on one of reading pages and a button is pressed
  case MagnetogamaUI::RECT_PAGE:
  case MagnetogamaUI::POLAR_PAGE:
  case MagnetogamaUI::GPS_PAGE:
    if (keypad.getState() != PRESSED) return;
    if (ui.getAcqMode() != MagnetogamaUI::MANUAL) return;
    switch (key)
    {
    case '0':
      ui.gotoPage(MagnetogamaUI::MENU_PAGE);
      // Reset the MagnetogamaData object
      data.Bcomp = BComponent();
      data.Pcomp = PolarComponent();
      data.location = Location();
      data.datetime = DateTime();
      break;
    case '8':
      if (ui.getCurrentPage() == MagnetogamaUI::RECT_PAGE)
      {
        ui.gotoPage(MagnetogamaUI::POLAR_PAGE);
      }
      else if (ui.getCurrentPage() == MagnetogamaUI::POLAR_PAGE)
      {
        ui.gotoPage(MagnetogamaUI::GPS_PAGE);
      }
      else
      {
        ui.gotoPage(MagnetogamaUI::RECT_PAGE);
      }
      break;

    case '#':
      measure();
      break;
    case '*':
      store();
      break;
    }
    break;

  case MagnetogamaUI::DUMP_PAGE:
    if (keypad.getState() != PRESSED) return;
    if (key == '#') dump2serial();
    else if (key == '*') ui.gotoPage(MagnetogamaUI::MENU_PAGE);
    break;


  // In case the UI is on clear data page, holding # for 3 seconds erase the data on SD card.
  case MagnetogamaUI::CLEAR_PAGE:
    switch (keypad.getState())
    {
    case PRESSED:
      if (key != '*') return;
      ui.gotoPage(MagnetogamaUI::MENU_PAGE);
      break;

    case HOLD:
      if (key != '#') return;
      clearSD();
      break;
    }
    break;

  }
}
