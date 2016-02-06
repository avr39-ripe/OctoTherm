#include <user_config.h>
#include <octotherm.h>
#include <user_interface.h>

void wifi_handle_event_cb(System_Event_t *evt);
Timer counterTimer;
void counter_loop();
unsigned long counter = 0;
TempSensorHttp *tempSensor;
Thermostat *thermostat[maxThermostats];

NtpClient ntpClient("pool.ntp.org", 30);

void wifi_handle_event_cb(System_Event_t *evt)
{
	os_printf("event %x\n", evt->event);
	switch (evt->event) {
	case EVENT_STAMODE_CONNECTED:
	os_printf("connect to ssid %s, channel %d\n",
	evt->event_info.connected.ssid,
	evt->event_info.connected.channel);
	break;
	case EVENT_STAMODE_DISCONNECTED:
	os_printf("disconnect from ssid %s, reason %d\n",
	evt->event_info.disconnected.ssid,
	evt->event_info.disconnected.reason);
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

	struct station_config iconfig[5];
	int ii;
	ii = wifi_station_get_ap_info(iconfig);

	Serial.printf("Cached conf:\n");
	for (uint8_t n = 0; n<ii; n++)
	{
		Serial.printf("%d: SSID: %s, PASSWORD: %s\n", n, iconfig[n].ssid, iconfig[n].password);
	};
	ntpClient.requestTime();
	tempSensor->start();
    Serial.printf("FreeHeap: %d\n", system_get_free_heap_size());
    system_print_meminfo();
	for (uint t=0; t < maxThermostats; t++)
		thermostat[t]->start();
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

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false);
	Serial.commandProcessing(false);

	SystemClock.setTimeZone(2);

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	ActiveConfig = loadConfig();

	tempSensor = new TempSensorHttp(ActiveConfig.sensorUrl);
//	thermostat[0] = new Thermostat(*tempSensor,"Office", 4000);
//	thermostat[1] = new Thermostat(*tempSensor,"Kitchen", 4000);
//	thermostat[2] = new Thermostat(*tempSensor,"Hall", 4000);
//	thermostat[3] = new Thermostat(*tempSensor,"Bedroom", 4000);

	for (uint t=0; t < maxThermostats; t++)
	{
		thermostat[t] = new Thermostat(*tempSensor,"Office" + (String)t, 4000);
	}
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

//	struct station_config stationConf;
//
//	os_memset(&stationConf, 0, sizeof(struct station_config));
//
//	os_sprintf((char *)stationConf.ssid, "%s", "Airport");
//	os_sprintf((char *)stationConf.password, "%s", "vi240776ka");
//	stationConf.bssid_set = 0;
//
//	wifi_station_set_config(&stationConf);
//	wifi_station_set_auto_connect(true);
//	wifi_set_opmode_current(STATION_MODE);
//	wifi_set_event_handler_cb(wifi_handle_event_cb);
//	wifi_station_connect();
	struct station_config config[5];
	int i = wifi_station_get_ap_info(config);

	Serial.printf("Cached conf:\n");
	for (uint8_t n = 0; n<i; n++)
	{
		Serial.printf("%d: SSID: %s, PASSWORD: %s\n", n, config[n].ssid, config[n].password);
	}
	wifi_set_opmode( STATION_MODE );
	wifi_set_event_handler_cb(wifi_handle_event_cb);
	wifi_station_ap_change(0);

	const char ssid[32] = "Airport";
	const char password[32] = "vi240776ka";

	struct station_config stationConf;

	if (i == 0)
	{
		Serial.printf("Initial CONFIG of SSID\n");
		os_memcpy(&stationConf.ssid, ssid, 32);
		os_memcpy(&stationConf.password, password, 32);
		wifi_station_set_config(&stationConf);
	}
	wifi_station_connect();





	if (ActiveConfig.StaEnable)
	{
//		WifiStation.waitConnection(StaConnectOk, StaConnectTimeout, StaConnectFail);
//		WifiStation.enable(true);
//		WifiStation.config(ActiveConfig.StaSSID, ActiveConfig.StaPassword);



	}
	else
	{
//		WifiStation.enable(false);
	}

	startWebServer();

	counterTimer.initializeMs(1000, counter_loop).start();
//	tempSensor->start();
//	thermostat->start();
    Serial.printf("FreeHeap: %d\n", system_get_free_heap_size());
    system_print_meminfo();
}

void counter_loop()
{
	counter++;
	Serial.printf("FreeHeap: %d\n", system_get_free_heap_size());
}


void StaConnectOk()
{
	Serial.println("connected to AP");
	ntpClient.requestTime();
	tempSensor->start();
    Serial.printf("FreeHeap: %d\n", system_get_free_heap_size());
    system_print_meminfo();
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
