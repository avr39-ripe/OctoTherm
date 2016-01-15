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

struct SchedUnit
{
	SchedUnit()
	{
		minutes = 0;
		targetTemp = 0;
	}
	uint16_t minutes; // scheduled interval start in minutes since 0:00
	float targetTemp; // target temperature for this interval
};

class Thermostat
{
public:
	Thermostat(TempSensor &tempSensor, String description = "Thermostat", uint16_t refresh = 4000);
	void start();
	void stop();
	void check();
//	void setSched(uint8_t wDay, uint8_t progNum, uint16_t minutes, float tergetTemp);
//	SchedUnit getSched(uint8_t wDay, uint8_t progNum);
	SchedUnit _schedule[7][maxProg]; // 7 day X maxProg programs in schedule
	float _targetTempDelta = 0.5;
	uint8_t getState() { return _state; };
	String getDescription() { return _description; };
private:
	uint16_t _refresh; // thermostat update interval
	Timer _refreshTimer; // timer for thermostat update
	TempSensor *_tempSensor;
	bool _state; // thermostat state on (true) or off (false)
	String _description; // some text description of thermostat
};



#endif /* INCLUDE_THERMO_H_ */
