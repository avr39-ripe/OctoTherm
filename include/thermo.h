/*
 * thermo.h
 *
 *  Created on: 29 дек. 2015 г.
 *      Author: shurik
 */

#ifndef INCLUDE_THERMO_H_
#define INCLUDE_THERMO_H_
#include <SmingCore/SmingCore.h>

const uint8_t maxProg = 6;

struct SchedUnit
{
	uint16_t minutes; // scheduled interval start in minutes since 0:00
	float targetTemp; // target temperature for this interval
};
class Thermostat
{
public:
	Thermostat(String description, uint16_t update);
	float getTemperature() { return _temperature; };
	bool check();
	uint8_t getState() { return _state; };
private:
	uint16_t _update; // thermostat update interval
	Timer _thermostatTimer; // timer for thermostat update
	float _getCurrTemp(); // get current temperature from sensor
	SchedUnit _schedule[7][maxProg]; // 7 day X maxProg programs in schedule
	bool _state; // thermostat state on (true) or off (false)
	float _temperature; // current temperature of corresponding temperature sensor
	String _description; // some text description of thermostat
};



#endif /* INCLUDE_THERMO_H_ */
