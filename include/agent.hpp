#ifndef AGENT_HPP
#define AGENT_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <InfluxDbClient.h>
#include <ezTime.h>
#include <Wire.h>
#include <SH1106Wire.h>
#include <Preferences.h>
#include <esp32-config-lib.hpp>

#define VERSION "0.0.0"

#define REED_STATUS_LED 25
#define REED_CONTACT 27
#define IR_STATUS_LED 18                                                                                                                                                                                         
#define IR_SENSOR 17  
#define BUTTON_PIN 26                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
#define DISPLAY_ADDRESS 0x3c

#define TIMEZONE "de"

class UserInterface;
class AgentConfiguration;
class SignalStabilizer;
class PageRenderer;
class ConnectionManager;
class MeasuringController;

enum Page {	HOME_PAGE, WIFI_PAGE, TIME_PAGE, ELECTRICITY_PAGE, GAS_PAGE };

class ConnectionManager {
	private:
		unsigned long lastConnectionCheck;
		const unsigned long connectionCheckInterval;
		InfluxDBClient influxDBClient;
	public:
		ConnectionManager(unsigned long connectionCheckInterval = 180000UL);
		void setup(AgentConfiguration& config);
		void loop(AgentConfiguration& config);
		bool checkConnections(AgentConfiguration& config);
		void sendMeasurement(AgentConfiguration& config, double value, const std::string& field);
};

class PageRenderer {
	private:
		std::string getNameOfDay(uint8_t d);
		std::string getNameOfDaylightSavingTime(bool dst);
	public:
		PageRenderer();
		void renderHomePage(SH1106Wire& display);
		void renderWifiPage(SH1106Wire& display);
		void renderTimePage(SH1106Wire& display, Timezone& timezone);
		void renderElectricityPage(SH1106Wire& display, MeasuringController& measuringController);
		void renderGasPage(SH1106Wire& display, MeasuringController& measuringController);
		void renderLoadingPage(SH1106Wire& display);
		void renderConfigPage(SH1106Wire& display, const char* ssid, const char* password, const char* ip);
};

class AgentConfiguration {
	private:
		esp32config::Server* server;
		char wifiSSID[32];
		char wifiPawword[64];
		char influxURL[64];
		char influxToken[128];
		char influxOrg[64];
		char influxBucket[64];
		char certRootCA[2048];
		std::string createPassword(int len);
	public:
		AgentConfiguration();
		char* getWiFiSSID();
		char* getWiFiPassword();
		char* getInfluxURL();
		char* getInfluxToken();
		char* getInfluxOrg();
		char* getInfluxBucket();
		char* getCertRootCA();
		void load();
		void setupConfigMode(UserInterface& ui);
		void loopConfigMode();
};

class SignalStabilizer {
	private:
		unsigned long lastChange;
		int  lastStatus;
		bool triggered;
		unsigned long stabilizationInterval;
		int inputPin;
		int triggerStatus;
		std::function<void(void)> callback;
	public:
		SignalStabilizer(int inputPin, int triggerStatus, unsigned long stabilizationInterval, std::function<void(void)> callback);
		void loop();
};

class UserInterface {
	private:
		SH1106Wire display;
		SignalStabilizer buttonStabilizer;
		Timezone timezone;
		PageRenderer renderer;
		const unsigned long screensaverTimeout;
		const unsigned long displayRefreshInterval;
		Page currentPage;
		bool displayOn;
		unsigned long buttonPressedTime;
		unsigned long lastRenderingTime;
		void buttonPressed();
		MeasuringController& electricity;
		MeasuringController& gas;
	public:
		UserInterface(MeasuringController& electricity, MeasuringController& gas, int displayAddress, int buttonPin, unsigned long screensaverTimeout = 60000UL, unsigned long displayRefreshInterval = 100UL, unsigned long buttonStabilizerInterval = 100UL);
		void loop();
		void setup();
		void switchDisplayOff();
		void switchDisplayOn();
		void setTimezone();
		void showInitMessage();
		void showConfigMessage(const char* ssid, const char* password, const char* ip);
};

class MeasuringController {
	private:
		unsigned long lastMeasured;
		double currentConsumption;
		const double consumptionPerTrigger;
		const int inputPin;
		const int triggerStatus;
		const int outputLED;
		std::string field;
		SignalStabilizer sensorStabilizer;
		void consumptionMeasured();
		AgentConfiguration& config;
		ConnectionManager& connManager;
	public:
		MeasuringController(ConnectionManager& connManager, AgentConfiguration& config, const std::string& field, const int inputPin, const int triggerStatus, const unsigned long sensorStabilizerInterval, const int outputLED, const double consumptionPerTrigger);
		void loop();
		void setup();
		double getCurrentConsumption();
};

#endif