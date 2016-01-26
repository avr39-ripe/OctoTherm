#include <user_config.h>
#include <octotherm.h>

Timer counterTimer;
void counter_loop();
unsigned long counter = 0;
TempSensorHttp *tempSensor;
Thermostat *thermostat;

NtpClient ntpClient("pool.ntp.org", 30);

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);
	Serial.commandProcessing(false);

	SystemClock.setTimeZone(2);

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	ActiveConfig = loadConfig();

	tempSensor = new TempSensorHttp(ActiveConfig.sensorUrl);
	thermostat = new Thermostat(*tempSensor,"Office", 4000);

	for(uint8_t i = 0; i< 7; i++)
	{
		thermostat->_schedule[i][0].start = 1439;
		thermostat->_schedule[i][0].targetTemp = 19;
		thermostat->_schedule[i][1].start = 420;
		thermostat->_schedule[i][1].targetTemp = 25;
		thermostat->_schedule[i][2].start = 1200;
		thermostat->_schedule[i][2].targetTemp = 22;
		thermostat->_schedule[i][3].start = 1380;
		thermostat->_schedule[i][3].targetTemp = 23;
	}

	//JSONIFy test
	StaticJsonBuffer<4096> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
//	JsonObject& schedule = root.createNestedObject("schedule");
//	root["schedule"] = schedule;
	for (uint8_t day = 0; day < 7; day++)
			{
				JsonArray& jsonDay = root.createNestedArray((String)day);
				//root[(String)day] = jsonDay;
				for (uint8_t prog = 0; prog < maxProg; prog++)
				{
					JsonObject& jsonProg = jsonBuffer.createObject();
					jsonProg["s"] = thermostat->_schedule[day][prog].start;
					jsonProg["tt"] = thermostat->_schedule[day][prog].targetTemp;
					jsonDay.add(jsonProg);
				}
				//root[(String)day] =
			}
	root.prettyPrintTo(Serial);

	if (ActiveConfig.StaEnable)
	{
		WifiStation.waitConnection(StaConnectOk, StaConnectTimeout, StaConnectFail);
//		WifiStation.enableDHCP(false);
//		WifiStation.setIP((String)"192.168.31.204", (String)"255.255.255.0", (String)"192.168.31.1");
//		WifiStation.setIP((String)"10.2.113.115", (String)"255.255.255.128", (String)"10.2.113.1");
		WifiStation.enable(true);
		WifiStation.config(ActiveConfig.StaSSID, ActiveConfig.StaPassword);
	}
	else
	{
		WifiStation.enable(false);
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
	thermostat->start();
	WifiAccessPoint.enable(false);

}

void StaConnectFail()
{
	Serial.println("connection FAILED");
	WifiStation.disconnect();
	WifiAccessPoint.config("OctoTherm", "20040229", AUTH_WPA2_PSK);
	WifiAccessPoint.enable(true);
}
