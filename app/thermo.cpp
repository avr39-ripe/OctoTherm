/*
 * thermo.cpp
 *
 *  Created on: 12 янв. 2016 г.
 *      Author: shurik
 */

#include <octotherm.h>

Thermostat::Thermostat( TempSensor &tempSensor, String name, uint16_t refresh)
{
	_tempSensor = &tempSensor;
	_name = name;
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
		Serial.print("daySchedule[i].minutes: "); Serial.println(daySchedule[i].start);
		Serial.print("daySchedule[nextIdx].minutes: "); Serial.println(daySchedule[nextIdx].start);

		bool dayTransit = daySchedule[i].start > daySchedule[nextIdx].start;
		Serial.print("dayTransit: "); Serial.println(dayTransit);

		if ( ((!dayTransit) && ((nowMinutes >= daySchedule[i].start) && (nowMinutes <= daySchedule[nextIdx].start))) )
		{
			Serial.print("AND Idx: "); Serial.println(i);
			targetTemp = daySchedule[i].targetTemp;
			Serial.print("AND selected targetTemp: "); Serial.println(targetTemp);
			break;
		}
		if ( ((dayTransit) && ((nowMinutes >= daySchedule[i].start) || (nowMinutes <= daySchedule[nextIdx].start))) )
		{
			Serial.print("OR Idx: "); Serial.println(i);
			targetTemp = daySchedule[i].targetTemp;
			Serial.print("OR selected targetTemp: "); Serial.println(targetTemp);
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

