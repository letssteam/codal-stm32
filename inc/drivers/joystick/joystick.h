#pragma once

#include "AnalogSensor.h"
#include "Button.h"
#include "Event.h"
#include "EventModel.h"
#include "STM32Pin.h"

namespace codal {
typedef void (*JoystickUserEvent)();

enum class JoystickDirection : uint8_t { Left = 0, Top = 1, Right = 2, Bottom = 3 };
enum class ButtonEvent : uint8_t { Click = 0, LongClick = 1, Up = 2, Down = 3, Hold = 4, DoubleClick = 5 };
enum class JoystickAxis : uint8_t { Horizontal, Vertical };
class Joystick {
  public:
    /**
     * @brief Sets the deadzone of the joystick
     *
     * @param newDeadzone deadzone's new value in range [0, 100]
     *
     * @return void
     */
    void setDeadzone(uint8_t newDeadzone);

    /**
     * @brief Get the deadzone value in range [0, 100]
     *
     * @return uint8_t
     */
    uint8_t getDeadzone();

    /**
     * @brief Gets the analog value of the given axis, in range [-100, 100]
     *
     * @param axis the sensor to get value of
     *
     * @return int8_t
     */
    int8_t getAxis(JoystickAxis axis);

    /**
     * @brief checks if the joystick is oriented in the given direction
     *
     * @param direction
     * @return true
     * @return false
     */
    bool isJoystickPointingTo(const JoystickDirection direction);

    /**
     * @brief checks if the joystick's button is pressed
     *
     * @return true
     * @return false
     */
    bool isButtonPressed();

    /**
     * @brief Registers a new event that triggers when the joystick is pointed in a specified direction
     *
     * @param direction the direction that triggers the event
     * @param handler the user function to execute when the event is triggered
     */
    void registerDirectionEvent(JoystickDirection direction, JoystickUserEvent handler);

    /**
     * @brief Registers a new event that triggers when the joystick's button is pressed
     *
     * @param handler the user function to execute when the event is triggered
     */
    void registerButtonEvent(ButtonEvent btnEvent, JoystickUserEvent handler);

    /**
     * @brief Joystick object constructor
     *
     * @param horizontalAxisPin the pin connected to the horizontal axis of the joystick
     * @param verticalAxisPin the pin connected to the horizontal axis of the joystick
     * @param buttonPin the pin connected to the button of the joystick
     * @param deadzone the joystick's deadzone, which is the amountthe joystick can move before an input is recognized
     */
    Joystick(codal::STM32Pin& horizontalAxisPin, codal::STM32Pin& verticalAxisPin, codal::STM32Pin& buttonPin,
             uint8_t deadzone = 10);

    ~Joystick() {}

  private:
    codal::AnalogSensor* horizontalSensor;
    codal::AnalogSensor* verticalSensor;
    codal::Button* button;
    uint8_t deadzone;
    JoystickUserEvent directionUserEvents[4];
    JoystickUserEvent buttonUserEvents[6];

    /**
     * @brief The function events use to execute the right event function of the right event
     *
     * @param event The event to execute
     */
    void onEvent(codal::Event event);

    /**
     * @brief Utility function used to factorize the joystick's button registering method
     *
     * @param enumEvent the type of event that will be registered
     * @param listenValue the value parameter of the listen function
     * @param handler the user function that will be executed when the event is triggered
     */
    void listenToButtonEvent(ButtonEvent enumEvent, uint8_t listenValue, JoystickUserEvent handler);

    /** @brief Utility function used to factorize the joystick's axis registering method
     *
     * @param direction the type of event that will be registered
     * @param listenValue the value parameter of the listen function
     * @param handler the user function that will be executed when the event is triggered
     */
    void listenToAxisEvent(JoystickDirection direction, uint8_t listenValue, JoystickUserEvent handler);

    /**
     * @brief Sets the Thresholds of the AnalogSensors
     *
     */
    void setThresholds();
};
}  // namespace codal