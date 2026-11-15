---
name: step-to-stl
description: Use when the mechanical design changed and the printable STL files must be made again from a STEP file under CAD/ - exporting each part of a Fusion 360 STEP assembly to its own STL under STL/, in the orientation it is printed in, and proving the files are the CAD.
---

# From a STEP assembly to printable STLs

The owner designs in Fusion 360 and commits one STEP per assembly to `CAD/`. Each printed part is published as its own STL under `STL/<assembly>/`, lying the way it is printed. This skill redoes that export without Fusion. The README with the assembly pictures is the owner's own step (it needs Fusion's renders): do not write it unless asked.

Everything goes through one script, `tools/step_stl.py`, and one manifest per STL folder, `stl_manifest.json`.

## Set up once

```bash
uv venv venv --python 3.12
uv pip install --python venv/Scripts/python.exe -r requirements.txt
```

`requirements.txt` lists the dependencies and why each is there. One venv at the repo root, never a second one. `venv/` is ignored by git; `tools/*.py` is tracked through an exception in `.gitignore` (the repo ignores `*.py*`).

Run every command from the repo root with `venv/Scripts/python.exe`. Every command reads the STEP first; the 20 MB shield STEP takes about 20 seconds.

## The path

1. **See what changed.** Take the previous STEP out of git and compare. Parts are told apart by volume and surface area, so a moved part does not count as changed.

   ```bash
   git log --oneline -- CAD
   git show <commit>^:"CAD/A&D FX Shield.step" > <scratchpad>/old.step
   venv/Scripts/python.exe tools/step_stl.py diff <scratchpad>/old.step "CAD/A&D FX Shield v3.step"
   ```

2. **See what the STEP holds.** `tree` lists every component and every named body with its place in the assembly. Screws and inserts are hidden below 500 mm3.

   ```bash
   venv/Scripts/python.exe tools/step_stl.py tree "CAD/A&D FX Shield v3.step" --depth 2
   ```

   A body the owner drew straight into an assembly (not in a component of its own) shows as `[body]` under that assembly: `display_adapter`, `front_shutter_blank`. New printable parts often arrive this way, so read the `[body]` lines, not only the components. Bodies still called `Body1` are not listed: a part that matters has a name.

3. **Ask which printed parts are in scope** when the STEP holds more than the folder you were sent to. The shield STEP carries the whole OpenTrickler, a display case and electronics. **Never export the electronics** (display modules such as `fly-mini12684` and `Bigtreetech Mini12864`, the Pico expansion board, motors, servos, bearings): the owner models them only to check the fit. Printed cases for the electronics are parts like any other. Where the owner put things in November 2026:

   | Printed parts | Folder |
   |---|---|
   | The shield itself | `STL/A&D FX Shield` |
   | Display case halves and `display_adapter` | `STL/Electronics/Mini 12864 Case` |
   | Pico expansion board enclosure, top and bottom | `STL/Electronics/Controller` |
   | `front_shutter_blank` (for people who do not want the shutter gate) | `STL/OpenTrickler/ServoGate` |
   | `opentrickler_assembly_base`, `handle1`, `handle2` | left out |

   The owner moves published files between folders by hand. When a file has moved, move its manifest line to a manifest in the new folder and run `check` there; do not export it again.

4. **Learn the published orientation before you overwrite anything.** `check` compares each published STL with the CAD part and says which axis of the part points up in the file. Run it first, while the old files are still there:

   ```bash
   venv/Scripts/python.exe tools/step_stl.py check "STL/A&D FX Shield/stl_manifest.json"
   ```

   - `ok ... +Z up`: the file is this CAD part, and this is how the owner laid it.
   - `POSE`: the same part, lying differently from what the manifest says. The file is the owner's choice: correct the manifest.
   - `DIFFERS`: the part changed, or the file was already older than the CAD. To learn how a changed part used to lie, write a scratch manifest whose `step` is the old STEP and whose `out` is the published folder (absolute paths work), and check that.

5. **Set the orientation of each part** (`"up"` in the manifest: the axis of the part's own Fusion component that points up on the bed).

   - **Every part was designed to be printed one way.** A part that already has an STL keeps that orientation, also when its shape changed. Do not improve on it from overhang numbers: `front_discharger_mount` is published with its larger overhang down, on purpose.
   - A new part has no precedent. Rate it and look at it:

     ```bash
     venv/Scripts/python.exe tools/step_stl.py orient "CAD/A&D FX Shield v3.step" display_adapter --png <scratchpad>/orient
     ```

     The table gives, for each of the six sides: height, flat area on the bed, overhang area (faces leaning more than 45 degrees), level ceilings. The pictures show the part from above and from below: blue lies on the bed, red is overhang. Read them with the Read tool.
   - One side with no overhang and a large face on the bed is the answer (`mini_12864_display_bottom`: open side up; `mini_12864_display_btt_top`: the visible front face on the bed).
   - **When two sides are close, ask the owner**, with the two pictures. The owner's words: "If you have doubt you can check with me." `display_adapter` was such a case; the owner chose the plain face down.
   - The repo promises that no part needs support. Red in the picture is not a failure by itself: the weighing pan prints its coin pockets as bridges (see the shield README), and engraved text on a bed face shows as small level ceilings 0.4 mm up.

6. **Write the files, then prove them.**

   ```bash
   venv/Scripts/python.exe tools/step_stl.py export "STL/A&D FX Shield/stl_manifest.json" --png <scratchpad>/png
   venv/Scripts/python.exe tools/step_stl.py check  "STL/A&D FX Shield/stl_manifest.json"
   venv/Scripts/python.exe tools/step_stl.py slice  "STL/A&D FX Shield/stl_manifest.json"
   ```

   - `export` stops rather than write a bad file: the mesh must be a closed solid with the volume (0.1 %) and area (0.2 %) of the CAD part. It warns when nothing flat lies on the bed.
   - `check` must end in `PASS`.
   - `slice` runs Bambu Studio (installed at `C:\Program Files\Bambu Studio`) on every file with its stock X1C 0.20 mm profile and must end in `PASS`: the slicer took the file and raised no warning. The profile is a yardstick only; it leaves supports off, so it does not show that none are needed. That judgement comes from the pictures and from the owner.
   - `--only name.stl` limits `export` and `slice` to some files.

7. **Show the owner every part as it will print.** Send the pictures from `--png` (a contact sheet of all of them is easiest to judge). The owner decides by looking, and may change an orientation after seeing it.

8. **Leave the rest.** Commit only when asked. Say which README tables now miss a file (new STLs are not listed until the owner writes the assembly steps).

## The manifest

`STL/<folder>/stl_manifest.json`, one per folder that has files made this way:

```json
{"step": "CAD/A&D FX Shield v3.step",
 "out": "STL/A&D FX Shield",
 "parts": [{"stl": "scale_shield.stl", "part": "scale_shield", "up": "+Z"}]}
```

- `part` is the component or body name from `tree`. Fusion's version suffix (` v36`) is dropped, so a re-save does not break the name. When two different shapes share a name, write `parent/name`; the script says so.
- `stl` names are the published ones and do not always equal the CAD name: `cup_base` is `cup_base_7mm.stl`, `weighing_pan_with_coin` is `weighing_pan_27mm.stl`. Keep published names: READMEs and other people's links point at them.
- `up` is one of `+X -X +Y -Y +Z -Z`, in the part's own component frame. The file stands on z = 0, centred in x and y.
- A file in a folder without an entry was not made from this STEP: `powder_cup_body.stl` and `powder_cup_handle.stl` are not in the shield STEP. Leave such files alone and say so.

## What the script does that a plain export gets wrong

Each of these cost a wrong or broken file before it was found.

- **Own frame, not assembly pose.** A part is read in the coordinates of its own Fusion component. The display case sits tilted in the assembly; in assembly axes none of its faces is level, in its own frame it is square. `up` therefore does not change when the owner re-poses the assembly.
- **Absolute tessellation tolerance.** Facets stay within 0.01 mm of the surface and turn at most 0.1 rad (`LIN_DEFLECTION`, `ANG_DEFLECTION` at the top of the script). cadquery's own `tessellate()` takes the tolerance relative to the part size and came out several times coarser. The owner asked for high quality for printing; these values give about 3.4 times the triangles of the earlier Fusion exports (6.4 MB became 21.8 MB for the same 14 shield files). Halving both triples the size again (the shield alone becomes 15 MB) and moves the surface by 0.03 mm at most.
- **Faces OpenCascade leaves empty.** At a fine tolerance the mesher gives up on some narrow cone bands, such as a 0.2 mm chamfer round a hole, and silently leaves a hole in the mesh. The script meshes only those faces again, coarser, keeping the fine edges they share with their neighbours.
- **Welding.** Faces meet at corners a hair apart, which leaves pinholes; vertices closer than 0.00001 mm are joined. A larger weld radius collapses real small triangles and breaks the solid.
- **Body names.** The names of loose bodies are not in the STEP's assembly structure; they are read from the solid entities.

`export` may print `the CAD solid touches itself along N edges`: two faces of the part meet in a line with no thickness (`scale_base_adapter_ring`, at its two lugs). The file is closed and slices; tell the owner, it is a CAD detail.

## Traps on this machine

- `bambu-studio.exe` prints nothing to the terminal. Its answer is `result.json` in `--outputdir`; without that option it drops the file into the current folder. The `slice` command uses a temporary folder.
- A STEP is far too large to read as text. Use `tree`, or grep for `=PRODUCT(` (components) and `MANIFOLD_SOLID_BREP('` (body names).
- Fusion leaves a hidden body out of the STEP, also when its component is shown. When the owner names a part that `tree` does not list, grep the file for the name. If it is not there, it was not exported: say so with the grep result and the file's save time, and ask for a new export. Do not guess at a similar part.
  - The cause the owner found for `mini_12864_display_fly_top`: the display case is its own design, linked into the shield assembly, and a linked copy shows what its original shows. The body has to be made visible in the original design; un-hiding in the main assembly is not enough.
- Write pictures, old STEP files and other scratch output to the scratchpad, not into the repo.

## The November 2026 export, as a worked example

`CAD/A&D FX Shield v3.step` against the STEP it replaced (`git show c03454a^:"CAD/A&D FX Shield.step"`):

- Changed: `scale_shield`, `trickler_adapter_plate`, `front_discharger_mount`. Each kept its published orientation (`+Z`, `-Z`, `+Z`).
- Unchanged in the CAD, but the 2023 STL was older than the CAD: `scale_base_adapter_ring` (the CAD has two lug cuts the old file lacked).
- Unchanged and identical to the published file: the other ten. They were written again from the v3 STEP so the whole folder has one source and one quality.
- New, display: `display_adapter` (`-Z`, the owner's choice), `mini_12864_display_bottom` (`+Z`), `mini_12864_display_btt_top` (`-Z`), and, after a second export of the STEP, `mini_12864_display_fly_top` (`-Z`, front face on the bed like its BTT sibling). First written to the shield folder, then moved by the owner to `STL/Electronics/Mini 12864 Case`.
- New, controller: `pico_expansion_board_enclosure_side_mnt_bottom` (`+Z`) and `pico_expansion_board_enclosure_side_mnt_top` (`-Z`), both open side up, in `STL/Electronics/Controller`.
- New, OpenTrickler: `front_shutter_blank` (`+Z`) in `STL/OpenTrickler/ServoGate`.
- `CAD/OpenTrickler Display Case.step` is the display case on its own. Its three printed bodies were identical to the ones in the shield STEP (`diff --all`), so the manifest reads them from the shield STEP, which also holds `display_adapter`. If the two ever differ, ask the owner which one is current.
