#include "LCD5110_Graph.h"
#include "Arduino.h"
#include "PenguinUI.h"
#include "PenguinData.h"

extern uint8_t SmallFont[];
extern unsigned char TinyFont[];
extern uint8_t EagleLogo[];
extern uint8_t PenguinLogo[];

// Constructor
PenguinUI::PenguinUI(int SCK, int MOSI, int DC, int RST, int CS)
{
	lcd = LCD5110(SCK, MOSI, DC, RST, CS);
	data = PenguinData();
	_backLightState = false;
}

PenguinUI::PenguinUI(int SCK, int MOSI, int DC, int RST, int CS, byte BLIGHT)
{
	lcd = LCD5110(SCK, MOSI, DC, RST, CS);
	data = PenguinData();
	_backLightState = false;
	_backLightPin = BLIGHT;
	pinMode(_backLightPin, OUTPUT);
}

// -- Public Function --
void PenguinUI::begin()
{
	lcd.InitLCD();
	drawLogo();
	setActivePage(BOOT_PAGE);
}

void PenguinUI::setNumOfRecord(unsigned int nrecord)
{
	_nrecord = nrecord;
}

void PenguinUI::toggleBackLight()
{
	if (_backLightPin == 0) return; // return if backlight pin is not defined.
	setBackLight(!_backLightState);
}

void PenguinUI::updateReadings(PenguinData newdata)
{
	data = newdata;
}

void PenguinUI::gotoPage(Pages page)
{
	lcd.clrScr();

	switch (page)
	{
	case MENU_PAGE:
		drawMenus();
		setActivePage(MENU_PAGE);
		break;
	case RECT_PAGE:
		setActivePage(RECT_PAGE);
		drawHeader();
		drawContent(data.Bcomp.X, data.Bcomp.Y, data.Bcomp.Z);
		drawFooter("*", "#", "STORE", "MEASURE");
		break;
	case POLAR_PAGE:
		setActivePage(POLAR_PAGE);
		drawHeader();
		drawContent(data.Pcomp.R, data.Pcomp.D, data.Pcomp.I);
		drawFooter("*", "#", "STORE", "MEASURE");
		break;
	case GPS_PAGE:
		setActivePage(GPS_PAGE);
		drawHeader();
		drawContent(data.location.latitude, data.location.longitude, data.location.elevation);
		drawFooter("*", "#", "STORE", "MEASURE");
		break;
	case DUMP_PAGE:
		printMessage("SERIAL DUMP", "19200 bps", 0);
		drawFooter("*", "#", "CANCEL", "CONTINUE");
		setActivePage(DUMP_PAGE);
		break;
	case CLEAR_PAGE:
		drawClearDataConfirmation();
		drawFooter("*", "#", "CANCEL", "HOLD");
		setActivePage(CLEAR_PAGE);
		break;
	case GPS_SEARCH_PAGE:
		printMessage("LOCKING SAT", "Please wait", 0);
		drawFooter("", "#", "", "CANCEL");
		setActivePage(GPS_SEARCH_PAGE);
		break;
	case RECALL_PAGE:
		printMessage("Recall Page", "Please wait", 0);
		drawFooter("*", "#", "CANCEL", "CONTINUE");
		setActivePage(RECALL_PAGE);
		break;

	}

	lcd.update();


}

void PenguinUI::selectMenu(Menus menu)
{
	_selectedMenu = menu;
	setActivePage(MENU_PAGE);
	drawMenus();
}

void PenguinUI::setBatteryLevel(int sensorBatt, int mcuBatt)
{
	_sensorBattLvl = sensorBatt;
	_mcuBattLvl = mcuBatt;
}

// -- End of Public Function --

// Private function
void PenguinUI::setActivePage(Pages page)
{
	_activePage = page;
}

void PenguinUI::setBackLight(bool state)
{
	digitalWrite(_backLightPin, state);
	_backLightState = state;
}

void PenguinUI::drawLogo()
{
	lcd.clrScr();
	lcd.drawBitmap(0, 0, PenguinLogo, 84, 48);
	lcd.update();
}

void PenguinUI::setLogoMsg(char * msg)
{
	lcd.setFont(TinyFont);
	lcd.print(msg, 30, 5);
	lcd.update();
}

void PenguinUI::drawHeader()
{
	lcd.setFont(SmallFont);
	lcd.print("n:", 2, 1);
	lcd.printNumI(_nrecord, 14, 1, 4, '0');
	lcd.drawRect(67, 0, 81, 8);
	lcd.drawRect(65, 3, 66, 5);
	lcd.drawRect(64, 0, 65, 1);
	lcd.drawRect(64, 7, 65, 8);

	int battBars1 = 0;
	int battBars2 = 0;

	if (_mcuBattLvl > 75)battBars1 = 4;
	else if (_mcuBattLvl > 50)battBars1 = 3;
	else if (_mcuBattLvl > 25)battBars1 = 2;
	else if (_mcuBattLvl > 5) battBars1 = 1;
	else battBars1 = 0;
	for (int i = 0; i < battBars1; i++)
		lcd.drawRect(78 - (i * 3), 2, 79 - (i * 3), 3);

	if (_sensorBattLvl > 75)battBars2 = 4;
	else if (_sensorBattLvl > 50)battBars2 = 3;
	else if (_sensorBattLvl > 25)battBars2 = 2;
	else if (_sensorBattLvl > 5) battBars2 = 1;
	for (int i = 0; i < battBars2; i++)
		lcd.drawRect(78 - (i * 3), 5, 79 - (i * 3), 6);

	for (int i = 0; i < 9; i++)
	{	for (int j = 0; j < 64; j++)
			lcd.invPixel(j, i);
	}

}

void PenguinUI::drawFooter(char *leftsymbol, char *rightsymbol, char *leftText, char *rightText)
{
	lcd.setFont(TinyFont);
	lcd.print(":", 49, 43);
	lcd.print(":", 7, 43);

	lcd.print(leftText, 11, 43);
	lcd.print(rightText, 53, 43);

	lcd.setFont(SmallFont);
	lcd.print(leftsymbol, 0, 42);
	lcd.print(rightsymbol, 42, 42);

}


void PenguinUI::drawSelectedMenu()
{
	lcd.setFont(SmallFont);

	for (int i = 0; i < sizeof(_menus) / sizeof(String); i++)
	{
		lcd.invertText(i == _selectedMenu);
		lcd.print(_menus[i], 0, i * 8);
	}

	lcd.invertText(false);
}

void PenguinUI::drawMenus()
{
	lcd.clrScr();
	drawSelectedMenu();
	drawFooter("*", "#", "SELECT", "MEASURE");
	lcd.update();
}

void PenguinUI::drawContent(float r1, float r2, float r3)
{
	lcd.setFont(SmallFont);

	switch (_activePage)
	{
	case RECT_PAGE:
		lcd.print("X", 4, 10);
		lcd.print("Y", 4, 18);
		lcd.print("Z", 4, 26);

		lcd.printNumF(r1, 2, 12, 10, '.', 9, ' ');
		lcd.printNumF(r2, 2, 12, 18, '.', 9, ' ');
		lcd.printNumF(r3, 2, 12, 26, '.', 9, ' ');

		lcd.print("nT", 68, 10);
		lcd.print("nT", 68, 18);
		lcd.print("nT", 68, 26);
		break;

	case POLAR_PAGE:
		lcd.print("R", 4, 10);
		lcd.print("D", 4, 18);
		lcd.print("I", 4, 26);

		lcd.printNumF(r1, 2, 12, 10, '.', 9, ' ');
		lcd.printNumF(r2, 3, 12, 18, '.', 10, ' ');
		lcd.printNumF(r3, 3, 12, 26, '.', 10, ' ');

		lcd.print("nT", 68, 10);
		lcd.setFont(TinyFont);
		lcd.print("o", 74, 17);
		lcd.print("o", 74, 25);
		lcd.setFont(SmallFont);
		break;

	case GPS_PAGE:
		lcd.print("X", 4, 10);
		lcd.print("Y", 4, 18);
		lcd.print("Z", 4, 26);

		lcd.printNumF(r1, 5, 12, 10, '.', 10, ' ');
		lcd.printNumF(r2, 5, 12, 18, '.', 10, ' ');
		lcd.printNumF(r3, 0, 12, 26, '.', 10, ' ');

		lcd.setFont(TinyFont);
		lcd.print("o", 74, 9);
		lcd.print("o", 74, 17);
		lcd.setFont(SmallFont);
		lcd.print("m", 72, 26);
		break;
	}


	lcd.setFont(TinyFont);

	lcd.print("--/--/--", 5, 35);
	lcd.printNumI(data.datetime.day, 5, 35, 2, '0');
	lcd.printNumI(data.datetime.month, 17, 35, 2, '0');
	lcd.printNumI(data.datetime.year, 29, 35, 4, '0');
	lcd.print("--:--:--", 48, 35);
	lcd.printNumI(data.datetime.hour, 48, 35, 2, '0');
	lcd.printNumI(data.datetime.minute, 60, 35, 2, '0');
	lcd.printNumI(data.datetime.second, 72, 35, 2, '0');
}

void PenguinUI::drawClearDataConfirmation()
{
	lcd.setFont(SmallFont);
	lcd.print("Hold '#' for", CENTER, 4);
	lcd.print("3 seconds to", CENTER, 13);
	lcd.print("clear all data", CENTER, 22);
}

void PenguinUI::printMessage(char * title, char * msg, unsigned int showDur)
{
	lcd.clrScr();

	lcd.setFont(SmallFont);
	lcd.drawRect(0, 17, 83, 30);
	lcd.print(title, CENTER, 8);
	lcd.print(msg, CENTER, 20);

	for (int i = 6; i < 17; i++)
	{	for (int j = 0; j < 84; j++)
			lcd.invPixel(j, i);
	}

	lcd.update();
	if (showDur > 0) delay(showDur);
}

void PenguinUI::setACQMode(ACQ_MODE mode)
{
	_acqmode = mode;
}

void PenguinUI::enableSleep()
{
	lcd.enableSleep();
}

void PenguinUI::disableSleep()
{
	lcd.disableSleep();
}

PenguinUI::Pages PenguinUI::getCurrentPage()
{
	return _activePage;
}

PenguinUI::Menus PenguinUI::getSelectedMenu()
{
	return _selectedMenu;
}

PenguinUI::ACQ_MODE PenguinUI::getAcqMode()
{
	return _acqmode;
}