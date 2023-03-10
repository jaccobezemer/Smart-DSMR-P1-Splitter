/**
 * Smart DSMR P1 Splitter.
 *
 * This software is licensed under The GNU GPLv3 License.
 *
 * Copyright (c) 2022-2023 Jacco Bezemer <jacco.bezemer@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <HardwareSerial.h>
#include <driver/adc.h>
#include <WiFi.h>
#include <dsmr.h>

#define P1_SERIAL_RX  18  // P1 -> UART1 which is attached to Smartmeter P1 TX pin
#define P1_DR         2   // P1 Data request
#define P2_SERIAL_TX  17  // P2 -> UART2 which, in my case, is attached to HomeWizard P1 RX pin
#define P2_DR         34  // P2 Data request
#define P3_SERIAL_TX  1   // P3 -> UART0 which, in my case, is attached to Alfen Loadbalancer RX pin 
#define P3_DR         35  // P3 Data request
#define P1_LED        14  // lights up when a telegram is received and stored
#define P2_LED        27  // lights up when P2 requests data
#define P3_LED        26  // lights up when P3 requests data

// LED stuf
const int dutyCycle = 950;  // dim the LEDs with this duty cycle
const int PWMFreq = 5000;   // 5 KHz
const int PWMResolution = 10;
const int MAX_DUTY_CYCLE = (int)(pow(2, PWMResolution) - 1);
const int pwmP1led = 0;
const int pwmP2led = 1;
const int pwmP3led = 2;
const int blinkTime = 100;            // must be less than telegram interval!
unsigned long time_now = 0;

#define LED_FOLLOW_DR               // uncomment this if you want the Data Request Led's follow the Data Request signal instead of the telegram transmit 
//#define USE_MONITOR                 // uncomment this if you want to use one of the UARTs to monitor during development
//#define PARSE_TELEGRAM              // uncomment this to enable the parser (display P1 values on a display or for debugging to serial)

void disableWiFi();

#ifdef PARSE_TELEGRAM
/**
 * Define the data we're interested in, as well as the datastructure to hold the parsed data.
 * Each template argument below results in a field of the same name.
**/

using MyData = ParsedData<
  /* FixedValue */ power_delivered,
  /* FixedValue */ power_returned
>;

struct Printer {
  template<typename Item>
  void apply(Item &i) {
    if (i.present()) {
      Serial.print(Item::get_name());
      Serial.print(F(": "));
      Serial.print(i.val());
      Serial.print(Item::unit());
      Serial.println();
    }
  }
};
#endif

P1Reader reader(&Serial1,P1_DR);

void setup() {
  
  disableWiFi();                        // WiFi is not needed so disable it and draw less power (possibly, maybe this is not needed)
#ifdef USE_MONITOR
  Serial.begin(115200);                 //  UART0, now used for monitoring, later for Alfen Load Balancer
  Serial.println("Smart Splitter for DSMR P1, Copyright 2022-2023, Jacco Bezemer");
#endif
  pinMode(P2_DR,INPUT);
  pinMode(P3_DR,INPUT);

  ledcSetup(pwmP1led, PWMFreq, PWMResolution);
  ledcSetup(pwmP2led, PWMFreq, PWMResolution);
  ledcSetup(pwmP3led, PWMFreq, PWMResolution);
  // Attach the LED PWM Channel to the GPIO Pin
  ledcAttachPin(P1_LED, pwmP1led);
  ledcAttachPin(P2_LED, pwmP2led);
  ledcAttachPin(P3_LED, pwmP3led);
  // Turn LEDs on at max brightness
  ledcWrite(pwmP1led,0);
  ledcWrite(pwmP2led,0);
  ledcWrite(pwmP3led,0);
  delay(2000);
  // turn the LEDs off
  ledcWrite(pwmP1led,MAX_DUTY_CYCLE);
  ledcWrite(pwmP2led,MAX_DUTY_CYCLE);
  ledcWrite(pwmP3led,MAX_DUTY_CYCLE);

  Serial1.begin(115200,SERIAL_8N1,P1_SERIAL_RX,-1,false);  //  Telgram receive port, UART1 attached to DSMR P1 port receiving telegrams at maximum interval, no send pin nessesary
  Serial2.begin(115200,SERIAL_8N1,-1,P2_SERIAL_TX,false);  //  UART2 attached to HomeWizard send telegrams, on request, no receive pin nessesary
#ifndef USE_MONITOR 
  Serial.begin(115200,SERIAL_8N1,-1,P3_SERIAL_TX,false);   //  UART0 attached to Alfen Load Balancer to send telegrams on request, no receive pin nessesary
#endif  
  reader.enable(false);                                    //  Reads the telegram send by the P1 meter

}

void loop() {

  time_now = millis();

#ifdef LED_FOLLOW_DR
  if (digitalRead(P2_DR) == LOW) ledcWrite(pwmP2led,dutyCycle);
  else ledcWrite(pwmP2led,MAX_DUTY_CYCLE);

  if (digitalRead(P3_DR) == LOW) ledcWrite(pwmP3led,dutyCycle);
  else ledcWrite(pwmP3led,MAX_DUTY_CYCLE); 
#endif

  reader.loop();                          // Allow the reader to check the serial buffer regularly

  if (reader.available()) {               // A complete and correct telegram has been received
    ledcWrite(pwmP1led,dutyCycle);        // turn on the telegram stored led

    // send the telegram to P2 (HomeWizard P1 meter)
    if (digitalRead(P2_DR) == LOW) {
    #ifndef LED_FOLLOW_DR
      ledcWrite(pwmP2led,dutyCycle);      // turn on the data request led for P2
    #endif  
      Serial2.print("/");                 // this whas stripped from the stored telegram so it has to be send ahead
      Serial2.print(reader.raw());        // sends the telegram that whas stored by the reader
      Serial2.print(reader.rawCRC());     // this whas stripped from the stored telegram so it has to be sent afterwards
    #ifndef LED_FOLLOW_DR  
      ledcWrite(pwmP2led,MAX_DUTY_CYCLE);
    #endif  
    }

    // send the telegram to P3 (Alfen Load Balancer)
    if (digitalRead(P3_DR) == LOW) {
    //if (counter == 10) {
    #ifndef LED_FOLLOW_DR  
      ledcWrite(pwmP3led,dutyCycle);      // turn on the data request led for P3
    #endif
      Serial.print("/");                  // this whas stripped from the stored telegram so it has to be send ahead
      Serial.print(reader.raw());         // sends the telegram that whas stored by the reader
      Serial.print(reader.rawCRC());      // this whas stripped from the stored telegram so it has to be sent afterwards
    #ifndef LED_FOLLOW_DR  
      ledcWrite(pwmP3led,MAX_DUTY_CYCLE);
    #endif  
    }

#ifndef PARSE_TELEGRAM
    reader.clear();                       // clears the stored telegram if we don't parse
#else
    // decode the data so it can be presented on a display (when attached, for now print to serial monitoring)
    MyData data;
    String err;
    if (reader.parse(&data, &err)) {
      // Parse succesful, print result
      data.applyEach(Printer());
    } else {
      // Parser error, print error
      Serial.println(err);
    }
#endif

    if (millis() < (time_now + blinkTime)) { 
      while(millis() < time_now + blinkTime){
          //wait approx. [period] ms
      }
    }
    ledcWrite(pwmP1led,MAX_DUTY_CYCLE);   // turn off the telegram stored led so we get notified of a new telegram
  }

}

void disableWiFi() {
    WiFi.mode(WIFI_OFF); // Switch the WiFi radio off
    adc_power_off(); //deprecated, use adc_power_release(void) instead    
    //adc_power_release();  //causes a reboot, E (10) ADC: adc_power_release called, but s_adc_power_on_cnt == 0
}