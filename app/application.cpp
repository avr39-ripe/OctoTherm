#include <user_config.h>
#include <octotherm.h>

Timer counterTimer;
void counter_loop();
unsigned long counter = 0;
float temperature = 0; // TyTherm accuired temperature

HttpClient tyTherm;

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false);
	Serial.commandProcessing(false);

	//SET higher CPU freq & disable wifi sleep
	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	ActiveConfig = loadConfig();

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

	counterTimer.initializeMs(5000, counter_loop).start();
}

void onDataSent(HttpClient& client, bool successful)
{
//	if (successful)
//		Serial.println("Success sent");
//	else
//		Serial.println("Failed");
//
	String response = client.getResponseString();
//	Serial.println("Server response: '" + response + "'");
	if (response.length() > 0)
	{
		StaticJsonBuffer<ConfigJsonBufferSize> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(response);
		root.prettyPrintTo(Serial); //Uncomment it for debuging
		if (root["temperature"].success())
		{
			temperature = root["temperature"];
		}


	}
}

void counter_loop()
{
	counter++;

	if (tyTherm.isProcessing())
		return; // We need to wait while request processing was completed
	else
		tyTherm.downloadString("http://10.2.113.42/state", onDataSent);
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
