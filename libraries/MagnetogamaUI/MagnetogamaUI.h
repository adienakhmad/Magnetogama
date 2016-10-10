// Project Magnetogama - Magnetogama v1.0 User Interface
// Written by Adien Akhmad, Anas Handaru

#ifndef MagnetogamaUI_h
#define MagnetogamaUI_h

#include "Arduino.h"
#include "LCD5110_Graph.h"
#include "MagnetogamaData.h"

class MagnetogamaUI
{
public:
	enum ACQ_MODE
	{
		MANUAL,
		MONITOR
	};

	enum Pages
	{
		BOOT_PAGE,
		MENU_PAGE,
		READING_PAGE,
		GPS_SEARCH_PAGE,
		RECALL_PAGE,
		RECT_PAGE,
		POLAR_PAGE,
		GPS_PAGE,
		DUMP_PAGE,
		CLEAR_PAGE
	};

	enum Menus
	{
		READING_MENU,
		GPS_SEARCH_MENU,
		RECALL_MENU,
		DUMP_MENU,
		CLEAR_MENU
	};

	MagnetogamaUI(int SCK, int MOSI, int DC, int RST, int CS);
	MagnetogamaUI(int SCK, int MOSI, int DC, int RST, int CS, byte BLIGHT);
	void begin();
	void selectMenu(Menus menu);
	void gotoPage(Pages page);
	void setBatteryLevel(int _sensorbatt, int _mcubatt);
	void printMessage(char * title, char * msg, unsigned int showDuration);
	void updateReadings(MagnetogamaData newdata);
	void setLogoMsg(char* msg);
	void enableBackLight(bool state);
	void setNumOfRecord(unsigned int nrecord);
	void setACQMode(ACQ_MODE mode);
	void enableSleep();
	void disableSleep();
	MagnetogamaUI::Pages getCurrentPage();
	MagnetogamaUI::Menus getSelectedMenu();
	MagnetogamaUI::ACQ_MODE getAcqMode();

private:
	void setBackLight(bool state);
	void setActivePage(Pages page);
	void drawLogo();
	void drawMenus();
	void drawSelectedMenu();
	void drawContent(float r1, float r2, float r3);
	void drawHeader();
	void drawFooter(char* leftsymbol, char* rightsymbol, char *leftText, char *rightText);
	void drawClearDataConfirmation();
	void inputPrompt();

	String _menus[5] =
	{
		"1.Readings",
		"2.GPS Search",
		"3.Recall",
		"4.Dump",
		"5.Clear data"
	};

	byte _backLightPin;
	bool _backLightState;
	Menus _selectedMenu;
	Pages _activePage;
	ACQ_MODE _acqmode;
	unsigned int _nrecord;
	int _sensorBattLvl;
	int _mcuBattLvl;
	LCD5110 lcd;
	MagnetogamaData data;
};

#endif