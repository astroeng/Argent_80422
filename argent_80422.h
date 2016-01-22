/* Derek Schacht
 *  2016 01 07
 *  License : Give me credit where it is due.
 *  Disclamer : I try and site code that I find on the internet but I am not perfect. If you find
 *              something that should be sited let me know and I will update my code.
 *  Warranty : Absolutly None
 *
 *  This header also applies to all previous commits. But, I reserve the right to modify this in the future.
 */

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
  void resetRainFall();

private:
  volatile unsigned long windSpeedEpoch;
  volatile unsigned long windSpeedTally;
  volatile unsigned char windSpeedCount;

  volatile unsigned int rainFallCount;

  unsigned char _direction_pin;
  unsigned char _anemometer_pin;
  unsigned char _bucket_pin;
};

#endif
