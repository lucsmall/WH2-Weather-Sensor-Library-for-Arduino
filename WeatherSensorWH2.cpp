/*
  WeatherSensorWH2.cpp - Library for receiving wireless data from the WH2
  wireless temperature and humidity sensor.
  Created by Luc Small on 30 April 2012.
  Released into the public domain.

  This code contains a CRC-8 function adapted from the Arduino OneWire library:
  http://www.pjrc.com/teensy/td_libs_OneWire.html
  Thanks go to the authors of that project.
*/
#include "WProgram.h"
#include "WeatherSensorWH2.h"

WeatherSensorWH2::WeatherSensorWH2()
{
  /* constructor */
  _acquired = false;
}


void WeatherSensorWH2::accept(byte interval)
{
  byte sample;
  static byte state = 0;
  static byte packet_no, bit_no, history;

  // 1 is indicated by 500uS pulse
  if (interval >= 17 && interval <= 25) {
    sample = 1;
  // 0 is indicated by ~1500us pulse
  } else if (interval >= 55  && interval <= 70) {
    sample = 0;    
  } else {
    state = 0;
    return;
  } 

  // reset if in initial state
  if(state == 0) {
     // should history be 0, does it matter?
    history = 0xFF; 
    state = 1;
  } // fall thru to state one
  
  // acquire preamble
  if (state == 1) {
     // shift history right and store new value
     history <<= 1;
     // store a 1 if required (right shift along will store a 0)
     if (sample == 1) {
       history |= 0x01;
     }
     // check if we have a valid start of frame
     // xxxxx110
     if ((history & B00000111) == B00000110) {
       // need to clear packet, and counters
       packet_no = 0;
       // start at 1 becuase only need to acquire 7 bits for first packet byte.
       bit_no = 1;
       _packet[0] = _packet[1] = _packet[2] = _packet[3] = _packet[4] = 0;
       // we've acquired the preamble
       state = 2;
     }
    return;
  }
  // acquire packet
  if (state == 2) {

    _packet[packet_no] <<= 1;
    if (sample == 1) {
      _packet[packet_no] |= 0x01;
    }

    bit_no ++;
    if(bit_no > 7) {
      bit_no = 0;
      packet_no ++; 
    }
    
    if (packet_no > 4) {
     // got packet - flag this event
     _acquired = true;
     // start the sampling process from scratch
     state = 0;
    }
  }
}

bool WeatherSensorWH2::acquired()
{
  bool temp = _acquired;
  _acquired = false;
  return temp;
}

byte WeatherSensorWH2::calculate_crc()
{
  return _crc8(_packet, 4);
}

bool WeatherSensorWH2::valid()
{
  return (calculate_crc() == _packet[4]);
}

byte* WeatherSensorWH2::get_packet()
{
  return _packet;
}

int WeatherSensorWH2::get_sensor_id()
{
  return (_packet[0] << 8) + (_packet[1] >> 4);
}

byte WeatherSensorWH2::get_humidity()
{
  return _packet[3];
}

/* Temperature in deci-degrees. e.g. 251 = 25.1 */
int WeatherSensorWH2::get_temperature()
{
  int temperature;
  temperature = ((_packet[1] & B00000111) << 8) + _packet[2];
  // make negative
  if (_packet[1] & B00001000) {
    temperature = -temperature;
  }
  return temperature;
}

String WeatherSensorWH2::get_temperature_formatted()
{
  int temperature;
  byte whole, partial, sign;
  String s;

  temperature = ((_packet[1] & B00000111) << 8) + _packet[2];
  whole = temperature / 10;
  partial = temperature - (whole*10);
  sign = ' ';
  if (_packet[1] & B00001000) {
   sign = '-'; 
  }
  s = String(sign);
  s = s.concat(String(whole, DEC));
  s = s.concat(String('.'));
  s = s.concat(String(partial, DEC));
  return s;
}


uint8_t WeatherSensorWH2::_crc8( uint8_t *addr, uint8_t len)
{
  uint8_t crc = 0;

  // Indicated changes are from reference CRC-8 function in OneWire library
  while (len--) {
    uint8_t inbyte = *addr++;
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = (crc ^ inbyte) & 0x80; // changed from & 0x01
      crc <<= 1; // changed from right shift
      if (mix) crc ^= 0x31;// changed from 0x8C;
      inbyte <<= 1; // changed from right shift
    }
  }
  return crc;
}

