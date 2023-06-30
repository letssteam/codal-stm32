/*
The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef CODAL_STM32_PIN_H
#define CODAL_STM32_PIN_H

#include "CodalComponent.h"
#include "CodalConfig.h"
#include "Pin.h"
#include "PinNames.h"
#include "PinNumber.h"
#include "STM32PWM.h"
#include "clock.h"

namespace codal {

class STM32Pin : public Pin {
  public:
    /**
     * Constructor.
     * Create a Pin instance, generally used to represent a pin on the edge connector.
     *
     * @param id the unique EventModel id of this component.
     *
     * @param name the PinNumber for this Pin instance.
     *
     * @param capability the capabilities this Pin instance should have.
     *                   (PIN_CAPABILITY_DIGITAL, PIN_CAPABILITY_ANALOG, PIN_CAPABILITY_AD, PIN_CAPABILITY_ALL)
     *
     * @code
     * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_ALL);
     * @endcode
     */
    STM32Pin(int id, PinNumber name, PinCapability capability);

    /**
     * Constructor.
     * Create a Pin instance, generally used to represent a pin on the edge connector.
     *
     * @param id the unique EventModel id of this component.
     *
     * @param name the PinName for this Pin instance.
     *
     * @param capability the capabilities this Pin instance should have.
     *                   (PIN_CAPABILITY_DIGITAL, PIN_CAPABILITY_ANALOG, PIN_CAPABILITY_AD, PIN_CAPABILITY_ALL)
     *
     * @code
     * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_ALL);
     * @endcode
     */
    STM32Pin(int id, PinName name, PinCapability capability) : STM32Pin(id, (PinNumber)name, capability) {}

    ~STM32Pin()
    {
        if (pwm != nullptr) delete pwm;
    }

    /**
     * Configures this IO pin as a digital output (if necessary) and sets the pin to 'value'.
     *
     * @param value 0 (LO) or 1 (HI)
     *
     * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
     *         if the given pin does not have digital capability.
     *
     * @code
     * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
     * P0.setDigitalValue(1); // P0 is now HI
     * @endcode
     */
    int setDigitalValue(int value) final override;

    /**
     * Configures this IO pin as a digital input (if necessary) and tests its current value.
     *
     *
     * @return 1 if this input is high, 0 if input is LO, or DEVICE_NOT_SUPPORTED
     *         if the given pin does not have digital capability.
     *
     * @code
     * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
     * P0.getDigitalValue(); // P0 is either 0 or 1;
     * @endcode
     */
    int getDigitalValue() final override;

    /**
     * Configures this IO pin as an analog/pwm output, and change the output value to the given level.
     *
     * @param value the level to set on the output pin, in the range 0 - 1024
     *
     * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
     *         if the given pin does not have analog capability.
     */
    int setAnalogValue(int value) final override;

    /**
     * Configures this IO pin as an analog/pwm output (if necessary) and configures the period to be 20ms,
     * with a duty cycle between 500 us and 2500 us.
     *
     * A value of 180 sets the duty cycle to be 2500us, and a value of 0 sets the duty cycle to be 500us by default.
     *
     * This range can be modified to fine tune, and also tolerate different servos.
     *
     * @param value the level to set on the output pin, in the range 0 - 180.
     *
     * @param range which gives the span of possible values the i.e. the lower and upper bounds (center +/- range/2).
     * Defaults to DEVICE_PIN_DEFAULT_SERVO_RANGE.
     *
     * @param center the center point from which to calculate the lower and upper bounds. Defaults to
     * DEVICE_PIN_DEFAULT_SERVO_CENTER
     *
     * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
     *         if the given pin does not have analog capability.
     */
    int setServoValue(int value, int range = DEVICE_PIN_DEFAULT_SERVO_RANGE,
                      int center = DEVICE_PIN_DEFAULT_SERVO_CENTER) final override;

    /**
     * Configures this IO pin as an analogue input (if necessary), and samples the Pin for its analog value.
     *
     * @return the current analogue level on the pin, in the range 0 - 1024, or
     *         DEVICE_NOT_SUPPORTED if the given pin does not have analog capability.
     *
     * @code
     * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
     * P0.getAnalogValue(); // P0 is a value in the range of 0 - 1024
     * @endcode
     */
    int getAnalogValue() final override;

    /**
     * Configures this IO pin as a "makey makey" style touch sensor (if necessary)
     * and tests its current debounced state.
     *
     * Users can also subscribe to DeviceButton events generated from this pin.
     *
     * @return 1 if pin is touched, 0 if not, or DEVICE_NOT_SUPPORTED if this pin does not support touch capability.
     *
     * @code
     * DeviceMessageBus bus;
     *
     * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_ALL);
     * if(P0.isTouched())
     * {
     *     //do something!
     * }
     *
     * // subscribe to events generated by this pin!
     * bus.listen(DEVICE_ID_IO_P0, DEVICE_BUTTON_EVT_CLICK, someFunction);
     * @endcode
     */
    int isTouched() final override;

    /**
     * Configures this IO pin as an analog/pwm output if it isn't already, configures the period to be 20ms,
     * and sets the pulse width, based on the value it is given.
     *
     * @param pulseWidth the desired pulse width in microseconds.
     *
     * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
     *         if the given pin does not have analog capability.
     */
    int setServoPulseUs(uint32_t pulseWidth) final override;

    /**
     * Configures the PWM period of the analog output to the given value.
     *
     * @param period The new period for the analog output in milliseconds.
     *
     * @return DEVICE_OK on success, or DEVICE_NOT_SUPPORTED if the
     *         given pin is not configured as an analog output.
     */
    int setAnalogPeriod(int periodMs) final override;

    /**
     * Configures the PWM period of the analog output to the given value.
     *
     * @param period The new period for the analog output in microseconds.
     *
     * @return DEVICE_OK on success, or DEVICE_NOT_SUPPORTED if the
     *         given pin is not configured as an analog output.
     */
    int setAnalogPeriodUs(uint32_t periodUs) final override;

    /**
     * Obtains the PWM period of the analog output in microseconds.
     *
     * @return the period on success, or DEVICE_NOT_SUPPORTED if the
     *         given pin is not configured as an analog output.
     */
    uint32_t getAnalogPeriodUs() final override;

    /**
     * Configures the pull of this pin.
     *
     * @param pull one of the mbed pull configurations: PullUp, PullDown, PullNone
     *
     * @return DEVICE_NOT_SUPPORTED if the current pin configuration is anything other
     *         than a digital input, otherwise DEVICE_OK.
     */
    int setPull(PullMode pull) final override;

    /**
     * Configures the events generated by this Pin instance.
     *
     * DEVICE_PIN_EVENT_ON_EDGE - Configures this pin to a digital input, and generates events whenever a rise/fall is
     * detected on this pin. (DEVICE_PIN_EVT_RISE, DEVICE_PIN_EVT_FALL) DEVICE_PIN_EVENT_ON_PULSE - Configures this pin
     * to a digital input, and generates events where the timestamp is the duration that this pin was either HI or LO.
     * (DEVICE_PIN_EVT_PULSE_HI, DEVICE_PIN_EVT_PULSE_LO) DEVICE_PIN_EVENT_ON_TOUCH - Configures this pin as a makey
     * makey style touch sensor, in the form of a DeviceButton. Normal button events will be generated using the ID of
     * this pin. DEVICE_PIN_EVENT_NONE - Disables events for this pin.
     *
     * @param eventType One of: DEVICE_PIN_EVENT_ON_EDGE, DEVICE_PIN_EVENT_ON_PULSE, DEVICE_PIN_EVENT_ON_TOUCH,
     * DEVICE_PIN_EVENT_NONE
     *
     * @code
     * DeviceMessageBus bus;
     *
     * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
     * P0.eventOn(DEVICE_PIN_EVENT_ON_PULSE);
     *
     * void onPulse(Event evt)
     * {
     *     int duration = evt.timestamp;
     * }
     *
     * bus.listen(DEVICE_ID_IO_P0, DEVICE_PIN_EVT_PULSE_HI, onPulse, MESSAGE_BUS_LISTENER_IMMEDIATE)
     * @endcode
     *
     * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER if the given eventype does not match
     *
     * @note In the DEVICE_PIN_EVENT_ON_PULSE mode, the smallest pulse that was reliably detected was 85us, around 5khz.
     * If more precision is required, please use the InterruptIn class supplied by ARM mbed.
     */
    int eventOn(int eventType) final override;

    void eventRiseAndFall();

  private:
    void disconnect();

    /**
     * This member function will construct an TimedInterruptIn instance, and configure
     * interrupts for rise and fall.
     *
     * @param eventType the specific mode used in interrupt context to determine how an
     *                  edge/rise is processed.
     *
     * @return DEVICE_OK on success
     */
    int enableRiseFallEvents(int eventType);

    /**
     * If this pin is in a mode where the pin is generating events, it will destruct
     * the current instance attached to this STM32L4xxPin instance.
     *
     * @return DEVICE_OK on success.
     */
    int disableEvents();

    codal::STM32PWM* pwm;
    uint32_t analogFrequency;
    uint32_t pulseInitialTimestamp;
    uint8_t eventFlags;
};

}  // namespace codal

#endif /* CODAL_STM32_PIN_H */
