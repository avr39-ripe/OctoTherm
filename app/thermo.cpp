/*
 * thermo.cpp
 *
 *  Created on: 12 янв. 2016 г.
 *      Author: shurik
 */

#include <octotherm.h>

Thermostat::start()
{
	_refreshTimer.initializeMs(_refresh, TimerDelegate(&TempSensor::_temp_start, this)).start(true);
}
