# KeyConnect R1 — Agent Context

## Board & Toolchain

- **MCU**: CH32X035G8R6 (RISC-V rv32imac)
- **Toolchain**: `riscv32-esp-elf-gcc` (from PlatformIO packages)
- **No `make`** — manual compile commands
- **Flash method**: CH32V003FUN `$flash` or WCH-Link utility
- **Project files**: `ch32fun/examples/Keyboard_Firmware/` (this directory)
- **Build working directory**: the ch32fun repo root (contains `ch32fun/`, `extralibs/`)

## Build

Compile from the ch32fun repo root — this is the *run directory*
for include/lib paths, **not** where artifacts land.

Output files (`main.bin`, `main.elf`, `main.map`) go to `examples/Keyboard_Firmware/`.

```powershell
$p = "$env:USERPROFILE\.platformio\packages\toolchain-riscv32-esp\bin"
$out = "examples/Keyboard_Firmware"

cmd /c """set PATH=$p;%PATH% && riscv32-esp-elf-gcc.exe -o $out/main.elf ch32fun/ch32fun.c examples/Keyboard_Firmware/main.c -g -Os -flto -ffunction-sections -fdata-sections -march=rv32imac -mabi=ilp32 -DCH32X03x=1 -I ch32fun -I extralibs -I examples/Keyboard_Firmware -nostdlib -Wall -Wextra -Wl,--gc-sections -lgcc -T ch32fun/generated_CH32X035G8R6_.ld -Wl,-Map=$out/main.map && riscv32-esp-elf-objcopy.exe -O binary $out/main.elf $out/main.bin"""
```

The `&&` chain inside `cmd /c` ensures objcopy only runs on gcc success.
Always regenerate `main.bin` after rebuilding — it is not auto-generated.

## Firmware Source Files

| File | Role |
|---|---|
| `main.c` | Key matrix scanner, debounce, HID report sender, bootloader entry |
| `usb_config.h` | USB/HID descriptors, string descriptors, FUSB compile-time config |
| `funconfig.h` | ch32fun build options (printf disabled, HPE setting) |
| `fsusb.h` | USB state struct, API declarations — **also `#include "fsusb.c"`** at bottom |
| `fsusb.c` | USB IRQ handler, EP management, `FSUSBSetup()` — **NOT compiled separately** |

**Do NOT add `fsusb.c` to the compile command.** It is included via `fsusb.h`.

Include chain: `fsusb.h` → `usb_config.h` → `funconfig.h`. Do not reorder or restructure headers.

## Hardware

- **Key matrix**: 16 keys, GPIOs listed in `KEY_PINS[]` in `main.c` (all with internal pull-up)
- **Bootloader entry**: PB12 DIP switch to GND. Active-LOW (`!funDigitalRead(PB12)`). Internal pull-up enabled. No external pull-up trace on PB12.
- **No LEDs** on the board

## USB Config

| Field | Value |
|---|---|
| VID / PID | `0x2709` / `0x0001` |
| Manufacturer | `"KeyConnect"` |
| Product | `"KeyConnect R1 Rev 0.2 Dev"` |
| bcdUSB | `0x0200` |
| bMaxPacketSize0 | `64` |
| bcdHID | `0x0111` |
| Config total length | `34` bytes |
| EP1 mode | `1` (IN interrupt, 8 bytes, 10ms interval) |
| Power | 100 mA, bus-powered |

## FUSB Compile-Time Flags (usb_config.h)

| Flag | Value | Notes |
|---|---|---|
| `FUSB_CONFIG_EPS` | `2` | EP0 + EP1 only |
| `FUSB_HID_INTERFACES` | `1` | One HID interface |
| `FUSB_HID_USER_REPORTS` | `1` | Feature reports implemented — Report IDs 1–4, all five `HandleHidUser*` callbacks present |
| `FUSB_SUPPORTS_SLEEP` | `0` | Sleep/wakeup paths disabled |
| `FUSB_IO_PROFILE` | `0` | GPIO profiling disabled |
| `FUSB_EP1_MODE` | `1` | Enables EP1 TX descriptor |
| `FUSB_USE_HPE` | `FUNCONF_ENABLE_HPE` | Tied to funconfig.h — do not hardcode |

All five `HandleHidUser*` callbacks are implemented in `main.c` (see `driver/firmware/config_protocol.md`).
Keep the callbacks and this flag consistent — if you disable the flag, the feature reports disappear.

## HID Report Descriptor

- Standard boot-compatible keyboard: modifier byte + reserved byte + 6 keycodes
- Keycode range: `0x00–0x91` (Usage Min/Max in descriptor) — do not change this range
- **No LED output report** — the board has no LEDs
- Report size: 8 bytes total (matches EP1 packet size)

## Key Decisions (Do Not Revert)

1. **`FSUSBSetup()`** (not `USBFSSetup()`) — uses the example `fsusb` API for CH32X035, which handles `RCC->AHBPCENR` internally. The extralibs `USBFSSetup()` does NOT do this for CH32X03x.
2. **`FUSB_EP1_MODE 1`** in `usb_config.h` — enables EP1 TX descriptor
3. **`USBFS_Endp_DataUp`** implemented in `main.c` using `USBFS_GetEPBufferIfAvailable()` + `USBFS_SendEndpoint()`
4. No LED output section in report descriptor — hardware has no LEDs
5. **PB12 bootloader**: active-LOW, internal pull-up, DIP switch to GND
6. `USBFS_IRQHandler` in `fsusb.c` overrides the weak default in `ch32fun.c` — link order must keep `ch32fun.c` before `main.c` (it already does)

## Versioning Rule

On every meaningful functional change (new features, protocol changes — NOT bug fixes or trivial edits), bump in **two** places in `usb_config.h`:

1. **`bcdDevice`** — bytes 12–13 of `device_descriptor[]`. Currently `0x00, 0x02` (v2.00).
2. **`STR_PRODUCT`** — currently `u"KeyConnect R1 Rev 0.2 Dev"`. Update to match.

Keep both in sync.

## driver/ Directory Structure

```
driver/
├── inf/
│   └── keyconnect.inf        ← Windows INF for proper display name
├── host/
│   ├── keyconnect-config.c   ← Skeleton CLI tool (hidapi, config read/write/save)
│   └── README.md             ← Build/usage docs
└── firmware/
    └── config_protocol.md    ← Vendor HID feature report spec
```

The host tool uses feature reports (Report IDs 1–3) via hidapi — no custom driver needed.

## Unfinished Work (Do Not Implement Without Explicit Instruction)

1. Sign the INF / create a proper installer
2. Build/ship the `keyconnect-config` CLI (`keyconnect-dev-test.exe` is prebuilt under `driver/host/`)
3. Fix the `PA7`/`PB0` short on the next board revision — physical keys H and I currently read identically

## I2C LCD (bit-banged, PCF8574 backpack)

### Pin Mapping
| Signal | MCU Pin | GPIOC Register Notes |
|--------|---------|----------------------|
| SDA    | PC18    | CFGXR shift = (18-16)×4 = 8; BSXR bit 2 (set), bit 18 (clr) |
| SCL    | PC19    | CFGXR shift = (19-16)×4 = 12; BSXR bit 3 (set), bit 19 (clr) |

### I2C Address Probing
`lcd_init()` probes addresses: `0x20`, `0x27`, `0x38`, `0x3F` — first ACK wins.
With A0/A1/A2 pads empty (no solder bridge), PCF8574 internal pull-down → **0x20**,
PCF8574A internal pull-down → **0x38** (also probes bridged variants 0x27/0x3F).

### Mixed-Voltage I2C (3.3V MCU → 5V Backpack)
- **Must use open-drain output** (`GPIO_CFGLR_OUT_10Mhz_OD = 5`). Push-pull at 3.3V
  cannot meet the PCF8574's VIH min (0.7×VCC = 3.5V at 5V).
- CH32X035 GPIO pins **are 5V-tolerant** in open-drain mode — the backpack pull-ups
  to 5V safely pull SDA/SCL HIGH when the MCU releases them.
- The backpack typically has 4.7kΩ or 10kΩ pull-ups to 5V on SDA/SCL — no external
  resistors needed if the backpack provides them.

### SDI/SWD Pin Conflict
PC18 (SWDIO) and PC19 (SWCLK) default to the 2-wire serial debug interface after
reset. They **must** be freed for GPIO before they can drive I2C:

```c
AFIO->PCFR1 = (AFIO->PCFR1 & ~AFIO_PCFR1_SWJ_CFG) | AFIO_PCFR1_SWJ_CFG_DISABLE;
```

This disables SW-DP permanently for the current session. Power-cycle + hold reset
to re-enable SWD for flashing/debugging.

### Init Sequence Correctness
If I2C comms fail (no ACK from any probed address), `lcd_addr` stays 0 and all
`lcd_*()` calls become no-ops. The LCD will show the default power-on state
(all white squares — DD RAM random, display ON) — this is the diagnostic symptom
of **no I2C communication**.

If I2C works but init logic is wrong, you'd see partial or garbled characters
on a cleared background instead.

### Timing
- `i2c_delay()`: 5 µs → SCL ~100 kHz (within PCF8574 spec)
- EN pulse width: one full I2C byte transfer (~160 µs) >> 450 ns minimum
- Power-on wait: 50 ms > 15 ms HD44780 requirement

### Source Files
- `i2c_lcd.h` — public API, pin/address defines, command constants
- `i2c_lcd.c` — bit-banged I2C + PCF8574-to-HD44780 4-bit driver (included via header)

## Known Gotchas

- `funconfig.h` disables both debug and UART printf. Do not add printf-based debug without re-enabling the relevant flag.
- `FUSB_USE_HPE` reads from `FUNCONF_ENABLE_HPE` in `funconfig.h` — editing `funconfig.h` affects interrupt entry/exit mode.
- CH32X03x needs `RCC->AHBPCENR |= RCC_USBFS | RCC_AHBPeriph_DMA1` before USB setup. `FSUSBSetup()` handles this; `USBFSSetup()` does not.
- Removing LED output from report descriptor saved ~20 bytes of text section.
