#include "openlog.h"

#include <stdio.h>

using namespace codal;

constexpr const unsigned BUFFER_SIZE           = 100;
constexpr const unsigned OPENLOG_BAUDRATE      = 9600;
constexpr const char* APPEND                   = "append";
constexpr const char* FILE_NAME                = "out.csv";
constexpr const uint8_t ESCAPE_SEQUENCE[]      = {26, 26, 26};
constexpr const char READY_TO_RECEIVE_ANY      = '<';
constexpr const char READY_TO_RECEIVE_COMMANDS = '>';

OpenLog::OpenLog(STM32Pin& tx, STM32Pin& rx, STM32Pin& reset) : resetPin(reset)
{
    this->buffer = (uint8_t*)malloc(BUFFER_SIZE);
    this->serial = new STM32Serial(tx, rx);
    this->serial->init(OPENLOG_BAUDRATE);
    this->reset();
    this->waitUntilReady(READY_TO_RECEIVE_ANY);
    this->goToCommandMode();
}

OpenLog::~OpenLog()
{
    free(this->buffer);
}

bool OpenLog::write(const char* data, unsigned dataSize)
{
    int sendResult;
    const unsigned commandLength = strlen(APPEND) + 1 + strlen(FILE_NAME) + 1;
    strcat(strcat(strcat(strcpy((char*)this->buffer, APPEND), " "), FILE_NAME), "\r");

    sendResult = this->sendData(this->buffer, commandLength);
    if (sendResult == DEVICE_INVALID_PARAMETER || sendResult == DEVICE_SERIAL_IN_USE) return false;

    this->waitUntilReady(READY_TO_RECEIVE_ANY);
    sendResult = this->sendData((uint8_t*)data, dataSize - 1);  // -1 because we're not sending the trailing \0
    if (sendResult == DEVICE_INVALID_PARAMETER || sendResult == DEVICE_SERIAL_IN_USE) return false;

    this->goToCommandMode();

    return true;
}

int OpenLog::sendData(const uint8_t* data, unsigned dataSize)
{
    const unsigned maxSendSize = CODAL_SERIAL_DEFAULT_BUFFER_SIZE;
    const unsigned sendNumbers = dataSize / maxSendSize + !(dataSize % maxSendSize == 0);
    unsigned transmittedBytes  = 0;
    int sendResult;

    for (unsigned i = 0; i < sendNumbers; ++i) {
        unsigned currentSendSize = (i < dataSize / maxSendSize) ? maxSendSize : dataSize % maxSendSize;
        buffer                   = (uint8_t*)memcpy(this->buffer, data + (i * maxSendSize), currentSendSize);
        sendResult               = this->serial->send(this->buffer, currentSendSize);

        if (sendResult != DEVICE_INVALID_PARAMETER && sendResult != DEVICE_SERIAL_IN_USE)
            transmittedBytes += sendResult;
        else {
            return sendResult;
        }
    }
    return transmittedBytes;
}

void OpenLog::goToCommandMode()
{
    this->sendData(ESCAPE_SEQUENCE, 3);
    this->waitUntilReady(READY_TO_RECEIVE_COMMANDS);
}

void OpenLog::reset()
{
    this->resetPin.setDigitalValue(0);
    fiber_sleep(100);
    this->resetPin.setDigitalValue(1);
}

void OpenLog::waitUntilReady(const char waitChar)
{
    if (!(waitChar == '>' || waitChar == '<')) return;
    do {
        fiber_sleep(10);
        this->serial->read(this->buffer, BUFFER_SIZE, ASYNC);
    } while (strchr((char*)this->buffer, waitChar) == NULL);
}