#include <HardwareSerial.h>
#include <driver/adc.h>
#include <WiFi.h>
#include <dsmr.h>

#define P1_SERIAL_RX  18  // P1 -> UART1 which is attached to Smartmeter P1 TX pin
#define P1_DR         2   // P1 Data request
#define P2_SERIAL_TX  17  // P2 -> UART2 which is attached to HomeWizard P1 RX pin
#define P2_DR         34  // P2 Data request
#define P3_SERIAL_TX  1   // P3 -> UART0 which is attached to Alfen Loadbalancer RX pin 
#define P3_DR         35  // P3 Data request
#define P1_LED        14  // lights up when a telegram is received and stored
#define P2_LED        27  // lights up when P2 requests data
#define P3_LED        26  // lights up when P3 requests data

void disableWiFi();

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

P1Reader reader(&Serial1,P1_DR);

void setup() {
  disableWiFi(); // WiFi is not needed so disable it and draw less power
  //Serial.begin(115200);                                   //  UART0, now used for monitoring, later for Alfen Load Balancer
  //Serial.println("Smart Splitter for DSMR P1, Copyright 2022, Jacco Bezemer");
  pinMode(P2_DR,INPUT);
  pinMode(P3_DR,INPUT);
  pinMode(P1_LED,OUTPUT_OPEN_DRAIN);
  pinMode(P2_LED,OUTPUT_OPEN_DRAIN);
  pinMode(P3_LED,OUTPUT_OPEN_DRAIN);
  digitalWrite(P1_LED,HIGH);
  digitalWrite(P2_LED,HIGH);
  digitalWrite(P3_LED,HIGH);
  Serial1.begin(115200,SERIAL_8N1,P1_SERIAL_RX,-1,false);  //  Telgram receive port, UART1 attached to DSMR P1 port receiving telegrams at maximum interval, no send pin nessesary
  Serial2.begin(115200,SERIAL_8N1,-1,P2_SERIAL_TX,false);  //  UART2 attached to HomeWizard send telegrams, on request, no receive pin nessesary
  Serial.begin(115200,SERIAL_8N1,-1,P3_SERIAL_TX,false);   //  UART0 attached to Alfen Load Balancer to send telegrams on request, no receive pin nessesary
  reader.enable(false);                                    //  Reads the telegram send by the P1 meter 
}

void loop() {
  
  reader.loop();                      // Allow the reader to check the serial buffer regularly

  if (reader.available()) {           // An complete and correct telegram has been received
    digitalWrite(P1_LED,LOW);         // turn on the telegram stored led

    // send the telegram to HomeWizard P1 meter
    if (digitalRead(P2_DR) == LOW) {
      digitalWrite(P2_LED,LOW);       // turn on the data request led for P2
      Serial2.print("/");             // this whas stripped from the stored telegram so it need's to be send ahead
      Serial2.print(reader.raw());    // sends the telegram that whas stored by the reader
      Serial2.print(reader.rawCRC()); // this whas stripped from the stored telegram so it need't to be send after
      digitalWrite(P2_LED,HIGH);      // is this long enough to see the led blink?
    }

    // send the telegram to Alfen Load Balancer
    if (digitalRead(P3_DR) == LOW) {
      digitalWrite(P3_LED,LOW);       // turn on the data request led for P3
      Serial.print("/");              // this whas stripped from the stored telegram so it need's to be send ahead
      Serial.print(reader.raw());     // sends the telegram that whas stored by the reader
      Serial.print(reader.rawCRC());  // this whas stripped from the stored telegram so it need't to be send after
      digitalWrite(P3_LED,HIGH);      // is this long enough to see the led blink?
    }

    reader.clear();                   // clears the stored telegram
    
    // decode the data so it can be presented on a display (when attached, for now print to serial monitoring)
    /*
    MyData data;
    String err;
    if (reader.parse(&data, &err)) {
      // Parse succesful, print result
      data.applyEach(Printer());
    } else {
      // Parser error, print error
      Serial.println(err);
    }
    */
    
    digitalWrite(P1_LED,HIGH);        // turn off the telegram stored led
  }  

}

void disableWiFi() {
    adc_power_off(); //deprecated, use adc_power_release(void) instead
    //adc_power_release();  //causes a reboot
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
}