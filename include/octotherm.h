#ifndef INCLUDE_OCTOTHERM_H_
#define INCLUDE_OCTOTHERM_H_
#include <configuration.h>
#include <SmingCore.h>
#include <libraries/OneWire/OneWire.h>
#include <tempsensor.h>
#include <switch.h>
#include <thermo.h>

//WIFI Stuff
//const uint8_t stationCacheSize = 4;
//extern struct station_config configSlots[stationCacheSize];
//extern uint8_t stationCurrentCacheId;
//extern uint8_t currentStaCacheSize;
extern bool ap_started;
extern void enableWifiStation(bool enable, bool save = false);
extern void configWifiStation(String ssid, String password, bool start = true);
extern void enableWifiAccessPoint(bool enable, bool save = false);

//OneWire stuff
const uint8_t onewire_pin = 2;
extern OneWire ds;

const uint8_t maxThermostats = 4;
const uint16_t thermostatsJsonBufSize = JSON_OBJECT_SIZE(10); // Termostats List Json Buffer size
const uint16_t thermostatsFileBufSize = 256;

extern TempSensorHttp *tempSensor;
extern Thermostat *thermostat[maxThermostats];

extern unsigned long counter; // Kind of heartbeat counter
extern float temperature; // TyTherm accuired temperature

const uint16_t ConfigJsonBufferSize = 300; // Application configuration JsonBuffer size ,increase it if you have large config
const uint16_t ConfigFileBufferSize = 2500; // Application configuration FileBuffer size ,increase it if you have large config

//Webserver
void startWebServer();

//STA disconnecter
const uint8_t StaConnectTimeout = 20; //15 sec to connect in STA mode
void StaConnectOk();
void StaConnectFail();

#endif /* INCLUDE_HEATCONTROL_H_ */
