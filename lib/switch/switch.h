/*
 * switch.h
 *
 *  Created on: 24 февр. 2016 г.
 *      Author: shurik
 */

#ifndef LIB_SWITCH_SWITCH_H_
#define LIB_SWITCH_SWITCH_H_
#include <SmingCore/SmingCore.h>

namespace SwitchState
{
	const uint8_t ON = 1;
	const uint8_t OFF = 0;
}

namespace SwitchConnectionStatus
{
	const uint8_t CONNECTED = 0;
	const uint8_t DISCONNECTED = 1;
}
const uint16_t switchJsonBufSize = JSON_OBJECT_SIZE(10);

class Switch
{
public:
	Switch(uint8_t polarity = 1, uint16_t refresh = 30);
	void start();
	void stop();
	virtual ~Switch() {};
	virtual void setState(uint8_t state);
private:
	virtual void _timeout_action();
	uint8_t _polarity = 1; // ON state polarity this value write to port to NO it
	uint16_t _refresh; // IN SECONDS!!!
	Timer _refreshTimer;
protected:
	uint8_t _state = 0; // Current switch state, initially OFF for default polarity
};

class SwitchGPIO : public Switch
{
public:
	SwitchGPIO(uint8_t gpio_pin, uint8_t polarity = 1, uint16_t refresh = 30);
	virtual ~SwitchGPIO() {};
	void httpSetState(HttpRequest &request);
	virtual void setState(uint8_t state);
private:
	uint8_t _gpio_pin; // GPIO pin number associated with this switch
};

class SwitchHttp : public Switch
{
public:
	SwitchHttp(String url, uint8_t polarity = 1, uint16_t refresh = 30);
	virtual ~SwitchHttp() {};
	virtual void setState(uint8_t state);
	void setUrl(String url);
private:
	virtual void _timeout_action();
	void _httpGetResponse(HttpClient& client, bool successful);
	HttpClient _httpClient;
	String _url;
	uint8_t _connectionStatus; // Switch connection status
};
#endif /* LIB_SWITCH_SWITCH_H_ */
