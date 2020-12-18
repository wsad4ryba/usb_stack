/*
 * Copyright  Onplick <info@onplick.com> - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */
#include <mutex.hpp>
#include "usb.hpp"

extern "C"
{
#include "board.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "composite.h"
#include "usb_phy.h"
}

namespace bsp
{
    usb_device_composite_struct_t *usbDeviceComposite = nullptr;
    TaskHandle_t usbTaskHandle = NULL;
    xQueueHandle USBReceiveQueue;
    static cpp_freertos::MutexStandard mutex;

    char usbSerialBuffer[SERIAL_BUFFER_LEN];

#if USBCDC_ECHO_ENABLED
    bool usbCdcEchoEnabled = false;

    constexpr std::string_view usbCDCEchoOnCmd("UsbCdcEcho=ON");
    constexpr std::string_view usbCDCEchoOffCmd("UsbCdcEcho=OFF");

    constexpr inline auto usbCDCEchoOnCmdLength  = usbCDCEchoOnCmd.length();
    constexpr inline auto usbCDCEchoOffCmdLength = usbCDCEchoOffCmd.length();
#endif

    int usbInit(xQueueHandle queueHandle, USBDeviceListener *deviceListener)
    {
        BaseType_t xReturned = xTaskCreate(reinterpret_cast<TaskFunction_t>(&bsp::usbDeviceTask),
                                           "bsp::usbDeviceTask",
                                           2048L / sizeof(portSTACK_TYPE),
                                           deviceListener, 2,
                                           &bsp::usbTaskHandle);

        if (xReturned == pdPASS) {
            LOG_DEBUG("init created device task");
        } else {
            LOG_DEBUG("init can't create device task");
            return -1;
        }

        USBReceiveQueue = queueHandle;
        usbDeviceComposite= composite_init();

        return (usbDeviceComposite == NULL) ? -1 : 0;
    }

    void usbDeinit()
    {
        LOG_INFO("usbDeinit");
        composite_deinit(usbDeviceComposite);
    }

    void usbDeviceTask(void *handle)
    {
        USBDeviceListener *deviceListener = static_cast<USBDeviceListener *>(handle);
        uint32_t dataReceivedLength;

        vTaskDelay(3000 / portTICK_PERIOD_MS);

        while (1) {
            dataReceivedLength = usbCDCReceive(&usbSerialBuffer);

            if (dataReceivedLength > 0) {
                LOG_DEBUG("usbDeviceTask Received: %d signs: [%s]", static_cast<int>(dataReceivedLength), usbSerialBuffer);

#if USBCDC_ECHO_ENABLED
                bool usbCdcEchoEnabledPrev = usbCdcEchoEnabled;

                auto usbEchoCmd = std::string_view{usbSerialBuffer, static_cast<size_t>(dataReceivedLength)};

                if ((dataReceivedLength == usbCDCEchoOnCmdLength) && (usbCDCEchoOnCmd == usbEchoCmd)) {
                    usbCdcEchoEnabled = true;
                }
                else if ((dataReceivedLength == usbCDCEchoOffCmdLength) && (usbCDCEchoOffCmd == usbEchoCmd)) {
                    usbCdcEchoEnabled = false;
                }

                if (usbCdcEchoEnabled || usbCdcEchoEnabledPrev) {
                    usbCDCSendRaw(usbSerialBuffer, dataReceivedLength);
                    LOG_DEBUG("usbDeviceTask echoed: %d signs: [%s]", static_cast<int>(dataReceivedLength), usbSerialBuffer);
                    continue;
                }
#endif

                if (deviceListener->getRawMode()) {
                    cpp_freertos::LockGuard lock(mutex);
                    deviceListener->rawDataReceived(&usbSerialBuffer, dataReceivedLength);
                }
                else if (uxQueueSpacesAvailable(USBReceiveQueue) != 0) {
                    static std::string receiveMessage;
                    receiveMessage = std::string(usbSerialBuffer, dataReceivedLength);
                    if (xQueueSend(USBReceiveQueue, &receiveMessage, portMAX_DELAY) == errQUEUE_FULL) {
                        LOG_ERROR("usbDeviceTask can't send data [%s] to receiveQueue", receiveMessage.c_str());
                    }
                }
            }
            else {
                vTaskDelay(1 / portTICK_PERIOD_MS);
            }
        }
    }

    int usbCDCReceive(void *buffer)
    {
        if (usbDeviceComposite->cdcVcom.inputStream == nullptr)
            return 0;

        memset(buffer, 0, SERIAL_BUFFER_LEN);

        return VirtualComRecv(&usbDeviceComposite->cdcVcom, buffer, SERIAL_BUFFER_LEN);
    }

    int usbCDCSend(std::string *message)
    {
        return usbCDCSendRaw(message->c_str(), message->length());
    }

    int usbCDCSendRaw(const char *dataPtr, size_t dataLen)
    {
        uint32_t dataSent = 0;

        do {
            uint32_t len =  VirtualComSend(&usbDeviceComposite->cdcVcom, dataPtr + dataSent, dataLen - dataSent);
            if (!len) {
                vTaskDelay(1 / portTICK_PERIOD_MS);
                continue;
            }
            dataSent += len;
        } while(dataSent < dataLen);

        return dataSent;
    }
}
