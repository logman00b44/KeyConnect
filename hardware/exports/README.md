# 3D exports

| File | What it is |
|---|---|
| `3D_PCB2_1_2026-10-18.step` | EasyEDA Pro 3D export of the **populated board** (revision 2026-10-18) |
| `shapr3d_export_2026-03-02_20h16m.step` | enclosure, early revision |
| `shapr3d_export_2026-03-02_20h28m.dwg` / `.dxf` / `.obj` | enclosure, early revision (2D + mesh) |
| `shapr3d_export_2026-03-15_00h14m.step` | enclosure, later revision |

## About the board STEP

`3D_PCB2_1_2026-10-18.step` contains a single assembly, `EasyEDA PCB Model`,
with 38 objects:

- `Board` — the bare 135 mm × 135 mm × 1.6 mm PCB,
- 36 component bodies — 16 Cherry MX switches (`U1`–`U4`, `U6`–`U17`), the
  QSOP-28 MCU (`U5`), the USB-C receptacle, the TO-220 regulator (`U19`), the
  slide switch, the TVS diode, the fuse, 6 capacitors, 7 resistors and the XH
  header,
- one origin/datum object.

The 11 test points have no 3D body, so 36 of the 47 placed parts are modelled.

Coordinates are the same as in the Gerbers: **X 0 → 135 mm, Y 0 → −135 mm,
Z up**, with the board occupying Z 0 → 1.6 mm. That means the fabrication artwork
in [`../gerbers/`](../gerbers) lines up with the model with **no offset math**, 
which is how [`../../mechanical/pcb/previews/`](../../mechanical/pcb/previews)
builds its renders: the model supplies the components, the Gerbers supply the
board surfaces.

Switches sit on the top face; the MCU and the test points are on the bottom face,
so the switches' pins and the MCU stick out on the −Z side of the board.
