#include <LCD5110_Graph.h>
#include <MagnetogamaUI.h>
#include <Keypad.h>


// Nokia 5110 PIN setup
MagnetogamaUI ui(45, 46, 47, 49, 48);

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

byte rowPins[ROWS] = {23, 28, 27, 25};
byte colPins[COLS] = {24, 22, 26};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// End of keypad pin setup

void setup()
{
  Serial.begin(9600);
  ui.begin();
  keypad.setHoldTime(3000);
  keypad.addEventListener(keypadEvent);
}

void loop()
{
  keypad.getKey();
}

void keypadEvent(KeypadEvent key)
{
  switch (keypad.getState())
  {
  case PRESSED:
    switch (key)
    {
    case '0':
      ui.backToMenu();
      break;
    case '1':
      ui.selectMenu(MagnetogamaUI::RECTMENU);
      break;
    case '2':
      ui.selectMenu(MagnetogamaUI::POLARMENU);
      break;
    case '3':
      ui.selectMenu(MagnetogamaUI::GPSMENU);
      break;
    case '4':
      ui.selectMenu(MagnetogamaUI::DUMPMENU);
      break;
    case '5':
      ui.selectMenu(MagnetogamaUI::CLEARMENU);
      break;
    case '*':
      ui.menuAction();
      break;
    case '#':
      if ((ui.getCurrentPage() == MagnetogamaUI::RECT_PAGE ) || (ui.getCurrentPage() == MagnetogamaUI::POLAR_PAGE) || (ui.getCurrentPage() == MagnetogamaUI::GPS_PAGE))
      {
        ui.printMessage("MEASURING", "Please Wait");
      }

      break;
    }
    break;
  case HOLD:
    if ((key == '#') && (ui.getCurrentPage() == MagnetogamaUI::CLEAR_PAGE)) ui.printMessage("SUCCESS", "Data Cleared.");
    break;
  }
}