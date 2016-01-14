#include <user_config.h>
#include <octotherm.h>

Timer counterTimer;
void counter_loop();
unsigned long counter = 0;
//TempSensorHttp tempSensor("http://10.2.113.114/state");
//Thermostat thermostat(tempSensor,"Office", 4000);
TempSensorHttp *tempSensor;
Thermostat *thermostat;
void init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true);
	Serial.commandProcessing(false);

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	ActiveConfig = loadConfig();

	tempSensor = new TempSensorHttp(ActiveConfig.sensorUrl);
	thermostat = new Thermostat(*tempSensor,"Office", 4000);

	if (ActiveConfig.StaEnable)
	{
		WifiStation.waitConnection(StaConnectOk, StaConnectTimeout, StaConnectFail);
		WifiStation.enable(true);
		WifiStation.config(ActiveConfig.StaSSID, ActiveConfig.StaPassword);
	}
	else
	{
		WifiStation.enable(false);
	}

	startWebServer();

	counterTimer.initializeMs(1000, counter_loop).start();
	tempSensor->start();
}

void counter_loop()
{
	counter++;
}

void StaConnectOk()
{
	Serial.println("connected to AP");
	WifiAccessPoint.enable(false);
}

void StaConnectFail()
{
	Serial.println("connection FAILED");
	WifiStation.disconnect();
	WifiAccessPoint.config("OctoTherm", "20040229", AUTH_WPA2_PSK);
	WifiAccessPoint.enable(true);
}
