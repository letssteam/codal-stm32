#include "STM32Pin.h"

#include "EventModel.h"
#include "PeripheralPins.h"
#include "PinConfigured.h"
#include "analog.h"
#include "clock.h"
#include "digital_io.h"
#include "interrupt.h"
#include "pins_arduino.h"

using namespace codal;

#ifndef DEFAULT_PWM_FREQ
#define DEFAULT_PWM_FREQ 20000
#endif

#ifndef PWM_SERVO_FREQ
#define PWM_SERVO_FREQ 50
#endif

#ifndef ANALOG_PWM_RESOLUTION
#define ANALOG_PWM_RESOLUTION 10
#endif

#ifndef ADC_SENSOR_RESOLUTION
#define ADC_SENSOR_RESOLUTION 12
#endif

#ifndef ADC_OUPUT_RESOLUTION
#define ADC_OUPUT_RESOLUTION 10
#endif

extern uint32_t g_anOutputPinConfigured[MAX_NB_PORT];
constexpr uint8_t INTERRUPT_ON_EDGE = 0x01;
constexpr uint8_t EVT_ON_EDGE       = 0x02;
constexpr uint8_t EVT_ON_PULSE      = 0x04;

static inline uint32_t mapResolution(uint32_t value, uint32_t from, uint32_t to)
{
    if (from != to) {
        if (from > to) {
            value = (value < (uint32_t)(1 << (from - to))) ? 0 : ((value + 1) >> (from - to)) - 1;
        }
        else {
            if (value != 0) {
                value = ((value + 1) << (to - from)) - 1;
            }
        }
    }
    return value;
}

STM32Pin::STM32Pin(int id, PinNumber name, PinCapability capability)
    : codal::Pin(id, name, capability), pwm(nullptr), analogFrequency(DEFAULT_PWM_FREQ)
{
    this->pullMode = DEVICE_DEFAULT_PULLMODE;

    // Power up in a disconnected, low power state.
    // If we're unused, this is how it will stay...
    this->status                = 0x00;
    this->pulseInitialTimestamp = 0;
    this->eventFlags            = 0x0;

    if ((PIN_CAPABILITY_ANALOG & capability) && pin_in_pinmap((PinName)name, PinMap_PWM)) {
        pwm = new STM32PWM((PinName)name, analogFrequency);
    }
}

inline int map(codal::PullMode pinMode)
{
    switch (pinMode) {
        case PullMode::Up:
            return GPIO_PULLUP;
        case PullMode::Down:
            return GPIO_PULLDOWN;
        case PullMode::None:
            return GPIO_NOPULL;
    }

    return GPIO_NOPULL;
}

inline PinName to_pinName(PinNumber pin)
{
    return (PinName)pin;
}

void STM32Pin::disconnect()
{
    this->status = 0x00;
    if (!(PIN_CAPABILITY_ANALOG & capability)) return;

    if (pwm != nullptr) {
        pwm->stop();
    }
}

int STM32Pin::setDigitalValue(int value)
{
    // Ensure we have a valid value.
    if (value < 0 || value > 1) return DEVICE_INVALID_PARAMETER;

    // Move into a Digital input state if necessary.
    if (!(this->status & IO_STATUS_DIGITAL_OUT)) {
        disconnect();
        pin_function(to_pinName(this->name), STM_PIN_DATA(STM_MODE_OUTPUT_PP, map(this->pullMode), 0));
        this->status |= IO_STATUS_DIGITAL_OUT;
    }

    digital_io_write(get_GPIO_Port(STM_PORT(this->name)), STM_LL_GPIO_PIN(this->name), value);
    return DEVICE_OK;
}

int STM32Pin::getDigitalValue()
{
    // Move into a Digital input state if necessary.
    if (!(status & (IO_STATUS_DIGITAL_IN | IO_STATUS_EVENT_ON_EDGE | IO_STATUS_EVENT_PULSE_ON_EDGE))) {
        disconnect();
        pin_function(to_pinName(this->name), STM_PIN_DATA(STM_PIN_INPUT, map(this->pullMode), 0));
        status |= IO_STATUS_DIGITAL_IN;
    }

    uint8_t level = 0;
    level         = digital_io_read(get_GPIO_Port(STM_PORT(this->name)), STM_LL_GPIO_PIN(this->name));

    return (level) ? HIGH : LOW;
}

int STM32Pin::setAnalogValue(int value)
{
    // check if this pin has an analogue mode...
    if (!(PIN_CAPABILITY_ANALOG & capability)) return DEVICE_NOT_SUPPORTED;

    if (!(status & IO_STATUS_ANALOG_OUT)) {
        disconnect();
        pwm->init();

        status = IO_STATUS_ANALOG_OUT;
    }

    // sanitise the level value
    if (value < 0 || value > DEVICE_PIN_MAX_OUTPUT) return DEVICE_INVALID_PARAMETER;

    if (pwm == nullptr) return DEVICE_NOT_SUPPORTED;

    pwm->setFrequency(analogFrequency);
    pwm->setDutyCycleFromResolution(value, ANALOG_PWM_RESOLUTION);
    pwm->start();

    return DEVICE_OK;
}

int STM32Pin::setServoValue(int value, int range, int center)
{
    // check if this pin has an analogue mode...
    if (!(PIN_CAPABILITY_ANALOG & capability)) return DEVICE_NOT_SUPPORTED;

    if (pwm == nullptr) return DEVICE_NOT_SUPPORTED;

    // sanitise the servo level
    if (value < 0 || range < 1 || center < 1) return DEVICE_INVALID_PARAMETER;

    // clip - just in case
    if (value > DEVICE_PIN_MAX_SERVO_RANGE) value = DEVICE_PIN_MAX_SERVO_RANGE;

    // calculate the lower bound based on the midpoint
    int lower = (center - (range / 2)) * 1000;

    value = value * 1000;

    // add the percentage of the range based on the value between 0 and 180
    int scaled = lower + (range * (value / DEVICE_PIN_MAX_SERVO_RANGE));

    return setServoPulseUs(scaled / 1000);
}

int STM32Pin::getAnalogValue()
{
    // check if this pin has an analogue mode...
    if (!(PIN_CAPABILITY_ANALOG & capability)) return DEVICE_NOT_SUPPORTED;

    if (!(status & IO_STATUS_ANALOG_IN)) {
        disconnect();
        pin_function(to_pinName(this->name), STM_PIN_DATA(STM_PIN_ANALOG, map(this->pullMode), 0));

        status = IO_STATUS_ANALOG_IN;
    }

    uint32_t value = 0;
    PinName p      = to_pinName(this->name);
    if (p != NC) {
        value = adc_read_value(p, ADC_SENSOR_RESOLUTION);
        value = mapResolution(value, ADC_SENSOR_RESOLUTION, ADC_OUPUT_RESOLUTION);
    }

    uint16_t adc_offset = 0;

    if (value <= 0)
        adc_offset = value;
    else if (value > 4095)
        adc_offset = value - 4095;

    return (value - adc_offset);
}

int STM32Pin::isTouched()
{
    return DEVICE_NOT_SUPPORTED;
}

int STM32Pin::setServoPulseUs(uint32_t pulseWidthUs)
{
    // check if this pin has an analogue mode...
    if (!(PIN_CAPABILITY_ANALOG & capability)) return DEVICE_NOT_SUPPORTED;

    if (pwm == nullptr) return DEVICE_NOT_SUPPORTED;

    pwm->setFrequency(PWM_SERVO_FREQ);
    pwm->setDutyCycleFromUs(pulseWidthUs);
    pwm->start();

    return DEVICE_OK;
}

int STM32Pin::setAnalogPeriod(int periodMs)
{
    // check if this pin has an analogue mode...
    if (!(PIN_CAPABILITY_ANALOG & capability)) return DEVICE_NOT_SUPPORTED;

    if (pwm == nullptr) return DEVICE_NOT_SUPPORTED;

    analogFrequency = (uint32_t)1000 / periodMs;

    return DEVICE_OK;
}
int STM32Pin::setAnalogPeriodUs(uint32_t periodUs)
{
    // check if this pin has an analogue mode...
    if (!(PIN_CAPABILITY_ANALOG & capability)) return DEVICE_NOT_SUPPORTED;

    if (pwm == nullptr) return DEVICE_NOT_SUPPORTED;

    analogFrequency = (uint32_t)1000000 / periodUs;

    return DEVICE_OK;
}
uint32_t STM32Pin::getAnalogPeriodUs()
{
    // check if this pin has an analogue mode...
    if (!(PIN_CAPABILITY_ANALOG & capability)) return 0;

    if (pwm == nullptr) return 0;

    return (uint32_t)1000000 / analogFrequency;
}
int STM32Pin::setPull(PullMode newPullMode)
{
    if (pullMode == newPullMode) return DEVICE_OK;

    pullMode = newPullMode;

    // have to disconnect to flush the change to the hardware
    disconnect();
    getDigitalValue();

    return DEVICE_OK;
}

int STM32Pin::eventOn(int eventType)
{
    switch (eventType) {
        case DEVICE_PIN_INTERRUPT_ON_EDGE:
        case DEVICE_PIN_EVENT_ON_EDGE:
        case DEVICE_PIN_EVENT_ON_PULSE:
            enableRiseFallEvents(eventType);
            break;

        case DEVICE_PIN_EVENT_NONE:
            disableEvents();
            break;

        default:
            return DEVICE_INVALID_PARAMETER;
    }

    return DEVICE_OK;
}

void STM32Pin::eventRiseAndFall()
{
    GPIO_PinState state = HAL_GPIO_ReadPin(get_GPIO_Port(STM_PORT(this->name)), STM_LL_GPIO_PIN(this->name));

    if (this->eventFlags & INTERRUPT_ON_EDGE && this->gpio_irq != NULL) this->gpio_irq(state);

    if (state == GPIO_PIN_RESET) {
        if (this->eventFlags & EVT_ON_EDGE) Event(this->id, DEVICE_PIN_EVT_FALL, CREATE_AND_FIRE);

        if (this->eventFlags & EVT_ON_PULSE) {
            if (getPolarity() > 0) {
                Event(this->id, DEVICE_PIN_EVT_PULSE_HI, getCurrentMicros() - this->pulseInitialTimestamp,
                      CREATE_AND_FIRE);
            }
            else {
                this->pulseInitialTimestamp = getCurrentMicros();
            }
        }
    }
    else {
        if (this->eventFlags & EVT_ON_EDGE) Event(this->id, DEVICE_PIN_EVT_RISE, CREATE_AND_FIRE);

        if (this->eventFlags & EVT_ON_PULSE) {
            if (getPolarity() > 0) {
                this->pulseInitialTimestamp = getCurrentMicros();
            }
            else {
                Event(this->id, DEVICE_PIN_EVT_PULSE_LO, getCurrentMicros() - this->pulseInitialTimestamp,
                      CREATE_AND_FIRE);
            }
        }
    }
}

/**
 * Enables interrupts at the hardware level and generates Codal Events when an interruption occurs
 * @param eventType The type of event to generate
 * @return DEVICE_OK if method is supported, DEVICE_NOT_SUPPORTED otherwise
 */
int STM32Pin::enableRiseFallEvents(int eventType)
{
    set_GPIO_Port_Clock(STM_PORT(this->name));

    switch (eventType) {
        case DEVICE_PIN_INTERRUPT_ON_EDGE:
            this->eventFlags = eventFlags | INTERRUPT_ON_EDGE;
            break;
        case DEVICE_PIN_EVENT_ON_EDGE:
            this->eventFlags = eventFlags | EVT_ON_EDGE;
            break;
        case DEVICE_PIN_EVENT_ON_PULSE:
            this->eventFlags = eventFlags | EVT_ON_PULSE;
            break;
    }
    stm32_interrupt_enable(get_GPIO_Port(STM_PORT(this->name)), STM_LL_GPIO_PIN(this->name),
                           std::bind(&STM32Pin::eventRiseAndFall, this), GPIO_MODE_IT_RISING_FALLING);
    return DEVICE_OK;
}

int STM32Pin::disableEvents()
{
    if (status & (IO_STATUS_EVENT_ON_EDGE | IO_STATUS_EVENT_PULSE_ON_EDGE | IO_STATUS_TOUCH_IN)) {
        this->eventFlags = 0x0;
        disconnect();
        getDigitalValue();
    }

    return DEVICE_OK;
    ;
}