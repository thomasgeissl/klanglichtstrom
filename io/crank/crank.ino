//https://github.com/CNMAT/OSC/
//https://github.com/adafruit/Adafruit_NeoPixel
//https://github.com/thomasgeissl/Parameter

#include <MIDI.h>
#include <Adafruit_NeoPixel.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>


#include <Parameter.h>
#include "./defines.h"

ParameterGroup _parameters;
Parameter<bool> _active;
Parameter<int> _value;
Parameter<int> _brightness;
Parameter<int> _incrementor;
Parameter<int> _decrementor;
Parameter<int> _decrementInterval;

bool lastReedValue = false;
unsigned long _lastTimestamp = 0;

Adafruit_NeoPixel strip(PIXELSOFFSET + NUMBEROFPIXELS, NEOPIXELPIN, NEO_RGBW + NEO_KHZ800);

EthernetUDP Udp;


void sendOSCMessage() {
  OSCMessage msg("/kls/io/crank");
  msg.add(_value.get());

  Udp.beginPacket(OSCHOST, OSCOUTPORT);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}
void sendMIDIMessage() {
  usbMIDI.sendControlChange(MIDIOUTCONTROL, _value.get(), MIDIOUTCHANNEL);
}

void setLeds() {
  int maxIndex = map(_value.get(), _value.getMin(), _value.getMax(), 0, NUMBEROFPIXELS);
  auto color = strip.Color(0, 0, 0, 255);
  for (auto i = 0; i < maxIndex; i++) {
    strip.setPixelColor(PIXELSOFFSET + i, 0, 0, 0, 255);
  }
  for (auto i = maxIndex; i < NUMBEROFPIXELS; i++) {
    strip.setPixelColor(PIXELSOFFSET + i, 0, 0, 0, 0);
  }
  strip.show();
}

void routeConfig(OSCMessage &msg, int addrOffset ) {
  if (msg.isInt(0)) {
    _brightness = msg.getInt(0);
  }
  if (msg.isInt(1)) {
    _incrementor = msg.getInt(1);
  }
  if (msg.isInt(2)) {
    _decrementor = msg.getInt(2);
  }
  if (msg.isInt(3)) {
    _decrementInterval = msg.getInt(3);
  }
}
void routeActive(OSCMessage &msg, int addrOffset ) {
  if (msg.isBoolean(0)) {
    _active = msg.getBoolean(0);
  }
}

void readOsc() {
  OSCBundle bundleIN;
  int size;

  if ( (size = Udp.parsePacket()) > 0)
  {
    while (size--)
    {
      bundleIN.fill(Udp.read());
    }

    if (!bundleIN.hasError()) {
      bundleIN.route("/kls/io/crank/config", routeConfig);
    }
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(REEDPIN, INPUT);

  _active.setup("active", true);
  _value.setup("value", 0, 0, 127);
  _incrementor.setup("incrementor", 4, 1, 16);
  _decrementor.setup("decrementor", 8, 1, 16);
  _decrementInterval.setup("decrementInterval", 2000, 100, 10000);
  _brightness.setup("brightness", 100, 0, 100);

  _value.addListener([&](String name, int value) {
    Serial.println(name + " changed, new value: " + String(value));
    //    sendOSCMessage();
    sendMIDIMessage();
    setLeds();
  });

  _brightness.addListener([&](String name, int value) {
    Serial.println(name + " changed, new value: " + String(value));
    strip.setBrightness(value);
  });

  _parameters.add(_active);
  _parameters.add(_value);
  _parameters.add(_incrementor);
  _parameters.add(_decrementor);
  _parameters.add(_decrementInterval);
  _parameters.add(_brightness);

  strip.begin();
  strip.show();
  strip.setBrightness(_brightness);

  //  Serial.println("Initialize Ethernet with DHCP:");
  //  if (Ethernet.begin(mac) == 0) {
  //    Serial.println("Failed to configure Ethernet using DHCP");
  //    // Check for Ethernet hardware present
  //    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
  //      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
  //      while (true) {
  //        delay(1); // do nothing, no point running without Ethernet hardware
  //      }
  //    }
  //    if (Ethernet.linkStatus() == LinkOFF) {
  //      Serial.println("Ethernet cable is not connected.");
  //    }
  //    // try to congifure using IP address instead of DHCP:
  //    Ethernet.begin(mac, ip);
  //    Serial.println(Ethernet.localIP());
  //
  //  } else {
  //    Serial.print("  DHCP assigned IP ");
  //    Serial.println(Ethernet.localIP());
  //  }
  //
  //  Udp.begin(OSCINPORT);

  Serial.println("setup - done");
}

void loop() {
  readOsc();
  if (_active) {
    auto reedValue = digitalRead(REEDPIN);
    auto timestamp = millis();

    if (reedValue && reedValue != lastReedValue) {
      _value = _value + _incrementor;
    }
    if (timestamp - _lastTimestamp > _decrementInterval) {
      _value = _value - _decrementor;
      _lastTimestamp = timestamp;
    }
    lastReedValue = reedValue;
    delay(2);
  } else {
    delay(200);
  }
}
