/*
  TestHarness.pde - Tests the basic functions of the WH2 sensor library
  Decodes received packets and writes a summary of each packet to the Arduino's
  serial port
  Created by Luc Small on 30 April 2012.
  Released into the public domain.
*/

#include <WeatherSensorWH2.h>

#define RF_IN 2
#define LED_PACKET A2
#define LED_ACTIVITY  A1

volatile byte got_interval = 0;
volatile byte interval = 0;

volatile unsigned long old = 0, packet_count = 0; 
volatile unsigned long spacing, now, average_interval;

WeatherSensorWH2 weather;

ISR(TIMER1_COMPA_vect)
{
  static byte count = 0;
  static byte was_hi = 0;
  
  
  if (digitalRead(RF_IN) == HIGH) {
    digitalWrite(LED_ACTIVITY, HIGH);
    count++;
    was_hi = 1; 
  } else {
    digitalWrite(LED_ACTIVITY, LOW);
    if (was_hi) {
      was_hi = 0;
      interval = count;
      got_interval = 1;
      count = 0;
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Weather Sensor WH2 Test Harness");
  
  pinMode(LED_PACKET, OUTPUT);
  pinMode(LED_ACTIVITY, OUTPUT);
  pinMode(RF_IN, INPUT);
  
  TCCR1A = 0x00;
  TCCR1B = 0x09;
  TCCR1C = 0x00;
  OCR1A = 399;
  TIMSK1 = 0x02;
  
  // enable interrupts
  sei();
}

void loop() {
  byte i;
  byte *packet;
  
  if (got_interval) {
    weather.accept(interval);  
    if (weather.acquired()) {
      now = millis();
      spacing = now - old;
      old = now;
      packet_count++;
      average_interval = now / packet_count;     
   
      Serial.print("Spacing: ");
      Serial.println(spacing, DEC);
      Serial.print("Packet count: ");
      Serial.println(packet_count, DEC);
    
      Serial.print("Average spacing: ");
      Serial.println(average_interval, DEC);
     
      // flash green led to say got packet
      digitalWrite(LED_PACKET, HIGH);
      delay(100);
      digitalWrite(LED_PACKET, LOW);
       
      packet = weather.get_packet();
      for(i=0;i<5;i++) {
        Serial.print(packet[i], HEX);
        Serial.print("/");
        Serial.print(packet[i], DEC);
        Serial.print(" ");
      }  
      Serial.print("crc: ");
      Serial.print(weather.calculate_crc(), HEX);
      Serial.println((weather.valid() ? " OK" : " BAD"));
      
      Serial.print("Humidity: ");
      Serial.print(weather.get_humidity(), DEC);
      Serial.println("%");
      
      Serial.print("Temperature: ");
      Serial.print(weather.get_temperature_formatted());
      Serial.print(" C  [");
      Serial.print(weather.get_temperature(), DEC);
      Serial.println("]");
   }
   
   got_interval = 0; 
  }
  
}









