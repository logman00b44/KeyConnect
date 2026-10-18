# Board previews

Images used by the project READMEs. They are **generated**, not hand-drawn — the
source of truth is [`hardware/gerbers/`](../../../hardware/gerbers) (2D) and
[`hardware/exports/3D_PCB2_1_2026-06-11.obj`](../../../hardware/exports) (3D).

| File | What it is |
|---|---|
| `top.svg` / `top.png` | Top copper + soldermask + silkscreen + drills |
| `bottom.svg` / `bottom.png` | Bottom copper + soldermask + silkscreen + drills |
| `Assembled_View.png` | 3D render of the populated board, top face |

## Regenerating the 2D previews

Rendered with [tracespace](https://github.com/tracespace/tracespace) straight
from the fabrication files — see
[`hardware/gerbers/README.md`](../../../hardware/gerbers/README.md) for the exact
command. Copy `board.top.svg` → `top.svg` and `board.bottom.svg` → `bottom.svg`.
The PNGs are rasterisations of those SVGs at 2400 px.

## Regenerating the 3D preview

`Assembled_View.png` is a Blender (Cycles, CPU) render of the EasyEDA OBJ export,
2400 × 1800:

1. Import `hardware/exports/3D_PCB2_1_2026-06-11.obj`.
2. Rotate the model so the board lies flat (Z up) with the **top face upward**.
3. Frame it with a 3/4 camera (~50 mm lens, ~2× the board diagonal) on a light
   grey background, one key light + two fill lights.
4. Render to PNG.
