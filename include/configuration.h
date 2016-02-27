#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>

const char THERM_CONFIG_FILE[] = ".therm.conf"; // leading point for security reasons :)

struct ThermConfig
{
	ThermConfig()
	{
		StaEnable = 1; //Enable WIFI Client
		sensorUrl = "http://192.168.31.130/state";
//		sensorUrl = "http://10.2.113.114/state";
	}

	String StaSSID;
	String StaPassword;
	uint8_t StaEnable;
	String sensorUrl;

};

ThermConfig loadConfig();
void saveConfig(ThermConfig& cfg);

extern ThermConfig ActiveConfig;

#endif /* INCLUDE_CONFIGURATION_H_ */
