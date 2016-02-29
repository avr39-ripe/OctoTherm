#include <octotherm.h>

char gbuf[4096];
Timer counterTimer;
void counter_loop();
unsigned long counter = 0;
TempSensorHttp *tempSensor;
Thermostat *thermostat[maxThermostats];
SwitchHttp officeSwitch("http://192.168.31.204/set_state");

NtpClient ntpClient("pool.ntp.org", 300);

void STADisconnect(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason);
void STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway);

void onOfficeStateChange(bool state)
{
	Serial.printf("Office state changed to %s!\n", state ? "true" : "false");
	officeSwitch.setState(state);
}

void initialWifiConfig()
{
	struct softap_config apconfig;
	if(wifi_softap_get_config_default(&apconfig))
	{
		if (strncmp((const char *)apconfig.ssid, (const char *)"TyTherm", 32) != 0)
		{
			WifiAccessPoint.config("TyTherm", "20040229", AUTH_WPA2_PSK);

		}
		else
			Serial.printf("AccessPoint already configured.\n");
	}
	else
		Serial.println("AP NOT Started! - Get config failed!");

	if (WifiStation.getSSID().length() == 0)
	{
		WifiStation.config(WIFI_SSID, WIFI_PWD);
		WifiStation.enable(true, true);
		WifiAccessPoint.enable(false, true);
	}
	else
		Serial.printf("Station already configured.\n");
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);
	Serial.commandProcessing(false);

	Serial.printf("FreeHeap: %d\n", system_get_free_heap_size());
	Serial.printf("COMPILE-IN SSID: %s, PASSWORD: %s\n", WIFI_SSID, WIFI_PWD);

	SystemClock.setTimeZone(2);

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	initialWifiConfig(); //One-time WIFI setup

	ActiveConfig = loadConfig();

	tempSensor = new TempSensorHttp(ActiveConfig.sensorUrl);
	thermostat[0] = new Thermostat(*tempSensor,"Office", 4000);
	thermostat[0]->onStateChange(onStateChangeDelegate(&SwitchHttp::setState, &officeSwitch));
	thermostat[1] = new Thermostat(*tempSensor,"Kitchen", 4000);
	thermostat[2] = new Thermostat(*tempSensor,"Hall", 4000);
	thermostat[3] = new Thermostat(*tempSensor,"Bedroom", 4000);

	for(uint8_t i = 0; i< 7; i++)
	{
		for (auto _thermostat: thermostat)
		{
			_thermostat->_schedule[i][0].start = 0;
			_thermostat->_schedule[i][0].targetTemp = 800;
			_thermostat->_schedule[i][1].start = 360;
			_thermostat->_schedule[i][1].targetTemp = 1800;
			_thermostat->_schedule[i][2].start = 540;
			_thermostat->_schedule[i][2].targetTemp = 1200;
			_thermostat->_schedule[i][3].start = 720;
			_thermostat->_schedule[i][3].targetTemp = 1500;
			_thermostat->_schedule[i][4].start = 1020;
			_thermostat->_schedule[i][4].targetTemp = 1800;
			_thermostat->_schedule[i][5].start = 1320;
			_thermostat->_schedule[i][5].targetTemp = 800;

			_thermostat->loadStateCfg();
			_thermostat->loadScheduleBinCfg();
		}
	}

	WifiEvents.onStationDisconnect(STADisconnect);
	WifiEvents.onStationGotIP(STAGotIP);

	startWebServer();

	counterTimer.initializeMs(1000, counter_loop).start();
}

void counter_loop()
{
	counter++;
//	counter = wifi_station_get_rssi();
//	Serial.printf("RSSI: %d\n", counter);
}
void STADisconnect(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason)
{
	Serial.printf("DELEGATE DISCONNECT - SSID: %s, REASON: %d\n", ssid.c_str(), reason);

	if (!WifiAccessPoint.isEnabled())
	{
		Serial.println("Starting OWN AP DELEGATE");
		WifiStation.disconnect();
		WifiAccessPoint.enable(true);
		WifiStation.connect();
	}
}

void STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway)
{
	Serial.printf("DELEGATE GOTIP - IP: %s, MASK: %s, GW: %s\n", ip.toString().c_str(),
																mask.toString().c_str(),
																gateway.toString().c_str());

	if (WifiAccessPoint.isEnabled())
	{
		WifiAccessPoint.enable(false);
	}

	ntpClient.requestTime();
	tempSensor->start();
	for (auto _thermostat: thermostat)
		_thermostat->start();
	officeSwitch.start();
}
