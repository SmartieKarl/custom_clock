// Buttons.cpp
#include "Buttons.h"

// Constructor
Buttons::Buttons(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
    : _btn1(b1), _btn2(b2), _btn3(b3), _btn4(b4), _mtx(NULL)
{
}

// Init mutex and pins
void Buttons::begin()
{
    pinMode(_btn1, INPUT_PULLUP);
    pinMode(_btn2, INPUT_PULLUP);
    pinMode(_btn3, INPUT_PULLUP);
    pinMode(_btn4, INPUT_PULLUP);

    _mtx = xSemaphoreCreateMutex();
    if (!_mtx)
        LOG.log("Warning! Buttons mutex initialization failed.");
}
// Update state of button hardware
void Buttons::update()
{
    // Button state read

    bool b1 = digitalRead(_btn1) == LOW;
    bool b2 = digitalRead(_btn2) == LOW;
    bool b3 = digitalRead(_btn3) == LOW;
    bool b4 = digitalRead(_btn4) == LOW;

    // Mutex lock for access to _currState
    xSemaphoreTake(_mtx, portMAX_DELAY);

    _prevState = _currState;

    // Update _currState
    _currState.btn1.isDown = b1;
    _currState.btn2.isDown = b2;
    _currState.btn3.isDown = b3;
    _currState.btn4.isDown = b4;

    _currState.btn1.pressed = b1 && !_prevState.btn1.isDown;
    _currState.btn2.pressed = b2 && !_prevState.btn2.isDown;
    _currState.btn3.pressed = b3 && !_prevState.btn3.isDown;
    _currState.btn4.pressed = b4 && !_prevState.btn4.isDown;

    _currState.btn1.released = !b1 && _prevState.btn1.isDown;
    _currState.btn2.released = !b2 && _prevState.btn2.isDown;
    _currState.btn3.released = !b3 && _prevState.btn3.isDown;
    _currState.btn4.released = !b4 && _prevState.btn4.isDown;
    xSemaphoreGive(_mtx);
}

// Returns cached button state
ButtonState Buttons::getState() const
{
    xSemaphoreTake(_mtx, portMAX_DELAY);
    ButtonState bs = _currState;
    xSemaphoreGive(_mtx);
    return bs;
}