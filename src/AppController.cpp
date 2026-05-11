#include "AppController.h"

AppController::AppController(Buttons &btn, RFIDHandler &rfid, AlarmSystem &alm, UI &ui, DFRobotDFPlayerMini &player)
    : _btn(btn), _rfid(rfid), _alm(alm), _ui(ui), _player(player), _lastPressedMillis(0)
{
}

// Calls handleButtonIn and handleRFIDIn. returns none.
void AppController::handleIn()
{
    handleButtonIn();
    // handleRFIDIn(); //TODO: add usage for this
}

// State machine that handles button input depending on UI state
void AppController::handleButtonIn()
{
    ButtonState s = _btn.getState();
    switch (_ui.getState())
    {
    case State::Boot:
    {
        /* code */
        break;
    }
    case State::Clock:
    {
        if (s.singlePressed() && !_alm.isRinging()) // Alarm disables button controls (for now)
        {
            if (s.btn1.pressed) // Decrement by 30 minutes
            {
                AlarmTime a = _alm.getAlarm();

                int totalMinutes = a.hour * 60 + a.minute - 30;
                if (totalMinutes < 0)
                    totalMinutes += 24 * 60;

                _alm.setAlarm((totalMinutes / 60) % 24, totalMinutes % 60, true);
            }
            if (s.btn2.pressed) // Increment by 30 minutes
            {
                AlarmTime a = _alm.getAlarm();

                int totalMinutes = a.hour * 60 + a.minute + 30;
                totalMinutes %= (24 * 60);

                _alm.setAlarm((totalMinutes / 60) % 24, totalMinutes % 60, true);
            }
            if (s.btn3.pressed) // Plays random track
            {
                if (millis() - _lastPressedMillis <= 500)
                    _player.stop();
                else
                    _player.play(random(1, Tracks::totalTrackCount + 1));
                _lastPressedMillis = millis();
            }
            if (s.btn4.pressed) // Toggles alarm
            {
                AlarmTime a = _alm.getAlarm();
                _alm.setAlarm(a.hour, a.minute, !a.enabled);
            }
            break;
        }
    }
    case State::Settings: // Settings not set up
    {
        /* code */
        break;
    }
    default:
        break;
    }
}

void AppController::handleRFIDIn()
{
    switch (_ui.getState())
    {
    case State::Boot:
    {
        /* code */
        break;
    }
    case State::Clock:
    {
        /* code */
        break;
    }
    case State::Settings: // Settings not set up
    {
        /* code */
        break;
    }
    default:
        break;
    }
}