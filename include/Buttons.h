#pragma once
#include "Log.h"
#include <Arduino.h>
#include <stdint.h>

// Buttons.h
// Handles reading the state of physical buttons

struct Button
{
    bool isDown = false;   // current level
    bool pressed = false;  // rising edge
    bool released = false; // falling edge
};

struct ButtonState
{
    Button btn1;
    Button btn2;
    Button btn3;
    Button btn4;
    bool anyDown() const { return btn1.isDown || btn2.isDown || btn3.isDown || btn4.isDown; }
    bool anyPressed() const { return btn1.pressed || btn2.pressed || btn3.pressed || btn4.pressed; }
    bool anyReleased() const { return btn1.released || btn2.released || btn3.released || btn4.released; }

    bool singlePressed() const { return btn1.pressed + btn2.pressed + btn3.pressed + btn4.pressed == 1; }
};

class Buttons
{
  public:
    Buttons(uint8_t btn1, uint8_t btn2, uint8_t btn3, uint8_t btn4);

    void begin();

    void update();
    ButtonState getState() const;

  private:
    uint8_t _btn1, _btn2, _btn3, _btn4;
    ButtonState _currState;
    ButtonState _prevState;
    mutable SemaphoreHandle_t _mtx; // Mutex safety
};