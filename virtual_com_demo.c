/*
 * Copyright  Onplick <info@onplick.com> - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "composite.h"

#include "virtual_com_demo.h"

static void VirtualComConsumerTask(void *handle)
{
#define RX_CHUNK 512
    char *input = pvPortMalloc(RX_CHUNK);
    usb_device_composite_struct_t *usbComposite = handle;
    size_t len;
    size_t total = 0;

    if (!input) {
        LOG_ERROR("Buffer allocation failed");
        return;
    }

    vTaskDelay(3000/portTICK_PERIOD_MS);

    LOG_INFO("Task started");

    while (1)
    {
        len = VirtualComRecv(&usbComposite->cdcVcom, input, RX_CHUNK);
        if (len > 0)
        {
            composite_deinit(usbComposite);
            break;
            size_t sent = 0;
            total += len;
            do {
                size_t result = VirtualComSend(&usbComposite->cdcVcom, &input[sent], len-sent);
                if (!result) {
                    continue;
                }
                sent += result;
            } while (sent < len);
        } else {
            vTaskDelay(1/portTICK_PERIOD_MS);
        }
    }

    while(1) {
        LOG_INFO("USB should be deinitialized");
        vTaskDelay(10000/portTICK_PERIOD_MS);
    }
}

void VirtualComDemoInit(usb_device_composite_struct_t *usbComposite)
{
    if (xTaskCreate(VirtualComConsumerTask,
                    "VirtualComDemo",
                    2048L / sizeof(portSTACK_TYPE),
                    usbComposite,
                    2,
                    NULL
                    ) != pdPASS)
    {
        LOG_ERROR("VirtualComDemo xTaskCreate failed");
    }
}
