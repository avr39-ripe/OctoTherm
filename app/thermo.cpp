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
void Thermostat::start()
{
	_refreshTimer.initializeMs(_refresh, TimerDelegate(&Thermostat::check, this)).start(true);
	_tempSensor->start();
}

void Thermostat::stop()
{
	_refreshTimer.stop();
	_tempSensor->stop();
}

