#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <hidapi.h>
#else
#include <hidapi/hidapi.h>
#endif

#define VID 0x2709
#define PID 0x0001

static void list_devices(void)
{
    if (hid_init()) { fprintf(stderr, "hidapi init failed\n"); return; }

    struct hid_device_info *devs = hid_enumerate(VID, PID);
    if (!devs) {
        printf("No KeyConnect devices found.\n");
    } else {
        for (struct hid_device_info *cur = devs; cur; cur = cur->next) {
            printf("  path: %s\n", cur->path);
            printf("  product: %ls\n", cur->product_string);
            printf("  serial: %ls\n\n", cur->serial_number);
        }
    }
    hid_free_enumeration(devs);
    hid_exit();
}

static void do_read(int layer, int key)
{
    hid_device *dev = hid_open(VID, PID, NULL);
    if (!dev) { fprintf(stderr, "Device not found\n"); return; }

    unsigned char setBuf[5] = { 0x01, layer, key, 0, 0 };
    if (hid_send_feature_report(dev, setBuf, sizeof(setBuf)) < 0) {
        fprintf(stderr, "set_feature (cursor) failed: %ls\n", hid_error(dev));
        hid_close(dev);
        return;
    }

    unsigned char getBuf[5] = { 0x01, 0, 0, 0, 0 };
    if (hid_get_feature_report(dev, getBuf, sizeof(getBuf)) < 0) {
        fprintf(stderr, "get_feature failed: %ls\n", hid_error(dev));
    } else {
        printf("Layer %d Key %d -> mod=0x%02X usage=0x%02X\n",
               getBuf[1], getBuf[2], getBuf[3], getBuf[4]);
    }
    hid_close(dev);
}

static void do_write(int layer, int key, int mod, int usage)
{
    hid_device *dev = hid_open(VID, PID, NULL);
    if (!dev) { fprintf(stderr, "Device not found\n"); return; }

    unsigned char buf[5] = { 0x02, layer, key, mod, usage };
    if (hid_send_feature_report(dev, buf, sizeof(buf)) < 0) {
        fprintf(stderr, "set_feature failed: %ls\n", hid_error(dev));
    } else {
        printf("Written: Layer %d Key %d <- mod=0x%02X usage=0x%02X\n",
               layer, key, mod, usage);
    }
    hid_close(dev);
}

static void do_save(void)
{
    hid_device *dev = hid_open(VID, PID, NULL);
    if (!dev) { fprintf(stderr, "Device not found\n"); return; }

    unsigned char buf[2] = { 0x03, 0xA5 };
    if (hid_send_feature_report(dev, buf, sizeof(buf)) < 0) {
        fprintf(stderr, "save failed: %ls\n", hid_error(dev));
    } else {
        printf("Configuration saved to flash.\n");
    }
    hid_close(dev);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage:\n"
               "  %s list\n"
               "  %s read <layer> <key>\n"
               "  %s write <layer> <key> <mod> <usage>\n"
               "  %s save\n", argv[0], argv[0], argv[0], argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "list")) {
        list_devices();
    } else if (!strcmp(argv[1], "read") && argc == 4) {
        do_read(atoi(argv[2]), atoi(argv[3]));
    } else if (!strcmp(argv[1], "write") && argc == 6) {
        do_write(atoi(argv[2]), atoi(argv[3]),
                 strtol(argv[4], NULL, 0), strtol(argv[5], NULL, 0));
    } else if (!strcmp(argv[1], "save")) {
        do_save();
    } else {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }
    return 0;
}
