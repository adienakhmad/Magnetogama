void checkBattLvl()
{
	int batt1adc = analogRead(A1);
	int batt2adc = analogRead(A2);

	float batt1adcv = batt1adc * (5 / 1023.0);
	float batt2adcv = batt2adc * (5 / 1023.0);

	float batt1v = batt1adcv * (20 / 5);
	float batt2v = batt2adcv * 2;

	float batt1percent = ((batt1v - 14) / (16 - 14)) * 100;
	float batt2percent = ((batt2v - 6.7) / (8 - 6.7)) * 100;

	ui.setBatteryLevel((int) batt1percent, (int) batt2percent);
}

// GPS Related
void gpsfeed()
{
	while (Serial2.available() > 0)
	{
		gps.encode(Serial2.read());
	}
}

void gpsSmartFeed(unsigned long ms)
{
	unsigned long start = millis();
	do
	{
		gpsfeed();
	} while (millis() - start < ms);
}

bool gpsWaitForFix()
{
	if (hw_error.gps)
	{
		ui.printMessage("ERROR", "No GPS", 1000);
		ui.gotoPage(MagnetogamaUI::MENU_PAGE);
		return false;
	}

	bool cancelled = false;

	while (true)
	{
		DEBUG_PRINTLN("Inside GPS Wait for fix");
		gpsfeed();
		char key = keypad.getKey();
		if (gps.location.isValid() && gps.altitude.isValid())
			break;

		else if (key == '#')
		{
			cancelled = true;
			break;
		}
	}

	return !cancelled;
}

// ADC Related Function

void averageInput(unsigned int ncount)
{
	float r1, r2, r3;

	// Efficient Input Cycling
	// to learn further, read on datasheet page 21, figure 19 : Cycling the ADS1256 Input Multiplexer
	adc.waitDRDY(); // wait for DRDY to go low before changing multiplexer register
	adc.switchChannel(1, 3);
	r1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN2 and AIN3

	adc.waitDRDY();
	adc.switchChannel(0, 3);
	r2 = adc.readCurrentChannel(); //// DOUT arriving here are from MUX AIN1 and AIN3

	adc.waitDRDY();
	adc.switchChannel(2, 3); // switch back to MUX AIN0 and AIN3
	r3 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN3 and AIN0


	//print the result.
	DEBUG_PRINT2(r1, 10);
	DEBUG_PRINT("\t");
	DEBUG_PRINT2(r2, 10);
	DEBUG_PRINT("\t");
	DEBUG_PRINTLN2(r3, 10);

	// Calculating the new average
	data.Bcomp.newAverage(r1, r2, r3, ncount);
}


void cycleInputFor(unsigned long duration)
{
	DEBUG_PRINT("Measuring for ");
	DEBUG_PRINT(duration / 1000);
	DEBUG_PRINTLN(" seconds.");

	unsigned long  timestamp = millis();

	data.Bcomp.update(0, 0, 0);
	unsigned int count = 0;

	while (millis() - timestamp < duration)
	{
		count++;
		averageInput(count);
	}

	//Calculate Polar Component after averaging completed
	data.Pcomp.calculate(data.Bcomp);

	DEBUG_PRINT("Duration: ");
	DEBUG_PRINT((millis() - timestamp) / 1000);
	DEBUG_PRINTLN(" seconds.");

	DEBUG_PRINTLN("Sensor averaged value: ");
	DEBUG_PRINT2(data.Bcomp.X, 10);
	DEBUG_PRINT("\t");
	DEBUG_PRINT2(data.Bcomp.Y, 10);
	DEBUG_PRINT("\t");
	DEBUG_PRINTLN2(data.Bcomp.Z, 10);

}

void measure()
{
	if (hw_error.adc)
	{
		ui.printMessage("ERROR", "ADC MUX ERROR", 1000);
		return;
	}

	ui.setACQMode(MagnetogamaUI::MANUAL);
	ui.printMessage("MEASURING", "Please Wait", 0);

	cycleInputFor(_measureDuration);
	gpsSmartFeed(1000);

	// Update Location from GPS
	if (gps.location.isUpdated() && gps.location.isValid() && gps.altitude.isValid() && gps.altitude.isUpdated())
	{
		data.location.update(gps.location.lat(), gps.location.lng(), gps.altitude.meters());
		DEBUG_PRINTLN("GPS location is updated and valid");
	}
	else
	{
		data.location.update(0, 0, 0);
		DEBUG_PRINTLN("GPS location is not updated nor valid");
	}

	// Update time from GPS

	if (gps.time.isUpdated() && gps.time.isValid() && gps.date.isUpdated() && gps.date.isValid())
	{
		data.datetime.update(gps.date.day(), gps.date.month(), gps.date.year(), gps.time.hour(), gps.time.minute(), gps.time.second());
		DEBUG_PRINTLN("GPS time is updated and valid");
	}

	else
	{
		data.datetime.update(0, 0, 0, 0, 0, 0);
		DEBUG_PRINTLN("GPS time is not updated nor valid");
	}

	DEBUG_PRINTLN("GPS Validity Check: loc, date, time.");
	DEBUG_PRINTLN(gps.location.isValid());
	DEBUG_PRINTLN(gps.date.isValid());
	DEBUG_PRINTLN(gps.time.isValid());

	DEBUG_PRINT("Lat: ");
	DEBUG_PRINTLN(data.location.latitude);
	DEBUG_PRINT("Lon: ");
	DEBUG_PRINTLN(data.location.longitude);
	DEBUG_PRINT("Elev: ");
	DEBUG_PRINTLN(data.location.elevation);

	DEBUG_PRINT("Day: ");
	DEBUG_PRINTLN(data.datetime.day);
	DEBUG_PRINT("Month: ");
	DEBUG_PRINTLN(data.datetime.month);
	DEBUG_PRINT("Year: ");
	DEBUG_PRINTLN(data.datetime.year);

	DEBUG_PRINT("Hour: ");
	DEBUG_PRINTLN(data.datetime.hour);
	DEBUG_PRINT("Minute: ");
	DEBUG_PRINTLN(data.datetime.minute);
	DEBUG_PRINT("Second: ");
	DEBUG_PRINTLN(data.datetime.second);

	// After all data has been updated, pass it to UI
	ui.updateReadings(data);
	ui.gotoPage(MagnetogamaUI::POLAR_PAGE);

}

void monitor()
{
	if (hw_error.adc)
	{
		ui.printMessage("ERROR", "ADC MUX ERROR", 1000);
		return;
	}

	ui.setACQMode(MagnetogamaUI::MONITOR);

	bool _isFrozen = false;
	bool _isStop = false;

	while (!_isStop)
	{

		float r1, r2, r3;
		// Efficient Input Cycling
		// to learn further, read on datasheet page 21, figure 19 : Cycling the ADS1256 Input Multiplexer
		adc.waitDRDY(); // wait for DRDY to go low before changing multiplexer register
		adc.switchChannel(1, 3);
		r1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN2 and AIN3

		adc.waitDRDY();
		adc.switchChannel(0, 3);
		r2 = adc.readCurrentChannel(); //// DOUT arriving here are from MUX AIN1 and AIN3

		adc.waitDRDY();
		adc.switchChannel(2, 3); // switch back to MUX AIN0 and AIN3
		r3 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN3 and AIN0

		gpsfeed();

		if (!_isFrozen)
		{
			data.Bcomp.update(r1, r2, r3);
			data.Pcomp.calculate(data.Bcomp);

			// Update Location from GPS
			if (gps.location.isUpdated() && gps.location.isValid() && gps.altitude.isValid() && gps.altitude.isUpdated())
				data.location.update(gps.location.lat(), gps.location.lng(), gps.altitude.meters());
			else
				data.location.update(0, 0, 0);


			// Update time from GPS

			if (gps.time.isUpdated() && gps.time.isValid() && gps.date.isUpdated() && gps.date.isValid())
				data.datetime.update(gps.date.day(), gps.date.month(), gps.date.year(), gps.time.hour(), gps.time.minute(), gps.time.second());
			else
				data.datetime.update(0, 0, 0, 0, 0, 0);

			ui.updateReadings(data);
			ui.gotoPage(MagnetogamaUI::RECT_PAGE);
		}

		char key = keypad.getKey();

		if (keypad.getState() != PRESSED) continue;
			switch (key)
			{
			case '#':
				_isFrozen =  !_isFrozen;
				break;
			case '*':
				if (_isFrozen) store();
				break;
			case '0':
				_isStop = true;
				break;

			case '8':
				if (_isFrozen)
				{
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
				}
				break;
			}

	}

	ui.setACQMode(MagnetogamaUI::MANUAL);
	ui.gotoPage(MagnetogamaUI::MENU_PAGE);


}

//SD Card related
void timestamp(uint16_t* date, uint16_t* time)
{
	*date = FAT_DATE(data.datetime.year, data.datetime.month, data.datetime.day);
	*time = FAT_TIME(data.datetime.hour, data.datetime.minute, data.datetime.second);
}

void store()
{
	if (hw_error.sdcard)
	{
		ui.printMessage("ERROR", "No SD Card", 1000);
		ui.gotoPage(MagnetogamaUI::MENU_PAGE);
		return;
	}

	char name[] = FILENAME;
	// Logfile file.
	ofstream logfile;
	logfile.open(name, ios::out | ios::app);

	if (!logfile.is_open())
	{
		ui.printMessage("ERROR", "file error", 500);
		ui.gotoPage(MagnetogamaUI::RECT_PAGE);
		return;
	}

	char buff[80];
	obufstream bout(buff, sizeof(buff));

	bout 	<< setw(10) << setprecision(3) 	<< data.Bcomp.X << ','
	        << setw(10) << setprecision(3) 	<< data.Bcomp.Y << ','
	        << setw(10) << setprecision(3) 	<< data.Bcomp.Z << ','
	        << setw(10) << setprecision(5) 	<< data.location.latitude << ','
	        << setw(10) << setprecision(5) 	<< data.location.longitude << ','
	        << setw(7) 	<< setprecision(2) 	<< data.location.elevation << ','
	        << setw(2) 	<< setfill('0') 	<< (int) data.datetime.day
	        << setw(2) 	<< setfill('0')		<< (int) data.datetime.month
	        << setw(4) 	<< setfill('0')		<< (int) data.datetime.year
	        << setw(2) 	<< setfill('0')		<< (int) data.datetime.hour
	        << setw(2) 	<< setfill('0')		<< (int) data.datetime.minute
	        << setw(2) 	<< setfill('0')		<< (int) data.datetime.second
	        << endl;

	logfile << buff << flush;
	logfile.close();
	nrecord++;
	ui.setNumOfRecord(nrecord);
	ui.printMessage("SUCCESS", "Data Stored", 300);
	ui.gotoPage(MagnetogamaUI::RECT_PAGE);
}

void dump2serial()
{
	if (hw_error.sdcard)
	{
		ui.printMessage("ERROR", "No SD Card", 1000);
		ui.gotoPage(MagnetogamaUI::MENU_PAGE);
		return;
	}

	ui.printMessage("DUMPING", "Please wait..", 0);
	char buffer[80];

	ArduinoOutStream cout(Serial);
	ifstream infile(FILENAME);

	while (infile.getline(buffer, 80, '\n'))
	{
		cout << buffer << endl;
	}

	ui.printMessage("FINISHED", "Data dumped.", 1000);
	ui.gotoPage(MagnetogamaUI::MENU_PAGE);
}

void clearSD()
{
	if (hw_error.sdcard)
	{
		ui.printMessage("ERROR", "No SD Card", 1000);
		ui.gotoPage(MagnetogamaUI::MENU_PAGE);
		return;
	}

	ofstream logfile;
	logfile.open(FILENAME, ios::out | ios::trunc);
	if (!logfile.is_open())
	{
		ui.printMessage("ERROR", "file error", 500);
		ui.gotoPage(MagnetogamaUI::RECT_PAGE);
		return;
	}
	logfile.close();
	ui.printMessage("SUCCESS", "Data Cleared.", 1000);
	ui.gotoPage(MagnetogamaUI::MENU_PAGE);
	nrecord = 0;
	ui.setNumOfRecord(nrecord);
}
