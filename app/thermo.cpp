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
	Serial.print("DateTime: "); Serial.println(now.toFullDateTimeString());
	SchedUnit daySchedule[maxProg] = _schedule[now.DayofWeek];
	Serial.print("dayOfWeek: "); Serial.println(now.DayofWeek);
	float targetTemp = 0;
	uint16_t nowMinutes = now.Hour * 60 + now.Minute;

	for (uint8_t i = 0; i < maxProg; i++)
	{
		Serial.print("I: "); Serial.println(i);
		uint8_t nextIdx = i < (maxProg - 1) ? i + 1 : 0;
		Serial.print("nextIdx: "); Serial.println(nextIdx);
		Serial.print("nowMinutes: "); Serial.println(nowMinutes);
		Serial.print("daySchedule[i].minutes: "); Serial.println(daySchedule[i].minutes);
		Serial.print("daySchedule[nextIdx].minutes: "); Serial.println(daySchedule[nextIdx].minutes);

		if ( (nowMinutes >= daySchedule[i].minutes) && (nowMinutes <= daySchedule[nextIdx].minutes) )
		{
			Serial.print("Idx: "); Serial.println(i);
			targetTemp = daySchedule[i].targetTemp;
			Serial.print("selected targetTemp: "); Serial.println(targetTemp);
			break;
		}
	}
	Serial.print("targetTemp: "); Serial.println(targetTemp);
	if (currTemp >= targetTemp + _targetTempDelta)
		_state = false;
	if (currTemp <= targetTemp - _targetTempDelta)
		_state = true;
	Serial.printf("State: %s\n", _state ? "true" : "false");
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

