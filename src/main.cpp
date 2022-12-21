#include <HardwareSerial.h>
#include <driver/adc.h>
#include <WiFi.h>
#include <dsmr.h>

#define P1_SERIAL_RX  18  // P1 -> UART1 which is attached to Smartmeter P1 TX pin
#define P1_DR         2
#define P2_SERIAL_TX  17  // P2 -> UART2 which is attached to HomeWizard P1 RX pin
#define P2_DR         34
#define P3_SERIAL_TX  1   // P3 -> UART0 which is attached to Alfen Loadbalancer RX pin 
#define P3_DR         35
#define P1_LED        14  // lights up when a telegram is received and stored
#define P2_LED        27  // lights up when a P2 requests data
#define P3_LED        26  // lights up when a P3 requests data

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
//String telegram;

void setup() {
  disableWiFi(); // we don't need WiFi so disable it to save power so this device can be powered by the DSMR meter (if other phantom powered peripherals are not too power demanding. Be aware, all must be under 250mA)
  //Serial.begin(115200);                                   //  UART0, now used for monitoring, later for Alfen Load Balancer
  //Serial.println("DSMR P1 Smart Splitter, Copyright 2022, Jacco Bezemer");
  pinMode(P2_DR,INPUT); // or should this be INPUT_PULLUP
  pinMode(P3_DR,INPUT); // or should this be INPUT_PULLUP
  pinMode(P1_LED,OUTPUT_OPEN_DRAIN);
  pinMode(P2_LED,OUTPUT_OPEN_DRAIN);
  pinMode(P3_LED,OUTPUT_OPEN_DRAIN);
  digitalWrite(P1_LED,HIGH);
  digitalWrite(P2_LED,HIGH);
  digitalWrite(P3_LED,HIGH);
  Serial1.begin(115200,SERIAL_8N1,P1_SERIAL_RX,-1,false);  //  Telgram receive port, UART1 attached to DSMR smartmeter receiving telegrams, no send pin nessesary
  Serial2.begin(115200,SERIAL_8N1,-1,P2_SERIAL_TX,false);  //  UART2 attached to HomeWizard sending telegrams, no receive pin nessesary
  Serial.begin(115200,SERIAL_8N1,-1,P3_SERIAL_TX,false);   //  UART0 attached to Alfen Load Balancer sending telegrams, no receive pin nessesary
  reader.enable(false);                                   //  Reads the telegram send by the P1 meter 
}

void loop() {
  // Allow the reader to check the serial buffer regularly
  reader.loop();

  if (reader.available()) {
    digitalWrite(P1_LED,LOW);

    // send the telegram to HomeWizard P1 meter
    if (digitalRead(P2_DR) == LOW) {
      digitalWrite(P2_LED,LOW);
      Serial2.print("/"); // -> is stripped from the raw message
      Serial2.print(reader.raw());
      Serial2.print(reader.rawCRC()); // -> is stripped from the raw message
      digitalWrite(P2_LED,HIGH);  // is this long enough to see the led blink?
    }

    // send the telegram to Alfen Load Balancer
    if (digitalRead(P3_DR) == LOW) {
      digitalWrite(P3_LED,LOW);
      Serial.print("/"); // -> is stripped from the raw message
      Serial.print(reader.raw());
      Serial.print(reader.rawCRC()); // -> is stripped from the raw message
      digitalWrite(P3_LED,HIGH);  // is this long enough to see the led blink?
    }

    reader.clear();
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
    digitalWrite(P1_LED,HIGH); // is this long enough to see the led blink? Yes, it is.
  }  

}

void disableWiFi() {
    adc_power_off(); //deprecated, use adc_power_release(void) instead
    //adc_power_release(); //causes a reboot
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
}