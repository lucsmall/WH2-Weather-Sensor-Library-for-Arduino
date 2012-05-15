/*
  WeatherSensorWH2.h - Library for receiving wireless data from the WH2
  wireless temperature and humidity sensor.
  Created by Luc Small on 30 April 2012.
  Released into the public domain.
*/
#ifndef WeatherSensorWH2_h
#define WeatherSensorWH2_h

/* Added to provide compatibility with Arduino 1.0 and 0022 */
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class WeatherSensorWH2
{
  public:
    WeatherSensorWH2();
    void accept(byte interval);
    bool acquired();
    int get_sensor_id();
    byte* get_packet();
    byte calculate_crc();
    bool valid();
    int  get_temperature();
    String get_temperature_formatted();
    byte get_humidity();
  private:
    byte _packet[5];
    bool _acquired; 
    uint8_t _crc8( uint8_t *addr, uint8_t len);
};

#endif

