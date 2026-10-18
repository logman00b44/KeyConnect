#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <hidapi.h>

#define VID 0x2709
#define PID 0x0001
#define PRODUCT_NAME L"KeyConnect R1 Rev 0.1 Dev"
#define NUM_KEYS 16

static const char *fmt_ctrl_char(char c, char *out)
{
    if ((unsigned char)c < 32) {
        snprintf(out, 4, "^%c", c + 0x40);
        return out;
    }
    out[0] = c;
    out[1] = '\0';
    return out;
}

static void fmt_shortcut_printable(const char *raw, char *out, size_t sz)
{
    size_t wi = 0;
    for (const char *p = raw; *p && wi < sz - 4; p++) {
        if ((unsigned char)*p < 32) {
            if (wi + 2 < sz) {
                out[wi++] = '^';
                out[wi++] = *p + 0x40;
            }
        } else {
            out[wi++] = *p;
        }
    }
    out[wi] = '\0';
}

static const char *KEY_LABELS[NUM_KEYS] = {
    "A", "B", "C", "D",
    "E", "F", "G", "H",
    "I", "J", "K", "L",
    "M", "N", "O", "P",
};

static const char *KEY_PIN_NAMES[NUM_KEYS] = {
    "PA0",  "PA1",  "PA2",  "PA3",
    "PA4",  "PA5",  "PA6",  "PA7",
    "PB0",  "PB1",  "PB3",  "PB4",
    "PB6",  "PB7",  "PB10", "PB11",
};

static const char *KEY_REFDES[NUM_KEYS] = {
    "U1",  "U2",  "U3",  "U4",
    "U6",  "U7",  "U8",  "U9",
    "U10", "U11", "U12", "U13",
    "U14", "U15", "U16", "U17",
};

static void timestamp_str(char *buf, size_t sz)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(buf, sz, "%04d-%02d-%02d %02d:%02d:%02d",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond);
}

static int read_input_line(char *buf, int bufsz)
{
    int pos = 0;
    while (pos < bufsz - 1) {
        int ch = _getch();
        if (ch == '\r' || ch == '\n') {
            printf("\n");
            break;
        }
        if (ch == '\b' || ch == 127) {
            if (pos > 0) {
                pos--;
                printf("\b \b");
            }
            continue;
        }
        if (ch < 32 && ch != '\t') {
            printf("^%c", ch + 0x40);
        } else {
            printf("%c", ch);
        }
        buf[pos++] = (char)(unsigned char)ch;
    }
    buf[pos] = '\0';
    return pos;
}

typedef struct {
    int  working;
    char shortcut[64];
    char note[128];
} KeyResult;

int main(void)
{
    printf("KeyConnect R1 - Manual Key Test\n");
    printf("================================\n\n");

    if (hid_init()) {
        fprintf(stderr, "hidapi init failed\n");
        return 1;
    }

    hid_device *dev = hid_open(VID, PID, NULL);
    if (!dev) {
        fprintf(stderr, "KeyConnect device not found (VID=0x%04X PID=0x%04X)\n", VID, PID);
        fprintf(stderr, "Plug in the keyboard and try again.\n");
        hid_exit();
        return 1;
    }

    printf("Device   : %S\n", PRODUCT_NAME);
    printf("VID/PID  : 0x%04X / 0x%04X\n", VID, PID);
    printf("\nHow to use:\n");
    printf("  Press the physical button on the keyboard.\n");
    printf("  Observe the shortcut it types on your PC.\n");
    printf("  Type what you saw here and press ENTER.\n");
    printf("  'n' = not working   'q' = quit early\n\n");

    hid_close(dev);
    hid_exit();

    printf("Press ENTER to start...");
    _getch();
    printf("\n\n");

    KeyResult results[NUM_KEYS];
    memset(results, 0, sizeof(results));

    int i;
    for (i = 0; i < NUM_KEYS; i++) {
        printf("========================================\n");
        printf("Key %2d of %2d\n", i + 1, NUM_KEYS);
        printf("  Ref   : %s    Label: %s    Pin: %s\n",
               KEY_REFDES[i], KEY_LABELS[i], KEY_PIN_NAMES[i]);
        printf("\n  PRESS the button, then type result:\n");
        printf("  > ");

        char line[256];
        int n = read_input_line(line, sizeof(line));

        if (n < 0) {
            printf("  -> Aborted by user.\n");
            break;
        }

        if (n == 0) {
            printf("  -> Skipped.\n");
            results[i].working = 0;
            strcpy(results[i].shortcut, "");
            strcpy(results[i].note, "skipped");
            continue;
        }

        if (strcmp(line, "q") == 0 || strcmp(line, "Q") == 0) {
            printf("  -> Quit requested.\n");
            break;
        }

        if (strcmp(line, "n") == 0 || strcmp(line, "N") == 0) {
            printf("  -> NOT WORKING.\n");
            results[i].working = 0;
            strcpy(results[i].shortcut, "");
            strcpy(results[i].note, "user reported not working");
            continue;
        }

        results[i].working = 1;
        strncpy(results[i].shortcut, line, sizeof(results[i].shortcut) - 1);
        results[i].shortcut[sizeof(results[i].shortcut) - 1] = '\0';
        {
            char printable[64];
            fmt_shortcut_printable(results[i].shortcut, printable, sizeof(printable));
            printf("  -> Recorded: %s\n", printable);
        }
    }

    FILE *log = fopen("session.log", "w");
    if (!log) {
        fprintf(stderr, "Error: could not write session.log\n");
        return 1;
    }

    char ts[32];
    timestamp_str(ts, sizeof(ts));

    int tested = 0, working = 0, failed = 0;
    for (i = 0; i < NUM_KEYS; i++) {
        if (results[i].shortcut[0] || results[i].note[0]) {
            tested++;
            if (results[i].working) working++;
            else failed++;
        }
    }

    fprintf(log, "KeyConnect R1 - Manual Test Session Log\n");
    fprintf(log, "========================================\n");
    fprintf(log, "Date    : %s\n", ts);
    fprintf(log, "Product : %S\n", PRODUCT_NAME);
    fprintf(log, "VID/PID : 0x%04X / 0x%04X\n", VID, PID);
    fprintf(log, "Tested  : %d / %d keys\n", tested, NUM_KEYS);
    fprintf(log, "Working : %d\n", working);
    if (failed > 0) fprintf(log, "Failed  : %d\n", failed);
    fprintf(log, "----------------------------------------\n\n");

    fprintf(log, "%-4s %-4s %-5s %-8s %-30s %s\n",
            "Idx", "Ref", "Label", "Pin", "Shortcut", "Status");
    fprintf(log, "%-4s %-4s %-5s %-8s %-30s %s\n",
            "---", "---", "-----", "--------", "------------------------------", "------");

    for (i = 0; i < NUM_KEYS; i++) {
        const char *sc = results[i].shortcut;
        const char *nt = results[i].note;
        char printable[64];
        fmt_shortcut_printable(sc, printable, sizeof(printable));

        if (!sc[0] && !nt[0]) {
            fprintf(log, "%-4d %-4s %-5s %-8s %-30s %s\n",
                    i, KEY_REFDES[i], KEY_LABELS[i], KEY_PIN_NAMES[i],
                    "-", "UNTESTED");
        } else if (results[i].working) {
            fprintf(log, "%-4d %-4s %-5s %-8s %-30s OK\n",
                    i, KEY_REFDES[i], KEY_LABELS[i], KEY_PIN_NAMES[i],
                    printable);
        } else {
            fprintf(log, "%-4d %-4s %-5s %-8s %-30s FAIL",
                    i, KEY_REFDES[i], KEY_LABELS[i], KEY_PIN_NAMES[i],
                    printable[0] ? printable : "-");
            if (nt[0]) fprintf(log, " [%s]", nt);
            fprintf(log, "\n");
        }
    }

    fprintf(log, "\n----------------------------------------\n");
    fprintf(log, "Summary: %d/%d tested, %d working", tested, NUM_KEYS, working);
    if (failed > 0) fprintf(log, ", %d FAILED", failed);
    fprintf(log, "\n");
    if (failed > 0) {
        for (i = 0; i < NUM_KEYS; i++) {
            if (!results[i].working && results[i].note[0]) {
                fprintf(log, "  FAIL: Key %d (%s, %s, %s) - %s\n",
                        i, KEY_REFDES[i], KEY_LABELS[i], KEY_PIN_NAMES[i],
                        results[i].note);
            }
        }
    }
    fprintf(log, "========================================\n");
    fclose(log);

    printf("\n========================================\n");
    printf("Test complete.\n");
    printf("  Tested : %d / %d\n", tested, NUM_KEYS);
    printf("  Working: %d\n", working);
    if (failed > 0) printf("  Failed : %d\n", failed);
    else           printf("  All working!\n");
    printf("Log written to session.log\n");

    return failed > 0 ? 1 : 0;
}
