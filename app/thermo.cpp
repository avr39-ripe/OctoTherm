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
	_manual = false;
	_active = true;
}

void Thermostat::check()
{
	float currTemp = _tempSensor->getTemp();
	float targetTemp = 0;

	if (!_active)
	{
		Serial.println("active = false");
		_state = false;
		return;
	}
	if (_manual)
	{
		targetTemp = float(_manualTargetTemp / 100.0);
	}
	else
	{
		DateTime now = SystemClock.now(eTZ_Local);
		Serial.print("DateTime: "); Serial.println(now.toFullDateTimeString());
		SchedUnit daySchedule[maxProg] = _schedule[now.DayofWeek];
		Serial.print("dayOfWeek: "); Serial.println(now.DayofWeek);
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
				targetTemp = (float)daySchedule[i].targetTemp / 100.0; //in-place convert to float
				Serial.print("AND selected targetTemp: "); Serial.println(targetTemp);
				break;
			}
			if ( ((dayTransit) && ((nowMinutes >= daySchedule[i].start) || (nowMinutes <= daySchedule[nextIdx].start))) )
			{
				Serial.print("OR Idx: "); Serial.println(i);
				targetTemp = (float)daySchedule[i].targetTemp / 100.0; //in-place convert to float
				Serial.print("OR selected targetTemp: "); Serial.println(targetTemp);
				break;
			}
		}
	}
	Serial.print("targetTemp: "); Serial.println(targetTemp);
	Serial.print("targetTempDelta: "); Serial.println((float)(_targetTempDelta / 100.0));
	if (currTemp >= targetTemp + (float)(_targetTempDelta / 100.0))
		_state = false;
	if (currTemp <= targetTemp - (float)(_targetTempDelta / 100.0))
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

uint8_t Thermostat::loadStateCfg()
{
	StaticJsonBuffer<stateJsonBufSize> jsonBuffer;

	if (fileExist(".state" + _name))
	{
		int size = fileGetSize(".state" + _name);
		char* jsonString = new char[size + 1];
		fileGetContent(".state" + _name, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		_name = String((const char*)root["name"]);
		_active = root["active"];
		_manual = root["manual"];
		_manualTargetTemp = root["manualTargetTemp"];
		_targetTempDelta = root["targetTempDelta"];

		Serial.printf("Name: %s, Active: %d, Manual: %d, ManualTT: %d, TTDelta: %d\n",_name.c_str(),_active,_manual,_manualTargetTemp, _targetTempDelta);
		delete[] jsonString;
		return 0;
	}

}
void Thermostat::onStateCfg(HttpRequest &request, HttpResponse &response)
{
	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if (request.getBody() == NULL)
		{
			debugf("NULL bodyBuf");
			return;
		}
		else
		{
			StaticJsonBuffer<stateJsonBufSize> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			root.prettyPrintTo(Serial); //Uncomment it for debuging

			if (root["active"].success()) // Settings
			{
				_active = root["active"];
				saveStateCfg();
				return;
			}
			if (root["manual"].success()) // Settings
			{
				_manual = root["manual"];
				saveStateCfg();
				return;
			}
			if (root["manualTargetTemp"].success()) // Settings
			{
				_manualTargetTemp = ((float)(root["manualTargetTemp"]) * 100);
				saveStateCfg();
				return;
			}
			if (root["targetTempDelta"].success()) // Settings
			{
				_targetTempDelta = ((float)(root["targetTempDelta"]) * 100);
				saveStateCfg();
				return;
			}
		}
	}
	else
	{
		JsonObjectStream* stream = new JsonObjectStream();
		JsonObject& json = stream->getRoot();

		json["name"] = _name;
		json["active"] = _active;
		json["state"] = _state;
		json["temperature"] = _tempSensor->getTemp();
		json["manual"] = _manual;
		json["manualTargetTemp"] = _manualTargetTemp;
		json["targetTempDelta"] = _targetTempDelta;

		response.setHeader("Access-Control-Allow-Origin", "*");
		response.sendJsonObject(stream);
	}
}

uint8_t Thermostat::saveStateCfg()
{
	StaticJsonBuffer<stateJsonBufSize> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["name"] = _name.c_str();
	root["active"] = _active;
	root["manual"] = _manual;
	root["manualTargetTemp"] = _manualTargetTemp;
	root["targetTempDelta"] = _targetTempDelta;

//	root.prettyPrintTo(Serial);

	char buf[stateFileBufSize];
	root.printTo(buf, sizeof(buf));
	fileSetContent(".state" + _name, buf);

	return 0;
}

uint8_t Thermostat::loadScheduleCfg()
{
	StaticJsonBuffer<scheduleJsonBufSize> jsonBuffer;

	if (fileExist(".sched" + _name))
	{
		int size = fileGetSize(".sched" + _name);
		char* jsonString = new char[size + 1];
		fileGetContent(".sched" + _name, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);

		for (uint8_t day = 0; day < 7; day++)
			{
			Serial.printf("%d: ", day);
				for (uint8_t prog = 0; prog < maxProg; prog++)
				{
					_schedule[day][prog].start = root[(String)day][prog]["s"];
					_schedule[day][prog].targetTemp = root[(String)day][prog]["tt"];
					Serial.printf("{s: %d,tt: %d}", _schedule[day][prog].start, _schedule[day][prog].targetTemp);
				}
				Serial.println();
			}
	}
	return 0;
}

void Thermostat::sendScheduleCfg(HttpRequest &request, HttpResponse &response)
{
//	StaticJsonBuffer<scheduleJsonBufSize> jsonBuffer;
//	JsonObjectStream* stream = new JsonObjectStream();
//	JsonObject& json = stream->getRoot();
//
//	for (uint8_t day = 0; day < 7; day++)
//	{
//		JsonArray& jsonDay = json.createNestedArray((String)day);
//		for (uint8_t prog = 0; prog < maxProg; prog++)
//		{
//			JsonObject& jsonProg = jsonBuffer.createObject();
//			jsonProg["s"] = _schedule[day][prog].start;
//			jsonProg["tt"] = _schedule[day][prog].targetTemp;
//			jsonDay.add(jsonProg);
//		}
//	}
	StaticJsonBuffer<scheduleJsonBufSize> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		for (uint8_t day = 0; day < 7; day++)
		{
			JsonArray& jsonDay = root.createNestedArray((String)day);
			for (uint8_t prog = 0; prog < maxProg; prog++)
			{
				JsonObject& jsonProg = jsonBuffer.createObject();
				jsonProg["s"] = _schedule[day][prog].start;
				jsonProg["tt"] = _schedule[day][prog].targetTemp;
				jsonDay.add(jsonProg);
			}
		}
	char buf[scheduleFileBufSize];
	root.printTo(buf, sizeof(buf));

	response.setHeader("Access-Control-Allow-Origin", "*");
	response.setContentType(ContentType::JSON);
	response.sendString(buf);
}
uint8_t Thermostat::saveScheduleCfg()
{
	StaticJsonBuffer<scheduleJsonBufSize> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	for (uint8_t day = 0; day < 7; day++)
	{
		JsonArray& jsonDay = root.createNestedArray((String)day);
		for (uint8_t prog = 0; prog < maxProg; prog++)
		{
			JsonObject& jsonProg = jsonBuffer.createObject();
			jsonProg["s"] = _schedule[day][prog].start;
			jsonProg["tt"] = _schedule[day][prog].targetTemp;
			jsonDay.add(jsonProg);
		}
	}
	root.prettyPrintTo(Serial);

	char buf[scheduleFileBufSize];
	root.printTo(buf, sizeof(buf));
	fileSetContent(".sched" + _name, buf);
}
