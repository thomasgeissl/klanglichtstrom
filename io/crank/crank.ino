//https://github.com/CNMAT/OSC/
//https://github.com/adafruit/Adafruit_NeoPixel
//https://github.com/thomasgeissl/Parameter

#include <Adafruit_NeoPixel.h>
//#include <OSCMessage.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
//#include <SPI.h>
#include <OSCMessage.h>

#include <Parameter.h>
#include "./defines.h"

ParameterGroup _parameters;
Parameter<int> _value;

bool lastReedValue = false;
unsigned long _lastTimestamp = 0;

Adafruit_NeoPixel strip(1, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

EthernetUDP Udp;



void sendOSCMessage() {
  Serial.println("sending osc message");
  OSCMessage msg("/kls/io/crank");
  msg.add(_value.get());

  Udp.beginPacket(OSCHOST, OSCOUTPORT);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
  Serial.println("sending osc message::done");
}

void setup() {
  Serial.begin(115200);

  pinMode(REEDPIN, INPUT);

  strip.begin();
  strip.show();
  strip.setBrightness(50);

  _value.setup("value", 0, 0, 127);
  _value.addListener([&](String name, int value) {
    sendOSCMessage();
    Serial.println(name + " changed, new value: " + String(value));

    strip.setPixelColor(0, strip.Color(map(value, _value.getMin(), _value.getMax(), 0, 255), 0, 0));
    strip.show();
  });
  _parameters.add(_value);


  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
    Serial.println(Ethernet.localIP());

  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  Udp.begin(OSCINPORT);

  Serial.println("setup - done");
}

void loop() {
  auto reedValue = digitalRead(REEDPIN);
  auto timestamp = millis();

  if (reedValue && reedValue != lastReedValue) {
    _value = _value + 1;
  }
  if (timestamp - _lastTimestamp > 2000) {
    _value = _value - 1;
    _lastTimestamp = timestamp;
  }
  lastReedValue = reedValue;
  delay(2);
}
