/*
 * switch.cpp
 *
 *  Created on: 24 февр. 2016 г.
 *      Author: shurik
 */

#include <switch.h>

// Switch

Switch::Switch(uint8_t polarity, uint16_t refresh)
{
	_polarity = polarity;
	_refresh = refresh;
}

void Switch::start()
{
	_refreshTimer.initializeMs(_refresh * 1000, TimerDelegate(&Switch::_timeout_action, this)).start(true);
}

void Switch::stop()
{
	_refreshTimer.stop();
}

void Switch::setState(uint8_t state)
{
	_state = state ? _polarity : !(_polarity);
	start(); //Restart timeout timer after setting up value
}

void Switch::_timeout_action()
{
	setState(SwitchState::ON); // If do not receive "keep-alive" state after _refresh timeout set switch state to ON
}
// SwitchGPIO

SwitchGPIO::SwitchGPIO(uint8_t gpio_pin, uint8_t polarity, uint16_t refresh)
:Switch(polarity,refresh)
{
	_gpio_pin = gpio_pin;
	pinMode(_gpio_pin, OUTPUT);
}

void SwitchGPIO::setState(uint8_t state)
{
	Switch::setState(state);
	digitalWrite(_gpio_pin, _state);
}

void SwitchGPIO::httpSetState(HttpRequest &request)
{
	if (request.getBody() == NULL)
		{
			Serial.printf("NULL bodyBuf\n");
			return;
		}
		else
		{
			StaticJsonBuffer<switchJsonBufSize> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(request.getBody());
			root.prettyPrintTo(Serial); //Uncomment it for debuging

			if (root["state"].success()) // Settings
			{
				setState((uint8_t)root["state"]);
			}
		}
}

// SwitchHttp

SwitchHttp::SwitchHttp(String url, uint8_t polarity, uint16_t refresh)
:Switch(polarity,refresh)
{
	_url = url;
	_connectionStatus = SwitchConnectionStatus::DISCONNECTED;
}

void SwitchHttp::setState(uint8_t state)
{
	Switch::setState(state);
	if (_httpClient.isProcessing())
	{
			Serial.printf("IS PROCESSING\n");
			return; // We need to wait while request processing was completed
	}
	else
	{
		String postBody = "{state: \"";
		postBody += (String)state;
		postBody += "\"}";
		Serial.printf("postBody: %s\n", postBody.c_str());
		_httpClient.setRequestContentType("Content-Type: application/json; charset=utf-8");
		_httpClient.setPostBody(postBody);
		_httpClient.downloadString(_url, HttpClientCompletedDelegate(&SwitchHttp::_httpGetResponse, this));
	}
}

void SwitchHttp::_httpGetResponse(HttpClient& client, bool successful)
{
	if (successful)
	{
		_connectionStatus = SwitchConnectionStatus::CONNECTED;
		Serial.printf("POST OK\n");
	}
	else
	{
		_connectionStatus = SwitchConnectionStatus::DISCONNECTED;
		Serial.printf("POST BAD!\n");
	}
}

void SwitchHttp::_timeout_action()
{
	setState(_state); // If do not receive "keep-alive" state after _refresh timeout set switch state to ON
}

void SwitchHttp::setUrl(String url)
{
	_url = url;
}
