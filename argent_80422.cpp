#include <argent_80422.h>
#include <Arduino.h>

/* The wind direction indicatior uses 8 resistor reed switch pairs
 * this along with a 10k ohm resistor tied to 5 volts will produce 
 * the voltages in this table. This table is in hundreths of a volt.
 */

//int voltage_map[] = {32,  // 112.5   [65]
//                     41,  // 67.5    [84]
//                     45,  // 90      [92]
//                     62,  // 157.5   [127]
//                     90,  // 135     [184]
//                     119,  // 202.5  [243]
//                     140,  // 180    [286]
//                     198,  // 22.5   [405]
//                     225,  // 45     [460]
//                     293,  // 247.5  [600]
//                     308,  // 225    [631]
//                     343,  // 337.5  [702]
//                     384,  // 0      [788]
//                     404,  // 292.5  [827]
//                     432,  // 315  //478???  [884] [979]
//                     462}; // 270    [946]

/* Adjusted to be the midpoint between two readings. */
const int voltage_map[] = {37,  // 112.5   [65]
	                       43,  // 67.5    [84]
	                       53,  // 90      [92]
	                       76,  // 157.5   [127]
						   104,  // 135     [184]
						   129,  // 202.5  [243]
						   169,  // 180    [286]
                           211,  // 22.5   [405]
						   259,  // 45     [460]
						   300,  // 247.5  [600]
						   325,  // 225    [631]
						   363,  // 337.5  [702]
						   394,  // 0      [788]
						   418,  // 292.5  [827]
						   447,  // 315  //478???  [884] [979]
						   472}; // 270    [946]
						   
const int degree_map[] = {1125,
	                       675,
						   900,
						  1575,
						  1350,
						  2025,
						  1800,
						   225,
						   450,
						  2475,
						  2250,
						  3375,
                             0,
						  2925,
						  3150,
						  2700};

#define RPMillis_to_MPH 1492
#define MILLIS_to_HOURS 1000 * 60 * 60


/* Setup for the argent_80422 anamometer, wind direction, 
 * and rain guage gizmo. This class handles all three 
 * functions.
 */

Argent_80422::Argent_80422(unsigned char direction_pin, 
	                       unsigned char anemometer_pin, 
			           	   unsigned char bucket_pin)
{
	_direction_pin  = direction_pin;
	_anemometer_pin = anemometer_pin;
	_bucket_pin     = bucket_pin;
						   	
}

void Argent_80422::begin()
{
	pinMode(_anemometer_pin, INPUT_PULLUP);
	pinMode(_bucket_pin, INPUT_PULLUP);
	pinMode(_direction_pin, INPUT);
	
	windSpeedElapsed = 1;
	windSpeedEpoch = millis();
	
	rainFallCount = 0;
	rainFallEpoch = millis();
	
}

/* This function is intended to be called when the reed switch 
 * for the anamometer sends a pulse. The best use is to call 
 * this function from an ISR that handles the other ISR duties.
 * High speed polling could be used to but pulses will be missed
 * if the code is not super simple.
 */ 

void Argent_80422::windSpeed_ISR()
{
	if (millis() >= windSpeedEpoch)
	{
		windSpeedElapsed = millis() - windSpeedEpoch;
	}
	else
	{
		windSpeedElapsed = 0xffffffff - windSpeedEpoch + millis();
	}
		
	windSpeedEpoch = millis();
}

/* This function is intended to be called when the reed switch 
 * for the rain guage sends a pulse. The best use is to call 
 * this function from an ISR that handles the other ISR duties.
 * High speed polling could be used to but pulses will be missed
 * if the code is not super simple.
 */ 

void Argent_80422::rainFall_ISR()
{
	rainFallCount++;
}

/* Returns wind direction in degrees */

unsigned int Argent_80422::getWindDirection()
{
    long value = ((long)analogRead(_direction_pin) * 500l) / 1024l;

	/* Making this a jump table vs a loop with a lookup array
	 * will cause the compiler to put this in program memory
	 * instead of dynamic memory.
	 */
	if (value < 37) return 1125;
	if (value < 43) return 675;
	if (value < 53) return 900;
	if (value < 76) return 1575;
	if (value < 104) return 1350;
	if (value < 129) return 2025;
	if (value < 169) return 1800;
	if (value < 211) return 225;
	if (value < 259) return 450;
	if (value < 300) return 2475;
	if (value < 325) return 2250;
	if (value < 363) return 3375;
	if (value < 394) return 0;
	if (value < 418) return 2925;
	if (value < 447) return 3150;
	if (value < 472) return 2700;

	return 9999;
}

/* Returns windspeed in tenths of a MPH */

unsigned int Argent_80422::getWindSpeed()
{
	return (RPMillis_to_MPH * 10) / windSpeedElapsed;
}

/* Returns the amount of rain fallen since the last reset in
 * thousanths of an inch. Resolution is 11 thousanths.
 */

unsigned int Argent_80422::getRainFall()
{
	return rainFallCount * 11;
}

/* Returns the duration of the rain fallen accumulation since
 * the last reset in hours.
 */

unsigned int Argent_80422::getRainFallElapsed()
{
	unsigned long timeValue = millis();
	
	if (timeValue > rainFallEpoch)
	{
		return (timeValue - rainFallEpoch) / MILLIS_to_HOURS;
	}
	else
	{
		return (0xffffffff - rainFallEpoch + timeValue) / MILLIS_to_HOURS;
	}
}

/* Function used to reset the rain fall accumulation.
 */

void Argent_80422::resetRainFall()
{
	rainFallCount = 0;
	rainFallEpoch = millis();
}
	