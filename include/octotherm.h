#ifndef INCLUDE_OCTOTHERM_H_
#define INCLUDE_OCTOTHERM_H_
#include <configuration.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>
#include <tempsensor.h>
#include <thermo.h>

//OneWire stuff
const uint8_t onewire_pin = 2;
extern OneWire ds;

const uint8_t maxThermostats = 16;
const uint16_t thermostatsJsonBufSize = JSON_OBJECT_SIZE(20); // Termostats List Json Buffer size
const uint16_t thermostatsFileBufSize = 512;

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
