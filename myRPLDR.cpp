#include "myRPlibs.h"
#include "myRPLDR.h"
#include <Arduino.h>
#include <../MySensors/core/MySensorsCore.h>

#define L_a_	-1.6477
#define L_b_	7.873

// log10(1) = a*log10(60) + b, log10(60) => 1.7782
// log10(60) = A*log10(5k) + b, log10(5k) => 3.699

#define RP_LDR_V_REF	5.	// reference V
#define RP_LDR_VCC		5.	// Vcc
#define RP_LDR_R1		30*1000	// R1 - PullUp value

#define RP_LDR_R(_adc_)	(RP_LDR_R1/(RP_LDR_VCC/(adc *(RP_LDR_V_REF/1023.)) - 1))

inline int sensorLDRToLux(int adc) {
	//return (2500/((value*0.0048828125)-500))/10; 

	/*float a = -1.6477;	// wspolczyniki log10(lux)= a*log10(R)+b
	float b = 7.873;
	float v = 5;
	float v0 = adc *(5./1023.); // 0.0048828125;	
	float R1 = 30*1000;*/

	//float R = R1/(v/v0 - 1);
	//float R = R1/(v/v0 - 1);

	int lightLevel = pow(10,L_a_*log10(RP_LDR_R(adc))+L_b_);
	//int lightLevel = pow(10,a*log10(R)+b);
	//int lightLevel = 7.46449E7/pow(R, 1.6477);
	return lightLevel;
}

// LUX
static byte dtLux = 100;
static MyMessage luxMsg1(RP_ID_LIGHT_SENSOR, V_LEVEL);
static byte id_ldr = RP_ID_LIGHT_SENSOR;

RpLdr::RpLdr(byte pin) 
	: RpSensor() {
	_pin = pin;
	Id = id_ldr;
	SensorType = S_LIGHT_LEVEL;
	SensorData = V_LIGHT_LEVEL;
	digitalWrite(pin, HIGH);	// pull up analog pin
	id_ldr++;					// increace for next pir sensor
	Serial.println("LDR pin set");
	EEReadByte(EE_LUX_MARGIN, &_luxMargin);
}

void RpLdr::loop() {
	int sensor = analogRead(_pin);
	_luxValue = (_luxValue*dtLux + sensor*10) / (dtLux + 1);
}

void RpLdr::loop_1s_tick(){
	int adcValue = _luxValue/10;	//read values were multiplied by 10

	int vlux = sensorLDRToLux(adcValue);

	int delta = MIN(_prevLuxValue, vlux)*_luxMargin/100;

	if(((vlux < _prevLuxValue - delta) || (_prevLuxValue + delta < vlux)) || ((rp_now - _lastSend) > 60*1000UL*rp_force_time)) {
		myresend(luxMsg1.setSensor(Id).set(vlux));
		_prevLuxValue = vlux;	
		_lastSend = rp_now;
	}
}

void RpLdr::loop_first() {
	_luxValue = analogRead(_pin)*10;
}

/*void RpLdr::presentation() {
	present(_id, S_LIGHT_LEVEL);
}*/

void RpLdr::report() {
	rp_addToBuffer("luxMargin[%]: ");
	rp_addToBuffer(_luxMargin);

	rp_reportBuffer();
}

void RpLdr::receiveCommon(const MyMessage &message){
	//RpSensor::receive(message);

	const char* data = message.data;
	char c = data[0];

	//if((message.sensor == Id) && (message.type == V_VAR2)) {
		
		if (c=='M') {
			_luxMargin = atoi(&data[1]);
			saveState(EE_LUX_MARGIN, _luxMargin);
		}
		report();		
	//}
}

void RpLdr::help() {
	myresend(_msgv.set("V2:M{9}"));
}

