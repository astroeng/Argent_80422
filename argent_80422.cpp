/* Derek Schacht
 *  2016 01 07
 *  License : Give me credit where it is due. 
 *  Disclamer : I try and site code that I find on the internet but I am not perfect. If you find 
 *              something that should be sited let me know and I will update my code.
 *  Warranty : Absolutly None
 *  
 *  This header also applies to all previous commits. But, I reserve the right to modify this in the future.
 */

#include <argent_80422.h>
#include <Arduino.h>


#define RPMillis_to_MPH 1492000l
#define MILLIS_to_HOURS 1000 * 60 * 60

long timeAge(long time_reference, long time_in_question)
{
	if (time_reference >= time_in_question)
	{
		return (time_reference - time_in_question);
	}
	else
	{
		return (0xffffffff - time_in_question + time_reference);
	}
}

/* Setup for the argent_80422 anemometer, wind direction, 
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

	windSpeedEpoch = millis();
	windSpeedTally = 0;
	windSpeedCount = 0;
	
	rainFallCount = 0;
	
}

/* This function is intended to be called when the reed switch 
 * for the anemometer sends a pulse. The best use is to call 
 * this function from an ISR that handles the other ISR duties.
 * High speed polling could be used to but pulses will be missed
 * if the code is not super simple.
 */ 

unsigned long windSpeedTally_ISR;
unsigned long windSpeedCount_ISR;

#define WINDSPEED_COUNT_LIMIT 4
#define WINDSPEED_COUNT_BOTTOM 0

void Argent_80422::windSpeed_ISR()
{
	unsigned long windSpeedElapsed;
	
	windSpeedElapsed = timeAge(micros(),windSpeedEpoch);
	
	if (windSpeedCount_ISR < WINDSPEED_COUNT_LIMIT && 
		windSpeedCount_ISR >= WINDSPEED_COUNT_BOTTOM)
	{
		/* Accumulate the interval that the anemometer takes to rotate.
		 * This combined with the number of rotations will give wind
		 * speed.
		 */
		windSpeedTally_ISR += windSpeedElapsed;
	}
	else if (windSpeedCount_ISR == WINDSPEED_COUNT_LIMIT)
	{
		/* Copy over the data and reset the internal stuff. 
		 */
		windSpeedTally = windSpeedTally_ISR;
		windSpeedCount = windSpeedCount_ISR;
		windSpeedTally_ISR = windSpeedElapsed;
		windSpeedCount_ISR = WINDSPEED_COUNT_BOTTOM;
		
	}
	else
	{
		// Something is horribly wrong.... reset the counters and pray.
		windSpeedTally_ISR = 0;
		windSpeedCount_ISR = WINDSPEED_COUNT_BOTTOM;
	}
	
	windSpeedCount_ISR++;
	
	windSpeedEpoch = micros();
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
	
    /* The wind direction indicatior uses 8 resistor reed switch pairs
     * this along with a 10k ohm resistor tied to 5 volts will produce 
     * the voltages in this table. This table is in hundreths of a volt.
     */

	/* Making this a jump table vs a loop with a lookup array
	 * will cause the compiler to put this in program memory
	 * instead of dynamic memory.
	 */

	/* Voltage (in hundreths) return Degrees (in tenths) */
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


/* Returns windspeed in hundreths of a MPH */

unsigned int Argent_80422::getWindSpeed()
{
	/* Set this to an arbitrarily large number. This
	 * will make the windSpeed basically zero if the 
	 * anemometer is not spinning.
	 */

	unsigned long averageWindSpeedElapsed = 0xffffffff;
	
	/* Check to make sure a windspeed is present and isn't very old.
	 * Basically assume a zero (0) windspeed if it is older than 30
	 * seconds since that implies that the anemometer is barely rotating.
	 * This equates to a wind speed of less than 0.05 mph.
	 */

	if ((windSpeedCount > 0) && timeAge(micros(), windSpeedEpoch) < 30000000)
	{
		averageWindSpeedElapsed = windSpeedTally / windSpeedCount;
	}

	return (RPMillis_to_MPH * 100l) / averageWindSpeedElapsed;
}

/* Returns the amount of rain fallen since the last reset in
 * thousanths of an inch. Resolution is 11 thousanths.
 */

unsigned int Argent_80422::getRainFall()
{
	return rainFallCount * 11;
}

/* Function used to reset the rain fall accumulation.
 */

void Argent_80422::resetRainFall()
{
	rainFallCount = 0;
}
	