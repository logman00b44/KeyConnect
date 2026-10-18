#ifndef _USB_CONFIG_H
#define _USB_CONFIG_H

#include "funconfig.h"
#include "ch32fun.h"

#define FUSB_CONFIG_EPS       2
#define FUSB_SUPPORTS_SLEEP   0
#define FUSB_HID_INTERFACES   1
#define FUSB_HID_USER_REPORTS 1
#define FUSB_IO_PROFILE       0
#define FUSB_USE_HPE          FUNCONF_ENABLE_HPE
#define FUSB_EP1_MODE         1

#include "usb_defines.h"

static const uint8_t device_descriptor[] = {
    18, 1, 0x00, 0x02, 0, 0, 0, 64,
    0x09, 0x27, 0x01, 0x00, 0x00, 0x02,
    1, 2, 0, 1
};

static const uint8_t key_rep_desc[] = {
    0x05, 0x01,
    0x09, 0x06, 0xA1, 0x01,
    0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7,
    0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02,
    0x95, 0x01, 0x75, 0x08, 0x81, 0x01,
    0x95, 0x06, 0x75, 0x08,
    0x26, 0xFF, 0x00, 0x05, 0x07,
    0x19, 0x00, 0x29, 0x91, 0x81, 0x00,
    0xC0
};

static const uint8_t config_descriptor[] = {
    0x09, 0x02, 0x22, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32,
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, sizeof(key_rep_desc), 0x00,
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0A
};

#define STR_MANUFACTURER u"KeyConnect"
#define STR_PRODUCT      u"KeyConnect R1 Rev 0.2 Dev"

struct usb_string_descriptor_struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wString[];
};

static const struct usb_string_descriptor_struct string0 = { 4, 3, {0x0409} };
static const struct usb_string_descriptor_struct string1 = { sizeof(STR_MANUFACTURER), 3, STR_MANUFACTURER };
static const struct usb_string_descriptor_struct string2 = { sizeof(STR_PRODUCT), 3, STR_PRODUCT };

static const struct descriptor_list_struct {
    uint32_t lIndexValue;
    const uint8_t *addr;
    uint8_t length;
} descriptor_list[] = {
    {0x00000100, device_descriptor, sizeof(device_descriptor)},
    {0x00000200, config_descriptor, sizeof(config_descriptor)},
    {0x00002200, key_rep_desc, sizeof(key_rep_desc)},
    {0x00002100, config_descriptor + 18, 9},
    {0x00000300, (const uint8_t *)&string0, 4},
    {0x04090301, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
    {0x04090302, (const uint8_t *)&string2, sizeof(STR_PRODUCT)},
};

#define DESCRIPTOR_LIST_ENTRIES (sizeof(descriptor_list) / sizeof(struct descriptor_list_struct))

#endif
