# KeyConnect R1 — Firmware

Firmware for the KeyConnect R1 16-key macropad, running on a **CH32X035G8R6**
(RISC-V `rv32imac`, 48 MHz) using the [ch32fun](https://github.com/cnlohr/ch32fun)
framework.

- USB HID boot-compatible keyboard (8-byte reports, 6KRO)
- 16-key matrix scanner with per-key debounce
- Rebindable keymap stored in flash, editable over HID feature reports
- Optional I2C character LCD (PCF8574 backpack)
- Bootloader entry via the on-board DIP/BTN on `PB12`
- VID/PID: `0x2709` / `0x0001`

> **Note:** `AGENTS.md` in this directory is the development/agent context file.
> It documents design decisions and hardware gotchas — read it before changing USB
> or pin configuration.

## Source layout

| Path | Role |
|---|---|
| `main.c` | Key matrix scan, debounce, HID report sender, bootloader entry, feature-report callbacks |
| `usb_config.h` | USB/HID descriptors, string descriptors, FUSB compile-time configuration |
| `funconfig.h` | ch32fun build options (debug/UART printf disabled) |
| `fsusb.h` / `fsusb.c` | USB state struct + IRQ handling / EP management. **`fsusb.c` is `#include`d by `fsusb.h`** |
| `key_config.h` | Flash-backed keymap storage (magic/version/16 entries) |
| `i2c_lcd.h` / `i2c_lcd.c` | Bit-banged I2C + HD44780 over PCF8574. **`i2c_lcd.c` is `#include`d by `i2c_lcd.h`** |
| `Makefile` | ch32fun make wrapper (`TARGET_MCU=CH32X035`, package `CH32X035G8R6`) |
| `Netlist_Schematic1_1_2026-06-03.tel` | Schematic netlist used to derive the `KEY_PINS[]` mapping |
| `WHCISPCONFIG.INI` | WCH ISP flashing configuration |
| `prebuilt/` | Known-good `main.bin` / `main.elf` / `main.map` |
| `driver/` | Windows host tools + INF (see below) |

## Default keymap

Physical keys are labelled **A–P** in schematic order (left→right, top→bottom).
Indices match `KEY_PINS[]` in `main.c`:

| Key | Pin | Default action | Key | Pin | Default action |
|---|---|---|---|---|---|
| A | PA0 | Ctrl+Z (Undo) | I | PB0 | Ctrl+T (New tab) |
| B | PA1 | Ctrl+C (Copy) | J | PB1 | Ctrl+W (Close tab) |
| C | PA2 | Ctrl+X (Cut) | K | PB2 | Ctrl+Shift+T (Reopen) |
| D | PA3 | Ctrl+Y (Redo) | L | PB3 | Alt+Tab |
| E | PA4 | Ctrl+V (Paste) | M | PB4 | Print Screen |
| F | PA5 | Ctrl+S (Save) | N | PB6 | Ctrl+L (Address bar) |
| G | PA6 | Ctrl+A (Select all) | O | PB8 | F5 (Refresh) |
| H | PA7 | Ctrl+F (Find) | P | PB10 | Ctrl+P (Print) |

> ⚠️ **Hardware note:** the current R1 board routes `PA7` and `PB0` to the same
> net, so physical keys **H** and **I** (indices 7 and 8) read identically. This
> is a board revision issue, noted in `main.c`; it is not a firmware bug.

## Configuration protocol

The keymap can be read/written from a host over standard HID **feature
reports** — no custom driver required. Report IDs:

| Report ID | Direction | Purpose |
|---|---|---|
| `0x01` | SET + GET | Select and read a keymap entry (layer, key → mod, usage) |
| `0x02` | SET | Write a keymap entry (RAM) |
| `0x03` | SET | Commit RAM keymap to flash (magic `0xA5`) |
| `0x04` | GET | Read the 16-bit debounced matrix state |

Full byte layout: [`driver/firmware/config_protocol.md`](driver/firmware/config_protocol.md).

## Building

### Option 1 — flash a prebuilt binary

A known-good build is committed under [`prebuilt/`](prebuilt/):

```
prebuilt/main.bin
prebuilt/main.elf
prebuilt/main.map
```

Use `main.bin` (or `main.elf`) with your CH32 programmer — no toolchain needed.
See *Flashing* below.

### Option 2 — build from source

The firmware depends on **ch32fun** and is written to build from inside a
ch32fun checkout. ch32fun is **not vendored** in this repository.

1. Clone ch32fun and place this firmware where its `Makefile` expects it:

   ```console
   git clone https://github.com/cnlohr/ch32fun
   # copy (or symlink) this directory to:
   #   ch32fun/examples/Keyboard_Firmware
   ```

   `Makefile` does `include ../../ch32fun/ch32fun.mk`, so the `examples/Keyboard_Firmware`
   location matters.

2. Build with the RISC-V toolchain (PlatformIO package or
   `riscv-none-embed-gcc`):

   ```console
   cd ch32fun/examples/Keyboard_Firmware
   make
   ```

   `make` builds `main.elf`/`main.bin` and flashes via `cv_flash`.

3. Manual build (no `make`) using the PlatformIO RISC-V GCC, run from the
   **ch32fun repo root**:

   ```powershell
   $p = "$env:USERPROFILE\.platformio\packages\toolchain-riscv32-esp\bin"
   $out = "examples/Keyboard_Firmware"
   cmd /c """set PATH=$p;%PATH% && riscv32-esp-elf-gcc.exe -o $out/main.elf ch32fun/ch32fun.c examples/Keyboard_Firmware/main.c -g -Os -flto -ffunction-sections -fdata-sections -march=rv32imac -mabi=ilp32 -DCH32X03x=1 -I ch32fun -I extralibs -I examples/Keyboard_Firmware -nostdlib -Wall -Wextra -Wl,--gc-sections -lgcc -T ch32fun/generated_CH32X035G8R6_.ld -Wl,-Map=$out/main.map && riscv32-esp-elf-objcopy.exe -O binary $out/main.elf $out/main.bin"""
   ```

   `objcopy` must be re-run after every `gcc` invocation — `main.bin` is **not**
   regenerated automatically, and `fsusb.c` / `i2c_lcd.c` must **not** be added
   to the compile line (they are `#include`d by their headers).

## Flashing

- **WCH-Link / minichlink:** connect `SWDIO`/`SWCLK` (plus GND/3V3) and flash
  `prebuilt/main.elf`.
- **USB bootloader:** hold `PB12` low to GND (DIP switch) while powering the
  board, then use the WCH ISP tool with `WHCISPCONFIG.INI`.
- Disabling `SWJ` for I2C on `PC18`/`PC19` frees the debug pins but disables
  SWD until a power cycle — see `AGENTS.md`.

## Host tools (`driver/`)

Windows tooling for reading/writing the keymap and installing the display name.
See [`driver/host/README.md`](driver/host/README.md) for build details.

```
driver/
├── inf/
│   └── keyconnect.inf              Windows INF (display name for VID_2709&PID_0001)
├── firmware/
│   └── config_protocol.md          Vendor HID feature report spec
└── host/
    ├── keyconnect-config.c         CLI tool (hidapi)
    ├── keyconnect-dev-test.c       Device smoke test
    ├── keyconnect-dev-test.exe     Prebuilt smoke test
    ├── hidapi.dll                  Runtime dependency
    ├── KeyConnectConfig.sln        WinUI 3 GUI (VS 2022 + Windows App SDK)
    └── KeyConnectConfig/           GUI sources: 4×4 grid, key picker, matrix tester
```

The GUI app enumerates by VID `0x2709` / PID `0x0001`, shows the 4×4 grid,
edits mappings and commits them to flash.

## Status / known limitations

Implemented: matrix + debounce, HID keyboard, flash keymap storage, feature-report
config protocol, I2C LCD, bootloader entry.

Not done: signing the INF / an installer, and a hardware fix for the `PA7`/`PB0`
short described above.

## License

Firmware and host software in this directory are licensed under the **MIT
License** — see [`../LICENSE`](../LICENSE). Documentation and hardware design
files are **CC BY-SA 4.0** — see [`../LICENSE-hardware`](../LICENSE-hardware).
