#include "joystick.h"

#include "AnalogSensor.h"
#include "Button.h"
#include "Event.h"
#include "EventModel.h"
#include "STM32Pin.h"

using namespace codal;

Joystick::Joystick(STM32Pin& horizontalAxisPin, STM32Pin& verticalAxisPin, STM32Pin& buttonPin, uint8_t deadzone)
    : directionUserEvents{{nullptr}}, buttonUserEvents{{nullptr}}
{
    horizontalSensor = new AnalogSensor(horizontalAxisPin, 6666);
    verticalSensor   = new AnalogSensor(verticalAxisPin, 9999);
    button           = new Button(buttonPin, 7777);
    setDeadzone(deadzone);
}

void Joystick::setDeadzone(uint8_t newDeadzone)
{
    if (newDeadzone > 100) {
        newDeadzone = 100;
    }
    deadzone = round((float)newDeadzone / 100 * 512);
    setThresholds();
    return;
}

uint8_t Joystick::getDeadzone()
{
    return round((float)deadzone / 512 * 100);
}

int8_t Joystick::getAxis(JoystickAxis axis)
{
    uint16_t axisValue;
    if (axis == JoystickAxis::Horizontal)
        axisValue = horizontalSensor->readValue();
    else if (axis == JoystickAxis::Vertical)
        axisValue = verticalSensor->readValue();
    else
        return 0;

    if (axisValue > 512 + deadzone) {
        // mapping of axis value between 0 and JoystickAxisRange
        axisValue = mapAxis(axisValue, 512 + deadzone, 1023, 0, JoystickAxisRange) + 1;
        return axisValue;
    }
    else if (axisValue < 512 - deadzone) {
        // mapping of axis value between -JoystickAxisRange and 0
        axisValue = mapAxis(axisValue, 0, 512 - deadzone, -JoystickAxisRange, 0);
        return axisValue;
    }
    else {
        return 0;
    }
}

bool Joystick::isJoystickPointingTo(const JoystickDirection direction)
{
    if (direction == JoystickDirection::Left && getAxis(JoystickAxis::Horizontal) < 0)
        return true;
    else if (direction == JoystickDirection::Top && getAxis(JoystickAxis::Vertical) < 0)
        return true;
    else if (direction == JoystickDirection::Right && getAxis(JoystickAxis::Horizontal) > 0)
        return true;
    else if (direction == JoystickDirection::Bottom && getAxis(JoystickAxis::Vertical) > 0)
        return true;
    else
        return false;
}

void Joystick::registerDirectionEvent(JoystickDirection direction, JoystickUserEvent handler)
{
    switch (direction) {
        case JoystickDirection::Left:
            listenToAxisEvent(JoystickDirection::Left, ANALOG_THRESHOLD_LOW, handler);
            break;
        case JoystickDirection::Top:
            listenToAxisEvent(JoystickDirection::Top, ANALOG_THRESHOLD_LOW, handler);
            break;
        case JoystickDirection::Right:
            listenToAxisEvent(JoystickDirection::Right, ANALOG_THRESHOLD_HIGH, handler);
            break;
        case JoystickDirection::Bottom:
            listenToAxisEvent(JoystickDirection::Bottom, ANALOG_THRESHOLD_HIGH, handler);
            break;
        default:
            break;
    }
}

void Joystick::registerJoystickButtonEvent(JoystickButtonEvent btnEvent, JoystickUserEvent handler)
{
    switch (btnEvent) {
        case JoystickButtonEvent::Click:
            listenToJoystickButtonEvent(JoystickButtonEvent::Click, DEVICE_BUTTON_EVT_CLICK, handler);
            break;
        case JoystickButtonEvent::LongClick:
            listenToJoystickButtonEvent(JoystickButtonEvent::LongClick, DEVICE_BUTTON_EVT_LONG_CLICK, handler);
            break;
        case JoystickButtonEvent::Up:
            listenToJoystickButtonEvent(JoystickButtonEvent::Up, DEVICE_BUTTON_EVT_UP, handler);
            break;
        case JoystickButtonEvent::Down:
            listenToJoystickButtonEvent(JoystickButtonEvent::Down, DEVICE_BUTTON_EVT_DOWN, handler);
            break;
        case JoystickButtonEvent::Hold:
            listenToJoystickButtonEvent(JoystickButtonEvent::Hold, DEVICE_BUTTON_EVT_HOLD, handler);
            break;
        case JoystickButtonEvent::DoubleClick:
            listenToJoystickButtonEvent(JoystickButtonEvent::DoubleClick, DEVICE_BUTTON_EVT_DOUBLE_CLICK, handler);
            break;
        default:
            break;
    }
}

void Joystick::onEvent(Event event)
{
    if (event.source == horizontalSensor->id) {
        switch (event.value) {
            case ANALOG_THRESHOLD_LOW:
                directionUserEvents[static_cast<uint8_t>(JoystickDirection::Left)]();
                break;
            case ANALOG_THRESHOLD_HIGH:
                directionUserEvents[static_cast<uint8_t>(JoystickDirection::Right)]();
            default:
                break;
        }
    }
    else if (event.source == verticalSensor->id) {
        switch (event.value) {
            case ANALOG_THRESHOLD_LOW:
                directionUserEvents[static_cast<uint8_t>(JoystickDirection::Top)]();
                break;
            case ANALOG_THRESHOLD_HIGH:
                directionUserEvents[static_cast<uint8_t>(JoystickDirection::Bottom)]();
                break;
            default:
                break;
        }
    }
    else if (event.source == button->id) {
        switch (event.value) {
            case DEVICE_BUTTON_EVT_CLICK:
                buttonUserEvents[static_cast<uint8_t>(JoystickButtonEvent::Click)]();
                break;
            case DEVICE_BUTTON_EVT_LONG_CLICK:
                buttonUserEvents[static_cast<uint8_t>(JoystickButtonEvent::LongClick)]();
                break;
            case DEVICE_BUTTON_EVT_UP:
                buttonUserEvents[static_cast<uint8_t>(JoystickButtonEvent::Up)]();
                break;
            case DEVICE_BUTTON_EVT_DOWN:
                buttonUserEvents[static_cast<uint8_t>(JoystickButtonEvent::Down)]();
                break;
            case DEVICE_BUTTON_EVT_HOLD:
                buttonUserEvents[static_cast<uint8_t>(JoystickButtonEvent::Hold)]();
                break;
            case DEVICE_BUTTON_EVT_DOUBLE_CLICK:
                buttonUserEvents[static_cast<uint8_t>(JoystickButtonEvent::DoubleClick)]();
            default:
                break;
        }
    }
}

void Joystick::setThresholds()
{
    horizontalSensor->setHighThreshold(512 + deadzone);
    horizontalSensor->setLowThreshold(512 - deadzone);
    verticalSensor->setHighThreshold(512 + deadzone);
    verticalSensor->setLowThreshold(512 - deadzone);
}

void Joystick::listenToJoystickButtonEvent(JoystickButtonEvent enumEvent, uint8_t listenValue,
                                           JoystickUserEvent handler)
{
    if (buttonUserEvents[static_cast<uint8_t>(enumEvent)] == nullptr) {
        EventModel::defaultEventBus->listen(button->id, listenValue, this, &Joystick::onEvent);
    }
    buttonUserEvents[static_cast<uint8_t>(enumEvent)] = handler;
}

void Joystick::listenToAxisEvent(JoystickDirection direction, uint8_t listenValue, JoystickUserEvent handler)
{
    if (buttonUserEvents[static_cast<uint8_t>(direction)] == nullptr) {
        switch (direction) {
            case JoystickDirection::Left:
            case JoystickDirection::Right:
                EventModel::defaultEventBus->listen(horizontalSensor->id, listenValue, this, &Joystick::onEvent);
                break;
            case JoystickDirection::Bottom:
            case JoystickDirection::Top:
                EventModel::defaultEventBus->listen(verticalSensor->id, listenValue, this, &Joystick::onEvent);
                break;
            default:
                break;
        }
    }
    directionUserEvents[static_cast<uint8_t>(direction)] = handler;
}

int32_t Joystick::mapAxis(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
    if (x > in_max) return out_max;
    if (x < in_min) return out_min;

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}