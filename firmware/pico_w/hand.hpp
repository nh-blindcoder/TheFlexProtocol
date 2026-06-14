#pragma once

#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include <cstdio>

// debug printf to save cpu time
#ifdef DEBUG
#define DBG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DBG_PRINT(fmt, ...)
#endif

extern "C" {
#include "pico_servo.h"
}

class Servo {
public:
  Servo() : gpio_(0xff) {}

  void attach(uint gpio) {
    gpio_ = gpio;
    int ret = servo_enable(gpio_);
    if (ret != 0) {
      DBG_PRINT("[SERVO] Failed to enable GPIO %u (err=%d)\n", gpio_, ret);
    } else {
      DBG_PRINT("[SERVO] Enabled GPIO %u\n", gpio_);
    }
    sleep_ms(10);
    write(90); // neutral on attach
  }

  // angle: 0–180 degrees
  void write(int angle) {
    if (gpio_ == 0xff)
      return;
    if (angle < 0)
      angle = 0;
    if (angle > 180)
      angle = 180;

    servo_set_position(gpio_, angle);
    DBG_PRINT("[SERVO] GPIO %u : %d°\n", gpio_, angle);
  }

private:
  uint gpio_;
};

class Hand {
public:
  Hand();

  void OpenPalm();
  void Fist();
  void Like();
  void Point();
  void MiddleFinger();
  void Pinch();
  void Relax();
  // these could easily be changed
private:
  static constexpr uint SERVO_PINKIE = 2;
  static constexpr uint SERVO_RING = 3;
  static constexpr uint SERVO_MIDDLE = 4;
  static constexpr uint SERVO_POINT = 5;
  static constexpr uint SERVO_THUMB = 6;
  // gpios for the servos
  Servo pinkie_;
  Servo ring_;
  Servo middle_;
  Servo point_;
  Servo thumb_;

  void writePose(int pinkie, int ring, int middle, int point, int thumb);
};

inline Hand::Hand() {
  pinkie_.attach(SERVO_PINKIE);
  ring_.attach(SERVO_RING);
  middle_.attach(SERVO_MIDDLE);
  point_.attach(SERVO_POINT);
  thumb_.attach(SERVO_THUMB);
  OpenPalm();
  sleep_ms(200);
  Fist();
  sleep_ms(200);
  // initial check for all the servos
}

inline void Hand::writePose(int p, int r, int m, int po, int t) {
  DBG_PRINT("[HAND] Pose: Pinkie=%d Ring=%d Middle=%d Point=%d Thumb=%d\n", p,
            r, m, po, t);
  pinkie_.write(p);
  ring_.write(r);
  middle_.write(m);
  point_.write(po);
  thumb_.write(t);
}

inline void Hand::OpenPalm() {
  DBG_PRINT("[HAND] : OpenPalm\n");
  writePose(180, 180, 180, 180, 180);
}
inline void Hand::Fist() {
  DBG_PRINT("[HAND] : Fist\n");
  writePose(0, 0, 0, 0, 0);
}
inline void Hand::Like() {
  DBG_PRINT("[HAND] : Like\n");
  writePose(0, 0, 0, 0, 180);
}
inline void Hand::Point() {
  DBG_PRINT("[HAND] : Point\n");
  writePose(0, 0, 0, 180, 0);
}
inline void Hand::MiddleFinger() {
  DBG_PRINT("[HAND] : MiddleFinger\n");
  writePose(0, 0, 180, 0, 0);
}

// Pinch: index + thumb, others closed
inline void Hand::Pinch() {
  DBG_PRINT("[HAND] : Pinch\n");
  writePose(0, 0, 0, 90, 90);
}

// Relax is treated the same as open palm
inline void Hand::Relax() {
  DBG_PRINT("[HAND] : Relax\n");
  OpenPalm();
}