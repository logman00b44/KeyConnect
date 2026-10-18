# Bill of materials — `PCB2_1`

Extracted from the EasyEDA Pro / JLCPCB SMT preview export for the
**2026-10-18** revision of the board.

- [`BOM_PCB2_1.csv`](BOM_PCB2_1.csv) — full table, including manufacturer,
  manufacturer part number, JLCPCB part class, and datasheet links.
- [`BOM_PCB2_1.md`](BOM_PCB2_1.md) — the same data as a Markdown table.
- [`InteractiveBOM_PCB2_1_2026-10-18.html`](InteractiveBOM_PCB2_1_2026-10-18.html)
  — the self-contained viewer exported from EasyEDA. Search, highlight by net,
  and use its 3D view; open it in a browser (GitHub will offer it as a download
  rather than rendering it).

## Summary

| Line item | Qty | Notes |
|---|---|---|
| Cherry MX switches | 16 | user-supplied, not SMD-assembled |
| CH32X035G8R6 | 1 | QSOP-28, RISC-V MCU |
| USB-C 16-pin receptacle | 1 | SHOU HAN `TYPE-C16PIN` |
| LM1117T-3.3 | 1 | TO-220 3.3 V LDO |
| SMBJ5.0A | 1 | SMB TVS diode (USB ESD) |
| 500 mA / 24 V fuse | 1 | 1206 SMD |
| 47 µF tantalum | 2 | SMD, CASE-C 6032 |
| 100 nF ceramic | 4 | through-hole |
| Resistors | 7 | 22 Ω ×2, 4.7 kΩ ×2, 5.1 kΩ ×2, 10 kΩ ×1 (all THT) |
| DSWB03LHGET | 1 | 3-position slide switch (mode/firmware) |
| XH 2.54 mm 4-pin header | 1 | I²C expansion |
| Test points | 11 | on the bottom face |

**47 placements total**, of which **6 are SMD** (MCU, USB-C, TVS diode, fuse
and the two 47 µF bulk capacitors). Everything else is through-hole and can be
hand-soldered.

> The Cherry MX switches (footprint `CherryMX_9.75u`) are deliberately not part
> of the SMT assembly — fit your own Gateron/Cherry/Kailh switches.
