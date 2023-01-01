#include "YmDevice.h"

/*
    Creates COM device communication layer
    Use dcbDef to specify custom DCB build string to configure baudrate, stopbits, etc.
*/
YmDevice::YmDevice(LPCWSTR fileName, LPCWSTR dcbDef) :
    pFrame1({0}), pFrame2({ 0 })
{
    hComPort = CreateFileW(fileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hComPort == INVALID_HANDLE_VALUE) {
        throw W32Exception(L"CreateFileW");
    }
    if (!GetCommState(hComPort, &dcb)) {
        throw W32Exception(L"GetCommState");
    }
    FillMemory(&dcb, sizeof(dcb), 0);
    dcb.DCBlength = sizeof(dcb);
    if (!BuildCommDCB(dcbDef ? dcbDef : L"500000,n,8,1", &dcb)) {
        throw W32Exception(L"BuildCommDCB");
    }
    if (!SetCommState(hComPort, &dcb)) {
        throw W32Exception(L"SetCommState");
    }
}

DWORD YmDevice::sendBuffer(uint8_t* buffer, DWORD length)
{
    DWORD written, read;
    if (!WriteFile(hComPort, buffer, length, &written, NULL)) {
        throw W32Exception(L"WriteFile");
    }
    if (written != length) {
        throw std::runtime_error("WriteFile incomplete");
    }
    if (!ReadFile(hComPort, buffer, length, &read, NULL)) {
        throw W32Exception(L"ReadFile");
    }
    if (read != length) {
        throw std::runtime_error("ReadFile incomplete");
    }
    for (uint32_t i = 0; i < length; i++) {
        if (buffer[i] != 0x06) {
            throw std::runtime_error("Device not returned ACK");
        }
    }
    return length;
}

/*
    Updates all YM registers from given structure
    R13 (0x0D) is treated specially - value 0xFF skips it, according to the Leonard protocol.
*/
void YmDevice::sendFrame(YmFrame nFrame) 
{
    uint8_t buffer[28];
    uint8_t length = 0;
    for (uint8_t r = 0; r < 13; r++) {
        buffer[length++] = r;
        buffer[length++] = nFrame.R[r];
    }
    if (nFrame.R[13] != 0xFF) {
        buffer[length++] = 13;
        buffer[length++] = nFrame.R[13];
    }
    sendBuffer(buffer, length);
    pFrame1 = pFrame2 = nFrame;
}

/*
    Updates only changed YM registers from given structure
    R13 (0x0D) is treated specially - value 0xFF skips it, according to the Leonard protocol.
*/
void YmDevice::updateFrame(YmFrame nFrame) {
    uint8_t buffer[28];
    uint8_t length = 0;
    for (uint8_t r = 0; r < 13; r++) {
        if ((nFrame.R[r] != pFrame1.R[r]) || (nFrame.R[r] != pFrame2.R[r])) {
            buffer[length++] = r;
            buffer[length++] = nFrame.R[r];
        }
    }
    if (nFrame.R[13] != 0xFF) {
        buffer[length++] = 13;
        buffer[length++] = nFrame.R[13];
    }
    if (length) {
        sendBuffer(buffer, length);
        pFrame1 = pFrame2 = nFrame;
    } 
}

/*
    Sends two different frames to two devices
    Behavior is very similar to updateFrame, but additional optimizations are made.
*/
void YmDevice::updateTurboFrame(YmFrame nFrame1, YmFrame nFrame2) {
    uint8_t buffer[56];
    uint8_t length = 0;
    uint8_t r;
    for (r = 0; r < 13; r++) {
        if ((nFrame1.R[r] != pFrame1.R[r]) && (nFrame2.R[r] != pFrame2.R[r]) && (nFrame1.R[r] == nFrame2.R[r])) {
            buffer[length++] = r;
            buffer[length++] = nFrame1.R[r];
        }
        else {
            if (nFrame1.R[r] != pFrame1.R[r]) {
                buffer[length++] = (r | 0x10);
                buffer[length++] = nFrame1.R[r];
            }
            if (nFrame2.R[r] != pFrame2.R[r]) {
                buffer[length++] = (r | 0x20);
                buffer[length++] = nFrame2.R[r];
            }
        }
    }
    r = 13;
    if ((nFrame1.R[r] != 0xFF) && (nFrame2.R[r] != 0xFF) && (nFrame1.R[r] == nFrame2.R[r])) {
        buffer[length++] = r;
        buffer[length++] = nFrame1.R[r];
    }
    else {
        if (nFrame1.R[r] != 0xFF) {
            buffer[length++] = (r | 0x10);
            buffer[length++] = nFrame1.R[r];
        }
        if (nFrame2.R[r] != 0xFF) {
            buffer[length++] = (r | 0x20);
            buffer[length++] = nFrame1.R[r];
        }
    }
    if (length) {
        sendBuffer(buffer, length);
        pFrame1 = nFrame1;
        pFrame2 = nFrame2;
    }
}

/*
    Mute device
    Sets channel A, B, C levels to 0, envelope off, disable mixer
*/
void YmDevice::mute(void) 
{
    uint8_t packet[8] = { 007, 0xFF, 010, 0x00, 011, 0x00, 012, 0x00 };
    sendBuffer(packet, sizeof(packet));
}

/*
    Set single register value
*/
void YmDevice::setRegister(uint8_t reg, uint8_t val) {
    uint8_t packet[2] = { reg, val };
    sendBuffer(packet, sizeof(packet));
}

YmDevice::~YmDevice()
{
    CloseHandle(hComPort);
    hComPort = INVALID_HANDLE_VALUE;
}