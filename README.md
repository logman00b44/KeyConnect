# KeyConnect

**An open-source, 16-key mechanical macropad built around the CH32X035 RISC-V MCU.**

KeyConnect is a fully open-source, programmable 16-key macropad designed for
accessibility, cheap hand-assembly, and easy customization. It uses through-hole
parts wherever possible, a fully reprogrammable RISC-V microcontroller, and a
vendor-HID configuration protocol so the keymap can be remapped from your desktop
without any custom driver.

### Top layer

Component placement, signal routing, and switch layout.

![Top layer](mechanical/pcb/previews/top.svg)

### Bottom layer

Ground plane, power routing, and connector placement.

![Bottom layer](mechanical/pcb/previews/bottom.svg)

### Assembled board

Fully populated 3D view.

![Assembled board](mechanical/pcb/previews/Assembled_View.png)

## Features

- **16× Cherry MX switches** in a 4×4 grid
- **CH32X035G8R6 RISC-V microcontroller** (48 MHz) — fully reprogrammable over USB
- **USB Type-C** connectivity with ESD protection
- **I2C expansion** — add an LCD/OLED, RGB controller, or rotary encoder; the
  board can also act as an I2C peripheral
- **Remappable keymap** stored in flash, editable over HID feature reports
- **Mostly through-hole** — only 3 SMD parts (USB connector, regulator, MCU)
- **Low-cost build** — under \$20/unit with Gateron switches, under \$40 with genuine Cherry MX
- **2-layer PCB** with proper ground planes and **11 test points**

## Technical specifications

| | |
|---|---|
| Microcontroller | CH32X035G8R6 (RISC-V `rv32imac`, 48 MHz) |
| Power | USB-powered, 3.3 V regulated (AP2112K) |
| Connectivity | USB 2.0 Full Speed, I2C (master/peripheral) |
| Protection | TVS diodes, 500 mA fuse |
| PCB | 2-layer FR-4 |
| Dimensions | 134 mm × 135 mm × 1.6 mm |
| USB IDs | VID `0x2709` / PID `0x0001` (HID keyboard) |

## Repository layout

```
.
├── docs/            OSHWLab project write-up
├── firmware/        CH32X035 firmware + Windows host tools  (MIT)
│   ├── main.c
│   ├── driver/      keyconnect.inf, config protocol, CLI + WinUI 3 GUI
│   └── prebuilt/    known-good main.bin / main.elf / main.map
├── hardware/        PCB + schematic sources and CAD exports   (CC BY-SA 4.0)
│   ├── pcb/         PCB2_1.pcbdoc
│   ├── schematic/   schematic sheets
│   └── exports/     STEP / DXF / DWG / OBJ exports
└── mechanical/      3D enclosure + board previews             (CC BY-SA 4.0)
```

## Firmware

The firmware targets the CH32X035 and is built with
[ch32fun](https://github.com/cnlohr/ch32fun) and the RISC-V GCC toolchain.
A known-good `main.bin` / `main.elf` is committed under `firmware/prebuilt/`,
so you can flash a board without building anything.

Highlights: 16-key debounce, USB HID keyboard reports, flash-backed keymap,
a HID feature-report config protocol (Report IDs `0x01`–`0x04`), an optional
I2C LCD, and a `PB12` bootloader entry.

See **[`firmware/README.md`](firmware/README.md)** for build, flash, pinout, and
the full configuration protocol.

### Host configuration tool

`firmware/driver/` contains an INF for a proper Windows device name, a `hidapi`
CLI (`keyconnect-config`), and a **WinUI 3 desktop app** that shows the 4×4 grid,
edits mappings, and saves them to flash.

## Applications

- Programmable macro shortcuts for productivity
- Custom gaming keypad
- Video-editing hotkeys
- CAD / design software shortcuts
- MIDI controller for music production
- An educational platform for learning PCB design

## Status

| | |
|---|---|
| PCB design | ✅ Complete |
| Schematic | ✅ Verified |
| BOM (LCSC part numbers) | ✅ Optimized |
| Firmware | ✅ Source available (`firmware/`) |
| Host configurator | ✅ CLI + WinUI 3 GUI source |
| 3D case | ⏳ In development |

## Future ideas

The I2C expansion port makes add-ons straightforward: per-key RGB (WS2812B),
an OLED status display, a rotary encoder, alternative case variants, and
possibly a QMK/Vial port.

## License

KeyConnect is released under two licenses:

- **Firmware and host software** (`firmware/`) — **MIT** — see [`LICENSE`](LICENSE)
- **Hardware design files** (`hardware/`, `mechanical/`, `docs/`) — **CC BY-SA 4.0** — see [`LICENSE-hardware`](LICENSE-hardware)

## Author

**Logmanbay** — [@logman00b44](https://github.com/logman00b44)

Rendered project write-up: [`docs/OSHWLab_readme.md`](docs/OSHWLab_readme.md)
