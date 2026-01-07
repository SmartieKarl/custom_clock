#include "button.h"
#include "config.h"
#include "domain.h"
#include "display.h"
#include <Arduino.h>

void initializeButtons()
{
    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);
    pinMode(BUTTON_3_PIN, INPUT_PULLUP);
    pinMode(BUTTON_4_PIN, INPUT_PULLUP);
}

//returns which buttons are actively being pressed
ButtonStates getButtonStates()
{
return {digitalRead(BUTTON_1_PIN) == LOW,
        digitalRead(BUTTON_2_PIN) == LOW,
        digitalRead(BUTTON_3_PIN) == LOW,
        digitalRead(BUTTON_4_PIN) == LOW};
}