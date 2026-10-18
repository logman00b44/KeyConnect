#include "ch32fun.h"
#include "fsusb.h"
#include "key_config.h"
#include "i2c_lcd.h"

/*
 * KEY_PINS layout — derived from netlist Netlist_Schematic1_1_2026-06-03.tel
 *
 * The 4×4 key grid is labelled A–P in schematic order (left→right, top→bottom):
 *
 *   A(U1)  B(U2)  C(U3)  D(U4)   → PA0  PA1  PA2  PA3
 *   E(U6)  F(U7)  G(U8)  H(U9)   → PA4  PA5  PA6  PA7
 *   I(U10) J(U11) K(U12) L(U13)  → PB0  PB1  PB3  PB4
 *   M(U14) N(U15) O(U16) P(U17)  → PB6  PB7  PB10 PB11
 *
 * Net → U5 pin number (from schematic image):
 *   A=pin9, B=pin10, C=pin11, D=pin12
 *   E=pin15, F=pin16, G=pin13, H=pin17
 *   I=pin14, J=pin20, K=pin18, L=pin19
 *   M=pin21, N=pin22, O=pin25, P=pin26
 */

typedef struct {
    GPIO_TypeDef *port;
    uint8_t       pin;
} KeyPin;

static const KeyPin KEY_PINS[16] = {
    { GPIOA, 0  },   /* [0]  A – PA0  – U1  */
    { GPIOA, 1  },   /* [1]  B – PA1  – U2  */
    { GPIOA, 2  },   /* [2]  C – PA2  – U3  */
    { GPIOA, 3  },   /* [3]  D – PA3  – U4  */
    { GPIOA, 4  },   /* [4]  E – PA4  – U6  */
    { GPIOA, 5  },   /* [5]  F – PA5  – U7  */
    { GPIOA, 6  },   /* [6]  G – PA6  – U8  */
    { GPIOA, 7  },   /* [7]  H – PA7  – U9  ⚠ shorted to PB0 */
    { GPIOB, 0  },   /* [8]  I – PB0  – U10 ⚠ shorted to PA7 */
    { GPIOB, 1  },   /* [9]  J – PB1  – U11 */
    { GPIOB, 2  },   /* [10] K – PB2  – U12 */
    { GPIOB, 3  },   /* [11] L – PB3  – U13 */
    { GPIOB, 4  },   /* [12] M – PB4  – U14 */
    { GPIOB, 6  },   /* [13] N – PB6  – U15 */
    { GPIOB, 8  },   /* [14] O – PB8  – U16 */
    { GPIOB, 10 },   /* [15] P – PB10 – U17 */
};

#define KEY_NONE       0x00
#define KEY_A          0x04
#define KEY_C          0x06
#define KEY_F          0x09
#define KEY_L          0x0F
#define KEY_P          0x13
#define KEY_S          0x16
#define KEY_T          0x17
#define KEY_V          0x19
#define KEY_W          0x1A
#define KEY_X          0x1B
#define KEY_Y          0x1C
#define KEY_Z          0x1D
#define KEY_F5         0x3E
#define KEY_PRTSCR     0x46

#define MOD_NONE       0x00
#define MOD_LCTRL      0x01
#define MOD_LSHIFT     0x02
#define MOD_LALT       0x04

/*
 * KEY_MAP[i] maps physical key index i (same order as KEY_PINS) to a
 * USB HID modifier+keycode pair.
 */
static const Action KEY_MAP[16] = {
    /* [0]  A */ { MOD_LCTRL,            KEY_Z     },  /* Ctrl+Z  Undo        */
    /* [1]  B */ { MOD_LCTRL,            KEY_C     },  /* Ctrl+C  Copy        */
    /* [2]  C */ { MOD_LCTRL,            KEY_X     },  /* Ctrl+X  Cut         */
    /* [3]  D */ { MOD_LCTRL,            KEY_Y     },  /* Ctrl+Y  Redo        */
    /* [4]  E */ { MOD_LCTRL,            KEY_V     },  /* Ctrl+V  Paste       */
    /* [5]  F */ { MOD_LCTRL,            KEY_S     },  /* Ctrl+S  Save        */
    /* [6]  G */ { MOD_LCTRL,            KEY_A     },  /* Ctrl+A  Select all  */
    /* [7]  H */ { MOD_LCTRL,            KEY_F     },  /* Ctrl+F  Find        */
    /* [8]  I */ { MOD_LCTRL,            KEY_T     },  /* Ctrl+T  New tab     */
    /* [9]  J */ { MOD_LCTRL,            KEY_W     },  /* Ctrl+W  Close tab   */
    /* [10] K */ { MOD_LCTRL|MOD_LSHIFT, KEY_T     },  /* Ctrl+Shift+T Reopen */
    /* [11] L */ { MOD_LALT,             0x2B      },  /* Alt+Tab             */
    /* [12] M */ { MOD_NONE,             KEY_PRTSCR},  /* Print Screen        */
    /* [13] N */ { MOD_LCTRL,            KEY_L     },  /* Ctrl+L  Address bar */
    /* [14] O */ { MOD_NONE,             KEY_F5    },  /* F5      Refresh     */
    /* [15] P */ { MOD_LCTRL,            KEY_P     },  /* Ctrl+P  Print       */
};

static uint8_t  read_cursor_layer;
static uint8_t  read_cursor_key;
static KeyConfig current_config;
static volatile uint8_t save_requested;

#define DEBOUNCE_MS 16
#define NUM_KEYS    16

static uint32_t last_change[NUM_KEYS];
static uint8_t  stable_state[NUM_KEYS];
static uint8_t  prev_stable[NUM_KEYS];

static uint8_t read_raw(int i)
{
    uint8_t bit = KEY_PINS[i].pin & 0x0F;
    /* Keys are active-low (pull-up + switch to GND); invert so 1 = pressed */
    return !((KEY_PINS[i].port->INDR >> bit) & 1);
}

/*
 * Debounce: accept a state change only after the raw reading has been
 * stable for DEBOUNCE_MS consecutive milliseconds.
 * Returns 1 on the tick where the key transitions low→pressed.
 */
static uint8_t debounce_tick(int i, uint32_t now_ms)
{
    uint8_t raw = read_raw(i);

    if (raw != stable_state[i]) {
        if (now_ms - last_change[i] >= DEBOUNCE_MS) {
            stable_state[i] = raw;
        }
    } else {
        last_change[i] = now_ms;
    }

    uint8_t pressed = (stable_state[i] && !prev_stable[i]);
    prev_stable[i] = stable_state[i];
    return pressed;
}

static void keys_init(void)
{
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB;

    for (int i = 0; i < NUM_KEYS; i++) {
        /*
         * CFGLR covers pins 0–7, CFGHR covers pins 8–15.
         * idx selects which register; shift selects the 4-bit field within it.
         */
        uint32_t mode_reg = (uint32_t)(&KEY_PINS[i].port->CFGLR);
        uint32_t shift    = (KEY_PINS[i].pin & 0x07) * 4;
        uint32_t idx      = (KEY_PINS[i].pin & 0x08) ? 1 : 0;
        *((__IO uint32_t *)mode_reg + idx) &= ~(0x0F << shift);
        *((__IO uint32_t *)mode_reg + idx) |=  (GPIO_CNF_IN_PUPD << shift);
        /* Drive OUTDR high to enable internal pull-up */
        KEY_PINS[i].port->OUTDR |= (1u << (KEY_PINS[i].pin & 0x0F));
    }

    /* PB12 — firmware-mode / bootloader button (not in KEY_PINS) */
    {
        uint32_t mode_reg = (uint32_t)(&GPIOB->CFGLR);
        uint32_t shift    = (12 & 0x07) * 4;   /* pin 12 & 7 = 4, shift = 16 */
        uint32_t idx      = (12 & 0x08) ? 1 : 0; /* idx = 1 → CFGHR */
        *((__IO uint32_t *)mode_reg + idx) &= ~(0x0F << shift);
        *((__IO uint32_t *)mode_reg + idx) |=  (GPIO_CNF_IN_PUPD << shift);
        GPIOB->OUTDR |= (1u << 12);
    }
}

void enter_bootloader(void)
{
    __disable_irq();
    SysTick->CTLR = 0;

    FLASH->BOOT_MODEKEYR = FLASH_KEY1;
    FLASH->BOOT_MODEKEYR = FLASH_KEY2;
    FLASH->STATR = (uint32_t)0x4000;
    RCC->RSTSCKR |= RCC_RMVF;
    PFIC->CFGR = NVIC_KEY3 | (1 << 7);

    while(1);
}

/* Wait up to ~50 ms for the endpoint buffer to become available, then send. */
static uint8_t ep_send_wait(uint8_t endp, const uint8_t *pbuf, uint16_t len)
{
    uint32_t timeout = 50;
    uint8_t *buf = NULL;
    while (timeout--) {
        buf = USBFS_GetEPBufferIfAvailable(endp);
        if (buf) break;
        Delay_Ms(1);
    }
    if (!buf) return 1;
    for (uint16_t i = 0; i < len; i++)
        buf[i] = pbuf[i];
    USBFS_SendEndpoint(endp, len);
    return 0;
}

uint8_t USBFS_Endp_DataUp(uint8_t endp, const uint8_t *pbuf, uint16_t len, uint8_t mod)
{
    (void)mod;
    uint8_t *buf = USBFS_GetEPBufferIfAvailable(endp);
    if (!buf) return 1;
    for (uint16_t i = 0; i < len; i++)
        buf[i] = pbuf[i];
    USBFS_SendEndpoint(endp, len);
    return 0;
}

int HandleHidUserGetReportSetup(struct _USBState *ctx, tusb_control_request_t *req)
{
    (void)ctx;
    uint8_t rid = req->wValue & 0xFF;
    if (rid == 1) return 5;
    if (rid == 4) return 3;
    return -1;
}

int HandleHidUserSetReportSetup(struct _USBState *ctx, tusb_control_request_t *req)
{
    (void)ctx;
    uint8_t rid = req->wValue & 0xFF;
    if (rid >= 1 && rid <= 3) return req->wLength;
    return -1;
}

void HandleHidUserReportDataOut(struct _USBState *ctx, uint8_t *data, int len)
{
    (void)ctx;
    uint8_t rid = FSUSBCTX.USBFS_IndexValue & 0xFF;

    if (rid == 1 && len >= 3) {
        read_cursor_layer = data[1];
        read_cursor_key = data[2];
    } else if (rid == 2 && len >= 5) {
        uint8_t layer = data[1];
        uint8_t key_idx = data[2];
        if (layer == 0 && key_idx < CONFIG_KEYS) {
            current_config.map[key_idx].mod = data[3];
            current_config.map[key_idx].key = data[4];
        }
    } else if (rid == 3 && len >= 2 && data[1] == 0xA5) {
        save_requested = 1;
    }
}

int HandleHidUserReportDataIn(struct _USBState *ctx, uint8_t *data, int len)
{
    (void)ctx;
    (void)len;
    uint8_t rid = FSUSBCTX.USBFS_IndexValue & 0xFF;

    if (rid == 1) {
        data[0] = 1;
        data[1] = read_cursor_layer;
        data[2] = read_cursor_key;
        if (read_cursor_layer == 0 && read_cursor_key < CONFIG_KEYS) {
            data[3] = current_config.map[read_cursor_key].mod;
            data[4] = current_config.map[read_cursor_key].key;
        } else {
            data[3] = 0;
            data[4] = 0;
        }
        return 5;
    }

    if (rid == 4) {
        uint16_t mask = 0;
        for (int i = 0; i < CONFIG_KEYS; i++) {
            if (stable_state[i]) mask |= (1u << i);
        }
        data[0] = 4;
        data[1] = (uint8_t)(mask & 0xFF);
        data[2] = (uint8_t)((mask >> 8) & 0xFF);
        return 3;
    }

    return 0;
}

void HandleHidUserReportOutComplete(struct _USBState *ctx)
{
    (void)ctx;
}

int main(void)
{
    SystemInit();
    funGpioInitAll();

    keys_init();
    FSUSBSetup();

    config_load(&current_config, KEY_MAP);
    save_requested = 0;

    lcd_init();
    lcd_str("KeyConnect R1");
    lcd_set_cursor(0, 1);
    lcd_str("Ready");

    Delay_Ms(500);

    uint32_t ms = 0;

    while (1) {
        Delay_Ms(1);
        ms++;

        /* PB12 held low for >50 ms → enter USB bootloader */
        if (!funDigitalRead(PB12)) {
            Delay_Ms(50);
            if (!funDigitalRead(PB12)) {
                enter_bootloader();
            }
        }

        if (save_requested) {
            save_requested = 0;
            config_save(&current_config);
        }

        for (int i = 0; i < NUM_KEYS; i++) {
            if (debounce_tick(i, ms)) {
                Action *a = &current_config.map[i];
                if (a->key != KEY_NONE) {
                    uint8_t buf[8] = { a->mod, 0, a->key, 0, 0, 0, 0, 0 };
                    ep_send_wait(1, buf, 8);
                    Delay_Ms(10);
                    uint8_t rel[8] = { 0 };
                    ep_send_wait(1, rel, 8);
                    Delay_Ms(10);
                }
            }
        }
    }
}