#include <user_config.h>
#include <octotherm.h>
#include <user_interface.h>

const char ssid[32] = "Airport";
const char password[32] = "vi240776ka";
const char APssid[32] = "OctoTherm";
const char APpassword[32] = "20040229";

struct station_config config[5];
struct station_config stationConf;

//SOFT-AP
struct softap_config APconfig;
bool ap_started = false;
void wifi_handle_event_cb(System_Event_t *evt);
Timer counterTimer;
void counter_loop();
unsigned long counter = 0;
TempSensorHttp *tempSensor;
Thermostat *thermostat[maxThermostats];

NtpClient ntpClient("pool.ntp.org", 120);

void startAP(uint8_t mode)
{
	struct softap_config apconfig;

	   if(wifi_softap_get_config(&apconfig))
	   {
	      os_memset(apconfig.ssid, 0, sizeof(apconfig.ssid));
	      os_memset(apconfig.password, 0, sizeof(apconfig.password));
	      os_memcpy(&apconfig.ssid, "OctoTherm", 32);
	      os_memcpy(&apconfig.password, "20040229", 32);
	      apconfig.authmode = AUTH_WPA_WPA2_PSK;
	      apconfig.ssid_len = 0;
	      apconfig.max_connection = 4;
	      wifi_set_opmode(mode);
	      if (wifi_softap_set_config(&apconfig))
	      {
		      Serial.println("AP Started");
	      }
	      else
	    	  Serial.println("AP NOT Started - CFG!!!");
	   }
	   else
		   Serial.println("AP NOT Started!!!");

}

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

	if (!ap_started)
	{
		Serial.println("Starting OWN AP CB");
		startAP(STATIONAP_MODE);
	    ap_started = true;
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
	wifi_set_opmode(STATION_MODE);
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



	wifi_station_ap_number_set(3);
	wifi_set_opmode(STATION_MODE);
	wifi_set_event_handler_cb(wifi_handle_event_cb);

	if (ActiveConfig.StaEnable)
	{
//		WifiStation.waitConnection(StaConnectOk, StaConnectTimeout, StaConnectFail);
//		WifiStation.enable(true);
//		WifiStation.config(ActiveConfig.StaSSID, ActiveConfig.StaPassword);

		int i = wifi_station_get_ap_info(config);

		Serial.printf("Cached conf:\n");
		for (uint8_t n = 0; n<i; n++)
		{
			Serial.printf("%d: SSID: %s, PASSWORD: %s\n", n, config[n].ssid, config[n].password);
		}

		if (i == 0)
		{
			wifi_station_ap_change(0);
			Serial.printf("Initial CONFIG of SSID1\n");
			os_memcpy(&stationConf.ssid, ssid, 32);
			os_memcpy(&stationConf.password, password, 32);
			wifi_station_set_config(&stationConf);

			wifi_station_ap_change(1);
			Serial.printf("Initial CONFIG of SSID1\n");
			os_memcpy(&stationConf.ssid, "infjust", 32);
			os_memcpy(&stationConf.password, "jujust12", 32);
			wifi_station_set_config(&stationConf);

			wifi_station_ap_change(2);
			Serial.printf("Initial CONFIG of SSID1\n");
			os_memcpy(&stationConf.ssid, "Mur", 32);
			os_memcpy(&stationConf.password, "success", 32);
			wifi_station_set_config(&stationConf);
}
		wifi_station_ap_change(0);
		wifi_station_connect();

	}
	else
	{
		Serial.println("Start OWN AP - INIT!!!");
		ap_started = true;
		startAP(SOFTAP_MODE);

//	    wifi_set_opmode( STATIONAP_MODE );
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
