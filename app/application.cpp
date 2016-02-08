#include <user_config.h>
#include <octotherm.h>

Timer counterTimer;
void counter_loop();
unsigned long counter = 0;
TempSensorHttp *tempSensor;
Thermostat *thermostat[maxThermostats];

NtpClient ntpClient("pool.ntp.org", 30);

bool ap_started = false;

void enableWifiStation(bool enable, bool save)
{
	uint8 opmode = wifi_get_opmode_default() & ~STATION_MODE;
	if (enable) opmode |= STATION_MODE;
	if (save)
		wifi_set_opmode(opmode);
	else
		wifi_set_opmode_current(opmode);
}

void enableWifiAccessPoint(bool enable, bool save)
{
	uint8 opmode = wifi_get_opmode_default() & ~SOFTAP_MODE;
	if (enable) opmode |= SOFTAP_MODE;
	if (save)
		wifi_set_opmode(opmode);
	else
		wifi_set_opmode_current(opmode);
}

void wifi_handle_event_cb(System_Event_t *evt)
{
	os_printf("event %x\n", evt->event);

	switch (evt->event)
	{
	case EVENT_STAMODE_CONNECTED:
		os_printf("connect to ssid %s, channel %d\n",
		evt->event_info.connected.ssid,
		evt->event_info.connected.channel);
		break;
	case EVENT_STAMODE_DISCONNECTED:
		os_printf("disconnect from ssid %s, reason %d\n",
		evt->event_info.disconnected.ssid,
		evt->event_info.disconnected.reason);

		if (!ap_started)
		{
			Serial.println("Starting OWN AP CB");
			wifi_station_disconnect();
			enableWifiAccessPoint(true);
			ap_started = true;
			wifi_station_connect();
		}
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		os_printf("mode: %d -> %d\n",
		evt->event_info.auth_change.old_mode,
		evt->event_info.auth_change.new_mode);
		break;
	case EVENT_STAMODE_GOT_IP:
		os_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
		IP2STR(&evt->event_info.got_ip.ip),
		IP2STR(&evt->event_info.got_ip.mask),
		IP2STR(&evt->event_info.got_ip.gw));
		os_printf("\n");
		for (uint t=0; t < maxThermostats; t++)
			thermostat[t]->start();
		enableWifiAccessPoint(false);
		ap_started = false;
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_printf("station: " MACSTR "join, AID = %d\n",
		MAC2STR(evt->event_info.sta_connected.mac),
		evt->event_info.sta_connected.aid);
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		os_printf("station: " MACSTR "leave, AID = %d\n",
		MAC2STR(evt->event_info.sta_disconnected.mac),
		evt->event_info.sta_disconnected.aid);
		break;
	default:
		break;
	}

}

void configWifiStation(String ssid, String password, uint8_t slot, uint8_t start)
{
	struct station_config stationConf;

	uint8_t opmode = 0;
	opmode = wifi_get_opmode_default();
	wifi_set_opmode_current(opmode | STATION_MODE); //enable station for configuration

	wifi_station_ap_change(slot);
	Serial.printf("Config SLOT: %d\n", slot);
	os_memcpy(&stationConf.ssid, ssid.c_str(), 32);
	os_memcpy(&stationConf.password, password.c_str(), 32);
	wifi_station_set_config(&stationConf);

	if (start)
		wifi_station_connect();
	else
		wifi_set_opmode_current(opmode); //restore default mode
}

void initialStationConfig()
{
	struct station_config configSlots[stationConfigSlots];
	struct station_config stationConf;

	uint8_t opmode = 0;
	opmode = wifi_get_opmode();
	wifi_set_opmode_current(opmode | STATION_MODE); //enable station for configuration

	uint8_t i = wifi_station_get_ap_info(configSlots);

	if (i == 0)
	{
		wifi_set_opmode(STATION_MODE); //Initialy enable station mode by default!

		wifi_station_ap_change(0);
		Serial.printf("Initial CONFIG of SSID1\n");
		os_memcpy(&stationConf.ssid, WIFI_SSID, 32);
		os_memcpy(&stationConf.password, WIFI_PWD, 32);
		wifi_station_set_config(&stationConf);

		wifi_station_ap_change(1);
		Serial.printf("Initial CONFIG of SSID2\n");
		os_memcpy(&stationConf.ssid, "Airport", 32);
		os_memcpy(&stationConf.password, "vi240776ka", 32);
		wifi_station_set_config(&stationConf);

		wifi_station_ap_change(2);
		Serial.printf("Initial CONFIG of SSID3\n");
		os_memcpy(&stationConf.ssid, "infjust", 32);
		os_memcpy(&stationConf.password, "jujust12", 32);
		wifi_station_set_config(&stationConf);

		wifi_station_ap_change(3);
		Serial.printf("Initial CONFIG of SSID4\n");
		os_memcpy(&stationConf.ssid, "Mur", 32);
		os_memcpy(&stationConf.password, "success_60", 32);
		wifi_station_set_config(&stationConf);

	}
	else
	{
		Serial.printf("Cached station configuration:\n");
		for (uint8_t n = 0; n<i; n++)
		{
			Serial.printf("Slot %d: SSID: %s, PASSWORD: %s\n", n, configSlots[n].ssid, configSlots[n].password);
		}
	}

	wifi_station_ap_change(0); // Set slot 0 mode as default

	if (opmode & STATION_MODE)
	{
		Serial.printf("Connect initially");
		wifi_station_connect(); // if station enabled by default - try to connect as defaultSlot config
	}
	else
		wifi_set_opmode_current(opmode); //restore default mode
}

void initialAccessPointConfig()
{
	struct softap_config apconfig;

	uint8_t opmode = 0;
	opmode = wifi_get_opmode();
	wifi_set_opmode_current(opmode | SOFTAP_MODE); //enable station for configuration

	if(wifi_softap_get_config_default(&apconfig))
	{
		if (os_strncmp((const char *)apconfig.ssid, (const char *)"OctoTherm", 32) != 0)
		{
			Serial.printf("Initialy config AccessPoint");
			os_memset(apconfig.ssid, 0, sizeof(apconfig.ssid));
			os_memset(apconfig.password, 0, sizeof(apconfig.password));
			os_memcpy(&apconfig.ssid, "OctoTherm", 32);
			os_memcpy(&apconfig.password, "20040229", 32);
			apconfig.authmode = AUTH_WPA_WPA2_PSK;
			apconfig.ssid_len = 0;
			apconfig.max_connection = 4;

	//		wifi_set_opmode(mode);
			if (wifi_softap_set_config(&apconfig))
			{
				Serial.println("AP Configured!");
			}
			else
				Serial.println("AP NOT Configured - Config save failed!");
		}
		else
			Serial.printf("AccessPoint already configured.\n");
	}
	else
		Serial.println("AP NOT Started! - Get config failed!");

	wifi_set_opmode(opmode); //restore default opmode
}
void init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false);
	Serial.commandProcessing(false);

	wifi_station_ap_number_set(stationConfigSlots); // set slots number for station configs
	Serial.printf("COMPILE-IN SSID: %s, PASSWORD: %s\n", WIFI_SSID, WIFI_PWD);

	SystemClock.setTimeZone(2);

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	ActiveConfig = loadConfig();

	tempSensor = new TempSensorHttp(ActiveConfig.sensorUrl);
	thermostat[0] = new Thermostat(*tempSensor,"Office", 4000);
	thermostat[1] = new Thermostat(*tempSensor,"Kitchen", 4000);
	thermostat[2] = new Thermostat(*tempSensor,"Hall", 4000);
	thermostat[3] = new Thermostat(*tempSensor,"Bedroom", 4000);

	for(uint8_t i = 0; i< 7; i++)
	{
		for (uint t=0; t < maxThermostats; t++)
		{
			thermostat[t]->_schedule[i][0].start = 0;
			thermostat[t]->_schedule[i][0].targetTemp = 800;
			thermostat[t]->_schedule[i][1].start = 360;
			thermostat[t]->_schedule[i][1].targetTemp = 1800;
			thermostat[t]->_schedule[i][2].start = 540;
			thermostat[t]->_schedule[i][2].targetTemp = 1200;
			thermostat[t]->_schedule[i][3].start = 720;
			thermostat[t]->_schedule[i][3].targetTemp = 1500;
			thermostat[t]->_schedule[i][4].start = 1020;
			thermostat[t]->_schedule[i][4].targetTemp = 1800;
			thermostat[t]->_schedule[i][5].start = 1320;
			thermostat[t]->_schedule[i][5].targetTemp = 800;
		}
	}

	for (uint t=0; t < maxThermostats; t++)
	{
		thermostat[t]->loadStateCfg();
		thermostat[t]->loadScheduleBinCfg();
	}

	wifi_set_event_handler_cb(wifi_handle_event_cb);
	initialStationConfig();
	initialAccessPointConfig();

	if (ActiveConfig.StaEnable)
	{
//		enableWifiStation(true);
//		WifiStation.waitConnection(StaConnectOk, StaConnectTimeout, StaConnectFail);
////		WifiStation.enableDHCP(false);
////		WifiStation.setIP((String)"192.168.31.204", (String)"255.255.255.0", (String)"192.168.31.1");
////		WifiStation.setIP((String)"10.2.113.115", (String)"255.255.255.128", (String)"10.2.113.1");
//		WifiStation.enable(true);
//		WifiStation.config(ActiveConfig.StaSSID, ActiveConfig.StaPassword);
	}
	else
	{
//		enableWifiStation(false);
//		enableWifiAccessPoint(true);
//		WifiStation.enable(false);
	}

	startWebServer();

	counterTimer.initializeMs(1000, counter_loop).start();
//	tempSensor->start();
//	thermostat->start();
}

void counter_loop()
{
	counter++;
}

void StaConnectOk()
{
	Serial.println("connected to AP");
	ntpClient.requestTime();
	tempSensor->start();
	for (uint t=0; t < maxThermostats; t++)
		thermostat[t]->start();
	WifiAccessPoint.enable(false);

}

void StaConnectFail()
{
	Serial.println("connection FAILED");
	WifiStation.disconnect();
	WifiAccessPoint.config("OctoTherm", "20040229", AUTH_WPA2_PSK);
	WifiAccessPoint.enable(true);
}
