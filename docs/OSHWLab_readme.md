# KeyConnect - Open-Source 16-Key Mechanical Macropad

## Overview
KeyConnect is a fully open-source, programmable 16-key mechanical macropad designed for maximum accessibility and customization. Built with through-hole components for easy hand assembly and designed to be cheap as possible.

## Key Features
- **16x Cherry MX switches** in 4×4 grid layout
- **CH32X035G8R6 RISC-V microcontroller** - Fully reprogrammable via USB
- **USB Type-C connectivity** with ESD protection
- **I2C expansion port** - Add OLED displays, RGB controllers, rotary encoders,can act as I2C peripheral.
- **Maximum THT components** - Easy hand soldering for beginners
- **Low Cost Manufacturing** - DIY build for under \$20/unit with quality Gateron switches; genuine Cherry MX option under \$40.
- **Professional PCB design** - 2-layer board with proper ground planes
- **11 test points** - Easy debugging and development

## Technical Specifications
- **Microcontroller:** CH32X035G8R6 (RISC-V, 48MHz)
- **Power:** USB-powered, 3.3V regulated (LM1117T-3.3, TO-220)
- **Connectivity:** USB 2.0 Full Speed, I2C peripheral/master
- **Protection:** TVS diodes, 500mA fuse
- **PCB:** 2-layer, FR-4
- **Dimensions:** 135mm × 135mm × 1.6mm

## PCB Design Views

### Top Layer
Shows component placement, signal routing, and switch layout.
![Top Layer View](https://raw.githubusercontent.com/logman00b44/KeyConnect/refs/heads/main/mechanical/pcb/previews/top.svg)

### Bottom Layer
Displays ground plane, power routing, and connector placement.
![Bottom Layer View](https://raw.githubusercontent.com/logman00b44/KeyConnect/refs/heads/main/mechanical/pcb/previews/bottom.svg)

### 3D Assembled View
Complete board visualization with all components populated, rebuilt from the Gerber artwork.
![3D Assembled PCB](https://raw.githubusercontent.com/logman00b44/KeyConnect/refs/heads/main/mechanical/pcb/previews/Assembled_View.png)

Seen from below (the MCU and test points are mounted on this side):
![3D Assembled PCB, underside](https://raw.githubusercontent.com/logman00b44/KeyConnect/refs/heads/main/mechanical/pcb/previews/Assembled_View_Bottom.png)

## Applications
- ⌨️ Programmable macro shortcuts for productivity
- 🎮 Custom gaming keypad
- 🎥 Video editing hotkeys
- 🎨 CAD/design software shortcuts
- 🎵 MIDI controller for music production
- 📚 Educational platform for learning PCB design


**Major components:**
- 16× Cherry MX switches
- 1× CH32X035G8R6 RISC-V microcontroller (QSOP-28, bottom side)
- 1× USB Type-C connector
- 1× LM1117T-3.3 regulator (TO-220) + SMBJ5.0A TVS diode + 500mA fuse
- 1× DSWB03LHGET mode slide switch
- 1× XH 2.54mm 4-pin I2C expansion header
- 11× test points
- Custom 3D-printed case

## Assembly Difficulty
🔧 **Beginner-Friendly** - All components are through-hole except 6 SMD parts (MCU, USB-C connector, TVS diode, fuse, and two bulk capacitors). Perfect for learning PCB assembly!

## Current Development Status
✅ PCB design complete  
✅ Schematic verified  
✅ BOM optimized with LCSC part numbers  
⏳ 3D case design in progress  
✅ Firmware source available in [`firmware/`](../firmware)  

## Open-Source Commitment
All project files are released under permissive licenses following industry best practices:
- **Hardware (PCB, schematics, 3D models):** CC BY-SA 4.0
- **Software (firmware):** MIT License

This ensures the maker community can learn from, modify, and improve KeyConnect while maintaining proper attribution.

## Complete Documentation Package
- ✅ Complete schematics (EasyEDA format) — [`hardware/schematic/`](../hardware/schematic)
- ✅ PCB layout files with Gerber exports — [`hardware/gerbers/`](../hardware/gerbers)
- ✅ Bill of Materials with LCSC part numbers — [`hardware/bom/BOM_PCB2_1.csv`](../hardware/bom/BOM_PCB2_1.csv)
- ✅ Interactive BOM viewer (with 3D view) — [`hardware/bom/InteractiveBOM_PCB2_1_2026-10-18.html`](../hardware/bom/InteractiveBOM_PCB2_1_2026-10-18.html)
- ✅ Exported netlist — [`hardware/schematic/Netlist_PCB2_1_2026-10-18.tel`](../hardware/schematic/Netlist_PCB2_1_2026-10-18.tel)
- ✅ 3D model (STEP assembly) — [`hardware/exports/`](../hardware/exports)
- ⏳ 3D case STL files (coming soon)
- ✅ Firmware source code — [`firmware/`](../firmware)
- ⏳ Assembly instructions with photos
- ✅ Programming guide and host configuration tool (hidapi CLI + WinUI 3 GUI)

## Future Enhancement Possibilities
The modular design and I2C expansion port enable exciting add-ons:
- 🌈 Per-key RGB LED support (WS2812B)
- 📺 OLED display for macro names and status
- 🎛️ Rotary encoder for volume/scrolling control
- 🎨 Multiple case design variants (low-profile, high-profile, angled)
- ⌨️ QMK/VIAL firmware port for advanced customization

## 3D Enclosure
### ⚠ Still In Development ⚠

![3D Enclosure Top](https://raw.githubusercontent.com/logman00b44/KeyConnect/refs/heads/main/mechanical/3D%20Enclosure/preview/3denclosure_top%20view.png)  

![3D Enclosure Side](https://raw.githubusercontent.com/logman00b44/KeyConnect/refs/heads/main/mechanical/3D%20Enclosure/preview/3denclosure_side%20view.png)   


## Connect With Me
**GitHub:** [@logman00b44](https://github.com/logman00b44)  
**Email:** logmanbay@gmail.com  
**Project Repository:** [KeyConnect on GitHub](https://github.com/logman00b44/KeyConnect)

---

**📄 License:** Hardware CC BY-SA 4.0 | Software MIT License  
**👤 Author:** Logmanbay  
**🔢 Version:** 1.0  
**🚀 Status:** Active Development  
**⭐ Project Type:** Open-Source Hardware | Consumer Electronics | DIY-Friendly