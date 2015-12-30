#ifndef ARGENT_80422_H
#define ARGENT_80422_H


class Argent_80422
{
public:
	Argent_80422(unsigned char direction_pin, 
	             unsigned char anemometer_pin, 
				 unsigned char bucket_pin);
	
	void begin();
    void windSpeed_ISR();
	void rainFall_ISR();
	unsigned int getWindDirection();   /* Wind direction in tenths of a degree. */
	unsigned int getWindSpeed();       /* Wind speed in tenths of a MPH. */
	unsigned int getRainFall();        /* Rain fall ammount in Thousanths of an inch. */
	unsigned int getRainFallElapsed(); /* Rain fall accumulation duration in minutes. */
	void resetRainFall();
	
private:
	volatile unsigned long windSpeedElapsed;
	volatile unsigned long windSpeedEpoch;
	
	volatile unsigned long rainFallCount;
	volatile unsigned long rainFallEpoch;
	
	unsigned char _direction_pin;
	unsigned char _anemometer_pin;
	unsigned char _bucket_pin;
};

#endif
