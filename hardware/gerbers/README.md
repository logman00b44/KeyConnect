# Gerber & drill files — `PCB2_1`

Export from **EasyEDA Pro v3.2.206**, generated **2026-10-18**. These are the
fabrication files for the KeyConnect R1 Rev 0.2 board (135 mm × 135 mm,
2-layer FR-4, 1.6 mm).

## Layer map

| File | Layer | Notes |
|---|---|---|
| `Gerber_TopLayer.GTL` | Top copper | signal + pad routing |
| `Gerber_BottomLayer.GBL` | Bottom copper | ground plane + routing |
| `Gerber_TopSilkscreenLayer.GTO` | Top silkscreen | labels, artwork, logo |
| `Gerber_BottomSilkscreenLayer.GBO` | Bottom silkscreen | artwork (bottom face) |
| `Gerber_TopSolderMaskLayer.GTS` | Top solder mask | openings |
| `Gerber_BottomSolderMaskLayer.GBS` | Bottom solder mask | openings |
| `Gerber_TopPasteMaskLayer.GTP` | Top paste | SMD stencil (top) |
| `Gerber_BottomPasteMaskLayer.GBP` | Bottom paste | SMD stencil (bottom) |
| `Gerber_TopAssemblyLayer.GTA` | Top assembly | pick-and-place reference |
| `Gerber_BoardOutlineLayer.GKO` | Board outline | 135.000 mm × 135.000 mm, 3 mm corner radius |
| `Gerber_DocumentLayer.GDL` | Documentation | drawing frame |
| `Gerber_DrillDrawingLayer.GDD` | Drill drawing | hole chart |
| `Drill_PTH_Through.DRL` | Plated through-holes | switches, headers, THT parts |
| `Drill_PTH_Through_Via.DRL` | Plated vias | layer transitions |
| `Drill_NPTH_Through.DRL` | Non-plated holes | mounting holes |
| `FlyingProbeTesting.json` | Flying-probe test | net test-point program |
| `How-to-order-PCB.txt` | — | stack-up / ordering note from the export |

## Ordering a board

Upload the whole folder (or a `.zip` of it) to JLCPCB / PCBWay / OSH Park.
Recommended options: **2 layers, 1.6 mm, HASL or ENIG, 1 oz copper**. Nothing on
this board needs an impedance-controlled or advanced stack-up.

## Regenerating the previews

`mechanical/pcb/previews/` is generated from these files with
[tracespace](https://github.com/tracespace/tracespace):

```sh
npx @tracespace/cli -L -o ../mechanical/pcb/previews \
  Gerber_TopLayer.GTL Gerber_BottomLayer.GBL \
  Gerber_TopSilkscreenLayer.GTO Gerber_BottomSilkscreenLayer.GBO \
  Gerber_TopSolderMaskLayer.GTS Gerber_BottomSolderMaskLayer.GBS \
  Gerber_BoardOutlineLayer.GKO \
  Drill_PTH_Through.DRL Drill_NPTH_Through.DRL Drill_PTH_Through_Via.DRL
```

That writes `board.top.svg` / `board.bottom.svg`, which are committed as
`top.svg` / `bottom.svg` (plus PNG rasterisations `top.png` / `bottom.png`).
