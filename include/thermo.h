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

const uint8_t maxProg = 6 ;

const uint16_t scheduleJsonBufSize = JSON_OBJECT_SIZE(120) + JSON_ARRAY_SIZE(42);
const uint16_t scheduleFileBufSize = 1152;
const uint16_t stateJsonBufSize = JSON_OBJECT_SIZE(10);
const uint16_t stateFileBufSize = 256;

struct SchedUnit
{
	SchedUnit()
	{
		start = 0;
		targetTemp = 0;
	}
	uint16_t start; // scheduled interval start in minutes since 0:00
	uint16_t targetTemp; // target temperature for this interval
};

class Thermostat
{
public:
	Thermostat(TempSensor &tempSensor, String name = "Thermostat", uint16_t refresh = 4000);
	void start();
	void stop();
	void check();
	void jsonToStateCfg(String json); //convert json-string to state-part config
	String stateCfgToJson(); //convert state-part cfg to json-String
	void jsonToScheduleCfg(String json); //convert json-string to schedule-part config
	String scheduleCfgToJson(); //convert schedule-part cfg to json-String
	uint8_t saveStateCfg();
	uint8_t loadStateCfg();
	uint8_t saveScheduleCfg();
	uint8_t loadScheduleCfg();

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
	uint16_t _manualTargetTemp = 2000; //target temperature for manual mode MULTIPLE BY 100
	uint16_t _targetTempDelta = 500; //delta +- for both _targetTemp and manualTargetTemp MULTIPLE BY 100
	uint16_t _refresh; // thermostat update interval
	Timer _refreshTimer; // timer for thermostat update
	TempSensor *_tempSensor;
};



#endif /* INCLUDE_THERMO_H_ */
