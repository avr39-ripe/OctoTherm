#include <user_config.h>
#include <octotherm.h>

Timer counterTimer;
void counter_loop();
unsigned long counter = 0;
TempSensorHttp *tempSensor;
Thermostat *thermostat[maxThermostats];

NtpClient ntpClient("pool.ntp.org", 300);

void STADisconnect(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason);
void STAGotIP(IPAddress ip, IPAddress mask, IPAddress gateway);

void initialAccessPointConfig()
{
	struct softap_config apconfig;

	uint8_t opmode = 0;
	opmode = wifi_get_opmode_default();

	if(wifi_softap_get_config_default(&apconfig))
	{
		if (os_strncmp((const char *)apconfig.ssid, (const char *)"OctoTherm", 32) != 0)
		{
			wifi_set_opmode_current(opmode | SOFTAP_MODE); //enable station for configuration

			Serial.printf("Initialy config AccessPoint\n");
			os_memset(apconfig.ssid, 0, sizeof(apconfig.ssid));
			os_memset(apconfig.password, 0, sizeof(apconfig.password));
			os_memcpy(&apconfig.ssid, "OctoTherm", 32);
			os_memcpy(&apconfig.password, "20040229", 32);
			apconfig.authmode = AUTH_WPA_WPA2_PSK;
			apconfig.ssid_len = 0;
			apconfig.max_connection = 4;

			if (wifi_softap_set_config(&apconfig))
			{
				Serial.println("AP Configured!");
			}
			else
			{
				Serial.println("AP NOT Configured - Config save failed!");
			}
			wifi_set_opmode_current(opmode); //restore current opmode
		}
		else
			Serial.printf("AccessPoint already configured.\n");
	}
	else
		Serial.println("AP NOT Started! - Get config failed!");
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false);
	Serial.commandProcessing(false);

	Serial.printf("COMPILE-IN SSID: %s, PASSWORD: %s\n", WIFI_SSID, WIFI_PWD);

	SystemClock.setTimeZone(2);

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	initialAccessPointConfig(); //One-time SOFTAP setup

	ActiveConfig = loadConfig();

	tempSensor = new TempSensorHttp(ActiveConfig.sensorUrl);
	thermostat[0] = new Thermostat(*tempSensor,"Office", 4000);
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
}
