# Board previews

Images used by the project READMEs. They are **generated, never hand-drawn** — the
single source of truth is the fabrication data in
[`hardware/gerbers/`](../../../hardware/gerbers).

| File | What it is |
|---|---|
| `top.svg` / `top.png` | Top side: soldermask, copper under the mask, exposed pads, silkscreen, drills |
| `bottom.svg` / `bottom.png` | Bottom side, same layer stack |
| `Assembled_View.png` | 3D render of the populated board (top 3/4 view) |
| `Assembled_View_Bottom.png` | 3D render of the underside, seen from below |

Both 2D views are drawn in the Gerber coordinate system (X to the right, Y up,
origin at the board's bottom-left corner), so the two images can be overlaid
directly. That also means bottom-side text reads mirrored in the flat views — the
same way a fab viewer or the interactive BOM shows it. Flip the board over and it
reads correctly, which is what `Assembled_View_Bottom.png` shows.

## The layer stack

Rather than rendering the copper as a bright "technical" colour, the previews look
like the finished board:

1. board outline — a 135 mm × 135 mm rounded rectangle (3 mm corner radius),
   generated from the numbers in `Gerber_BoardOutlineLayer.GKO`, used as a clip;
2. soldermask over bare laminate (background);
3. copper under the mask (`Gerber_TopLayer.GTL` / `Gerber_BottomLayer.GBL`),
   slightly lighter than the laminate;
4. silkscreen (`Gerber_TopSilkscreenLayer.GTO` / `Gerber_BottomSilkscreenLayer.GBO`);
5. mask **openings** (`Gerber_TopSolderMaskLayer.GTS` / `...GBS`) painted as exposed
   pad finish, on top of everything (real mask openings have no silk in them);
6. drill hits (`Drill_*.DRL`) as dark holes.

## Regenerating the 2D previews

Each layer is converted to SVG with
[tracespace](https://github.com/tracespace/tracespace), then all layers are
re-anchored into one shared coordinate system, clipped to the board outline and
composited:

```sh
npx @tracespace/cli@4.2.8 -B -o layers \
  hardware/gerbers/Gerber_TopLayer.GTL \
  hardware/gerbers/Gerber_TopSolderMaskLayer.GTS \
  hardware/gerbers/Gerber_TopSilkscreenLayer.GTO \
  hardware/gerbers/Gerber_BottomLayer.GBL \
  hardware/gerbers/Gerber_BottomSolderMaskLayer.GBS \
  hardware/gerbers/Gerber_BottomSilkscreenLayer.GBO \
  hardware/gerbers/Gerber_BoardOutlineLayer.GKO \
  hardware/gerbers/Drill_PTH_Through.DRL \
  hardware/gerbers/Drill_PTH_Through_Via.DRL \
  hardware/gerbers/Drill_NPTH_Through.DRL
```

Each tracespace SVG wraps its artwork in `<g transform="translate(0,-T) scale(1,-1)">`.
Wrapping that group in `<g transform="translate(0,T)">` cancels the per-layer
offset, so every layer lands in the same space. The composite is then rasterised
with [sharp](https://sharp.pixelplumbing.com/) (`top.svg`/`bottom.svg` →
`top.png`/`bottom.png`, 3000 px).

## Regenerating the 3D previews

`Assembled_View.png` is a Blender 5.2 / Cycles (CPU, 128 samples) render:

1. Build a 135 mm × 135 mm × 1.6 mm board slab with 3 mm rounded corners.
2. Texture its top and bottom faces with `top.png` / `bottom.png`, with UVs
   assigned from world XY so a texel at (x, y) lands on the model at (x, y) —
   the STEP export shares the Gerber coordinate frame, so nothing needs aligning.
   A third material colours the FR-4 edge.
3. Convert the [3D STEP model](../../../hardware/exports) to OBJ with FreeCAD
   (`Mesh.export` / `Import.export` in `freecadcmd`) and import it. The model's
   own board solid is left out — only the 36 component bodies are used, so the
   gerber-derived board is what you see.
4. Light it with a key, a fill and a rim area light plus a soft world, film
   transparent, `Standard` view transform, and frame each view automatically
   (~62 mm lens, camera distance solved so the content fits the frame).
5. Re-colour two materials by name: the switch housings (`switch`) are rendered
   translucent (alpha 0.35) so the silkscreen underneath stays readable, and the
   DIP/slide switch (`slide`) is solid red. Nothing else is touched — the board
   artwork still comes only from the Gerbers.

The published renders are 1280 px wide so the READMEs stay quick to load.

The lighting is deliberately soft: the point is to show the artwork, not to
blow out the highlights.

## A note on the hatched squares

Every switch position in the top view carries a white square filled with diagonal
hatching. That is **real silkscreen**, straight out of the `CherryMX_9.75u`
footprint in the Gerber (`Gerber_TopSilkscreenLayer.GTO`, 250 `G36` regions, no
`GKO` cut-outs anywhere on the board) — it is the footprint's switch-body outline.
It gets printed on the real board exactly as drawn, so if it is not wanted it has
to be removed in the footprint itself. In `Assembled_View.png` the translucent
switch housings let it show through.

## A note on the gold wordmark

The `KeyConnect` wordmark on the top side renders gold, not white. That is
correct: it is a **soldermask opening** (`Gerber_TopSolderMaskLayer.GTS`), i.e.
exposed copper, not silkscreen. The mask is painted last in the composite for
exactly this reason — a real board has no ink inside a mask opening.
