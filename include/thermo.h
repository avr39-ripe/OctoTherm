/*
 * thermo.h
 *
 *  Created on: 29 дек. 2015 г.
 *      Author: shurik
 */

#ifndef INCLUDE_THERMO_H_
#define INCLUDE_THERMO_H_
#include <SmingCore/SmingCore.h>
#include <tempsensor.h>

const uint8_t maxProg = 4 ;
const uint16_t thermostatConfJsonBufSize = 1024;
const uint16_t thermostatConfFileBufSize = 4096;

struct SchedUnit
{
	SchedUnit()
	{
		start = 0;
		targetTemp = 0;
	}
	uint16_t start; // scheduled interval start in minutes since 0:00
	float targetTemp; // target temperature for this interval
};

class Thermostat
{
public:
	Thermostat(TempSensor &tempSensor, String name = "Thermostat", uint16_t refresh = 4000);
	void start();
	void stop();
	void check();
	uint8_t saveConfig();
	uint8_t loadConfig();

//	void setSched(uint8_t wDay, uint8_t progNum, uint16_t minutes, float tergetTemp);
//	SchedUnit getSched(uint8_t wDay, uint8_t progNum);
	SchedUnit _schedule[7][maxProg]; // 7 day X maxProg programs in schedule
	uint8_t getState() { return _state; };
	String getDescription() { return _name; };
private:
	String _name; // some text description of thermostat
	bool _active; //thermostat active (true), ON,  works, updates, changes its _state or turned OFF
	bool _state; // thermostat state on (true) or off (false)
	bool _manual; //thermostat in manual mode (true) or automatic schedule mode (false)
	float _manualTargetTemp = 20; //target temperature for manual mode
	float _targetTempDelta = 0.5; //delta +- for both _targetTemp and manualTargetTemp
	uint16_t _refresh; // thermostat update interval
	Timer _refreshTimer; // timer for thermostat update
	TempSensor *_tempSensor;
};



#endif /* INCLUDE_THERMO_H_ */
