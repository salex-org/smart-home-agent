#include <agent.hpp>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               

UserInterface::UserInterface(MeasuringController& electricity, MeasuringController& gas, int displayAddress, int buttonPin, unsigned long screensaverTimeout, unsigned long displayRefreshInterval, unsigned long buttonStabilizerInterval) :
	buttonStabilizer(SignalStabilizer(buttonPin, LOW, buttonStabilizerInterval, std::bind(&UserInterface::buttonPressed, this))),
	display(SH1106Wire(displayAddress, SDA, SCL)),
	screensaverTimeout(screensaverTimeout),
	displayRefreshInterval(displayRefreshInterval),
	electricity(electricity), gas(gas) {
	this->currentPage = HOME_PAGE;
	this->displayOn = false;
	this->buttonPressedTime = 0UL;
	this->lastRenderingTime = 0UL;
}

void UserInterface::setup() {
	this->display.init();
	this->display.flipScreenVertically();
}

void UserInterface::showInitMessage() {
	this->display.clear();
	this->renderer.renderLoadingPage(this->display);
	this->display.display();
}

void UserInterface::showConfigMessage(const char* ssid, const char* password, const char* ip) {
	this->display.clear();
	this->renderer.renderConfigPage(this->display, ssid, password, ip);
	this->display.display();
}

void UserInterface::switchDisplayOff() {
	this->displayOn = false;
}

void UserInterface::switchDisplayOn() {
	this->displayOn = true;
}

void UserInterface::setTimezone() {
	this->timezone.setLocation(TIMEZONE);
}

void UserInterface::buttonPressed() {
	this->buttonPressedTime = millis();
	if(this->displayOn) {
		// Switch to next page
		switch(this->currentPage) {
			case HOME_PAGE:
				this->currentPage = WIFI_PAGE;
				break;
			case WIFI_PAGE:
				this->currentPage = TIME_PAGE;
				break;
			case TIME_PAGE:
				this->currentPage = ELECTRICITY_PAGE;
				break;
			case ELECTRICITY_PAGE:
				this->currentPage = GAS_PAGE;
				break;
			default:
				this->currentPage = HOME_PAGE;
				break;
		}
	} else {
		// Activate display and switch to home page
		this->displayOn = true;
		this->currentPage = HOME_PAGE;
	}
}

void UserInterface::loop() {
	unsigned long now = millis();

	// Loop the input thread
	this->buttonStabilizer.loop();

	// Switch display off, if screensaver should be activated
    this->displayOn = !(now - this->buttonPressedTime > this->screensaverTimeout);

	// Update display only when refresh interval is reached
    if(now - this->lastRenderingTime > this->displayRefreshInterval) {
		this->display.clear();
		if(this->displayOn) {
			// Update display
			switch(this->currentPage) {
				case WIFI_PAGE:
					this->renderer.renderWifiPage(this->display);
					break;
				case TIME_PAGE:
					this->renderer.renderTimePage(this->display, this->timezone);
					break;
				case ELECTRICITY_PAGE:
					this->renderer.renderElectricityPage(this->display, this->electricity);
					break;
				case GAS_PAGE:
					this->renderer.renderGasPage(this->display, this->gas);
					break;
				default:
					this->renderer.renderHomePage(this->display);
					break;
			}
		}
		this->display.display();
	}
}

