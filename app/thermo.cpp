/*
 * thermo.cpp
 *
 *  Created on: 12 янв. 2016 г.
 *      Author: shurik
 */

#include <octotherm.h>

Thermostat::Thermostat( TempSensor &tempSensor, String description, uint16_t refresh)
{
	_tempSensor = &tempSensor;
	_description = description;
	_refresh = refresh;
	_state = false;
}

void Thermostat::check()
{
	float currTemp = _tempSensor->getTemp();
	DateTime now = SystemClock.now(eTZ_Local);
	SchedUnit daySchedule[maxProg] = _schedule[now.DayofWeek];
	float targetTemp;
	uint16_t nowMinutes = now.Hour * 60 + now.Minute;

	for (uint8_t i = 0; i < maxProg; i++)
	{
		uint8_t nextIdx = i < (maxProg - 1) ? i + 1 : 0;
		if ( (nowMinutes >= daySchedule[i].minutes) && (nowMinutes <= daySchedule[nextIdx].minutes) )
		{
			targetTemp = daySchedule[i].targetTemp;
			break;
		}
	}
	if (currTemp >= targetTemp + _targetTempDelta)
		_state = false;
	if (currTemp <= targetTemp - _targetTempDelta)
		_state = true;
}

void Thermostat::start()
{
	_refreshTimer.initializeMs(_refresh, TimerDelegate(&Thermostat::check, this)).start(true);
//	_tempSensor->start();
}

void Thermostat::stop()
{
	_refreshTimer.stop();
//	_tempSensor->stop();
}

