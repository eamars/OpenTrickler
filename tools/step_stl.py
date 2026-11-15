"""Export the printable parts of a STEP assembly to STL, each in its print orientation.

Run with the repo venv from the repo root (see requirements.txt):

  venv/Scripts/python.exe tools/step_stl.py tree    "CAD/A&D FX Shield v3.step" [--depth 2]
  venv/Scripts/python.exe tools/step_stl.py diff    OLD.step NEW.step
  venv/Scripts/python.exe tools/step_stl.py orient  STEP PART [PART ...] [--png DIR]
  venv/Scripts/python.exe tools/step_stl.py check   MANIFEST.json
  venv/Scripts/python.exe tools/step_stl.py export  MANIFEST.json [--only STL ...] [--png DIR]
  venv/Scripts/python.exe tools/step_stl.py slice   MANIFEST.json [--only STL ...]

A part is named as the STEP names it: a component ("scale_shield") or a body inside a component
("display_adapter"). Fusion's version suffix (" v12") is dropped, so a name survives a re-save.
Everything is in millimetres. A part is read in the coordinates of its own Fusion component (a loose
body: of the assembly that holds it), not as it is posed in the top assembly.

The manifest lists, for every STL, the part and which axis of its own component points up on the print bed:

  {"step": "CAD/x.step", "out": "STL/x",
   "parts": [{"stl": "scale_shield.stl", "part": "scale_shield", "up": "-Z"}, ...]}
"""
import argparse
import json
import os
import re
import sys

import numpy as np
import trimesh

LIN_DEFLECTION = 0.01   # mm: how far a facet may sit from the true surface
ANG_DEFLECTION = 0.1    # rad: how far a facet may turn from its neighbour
WELD = 1e-5            # mm: vertices closer than this are one vertex
OVERHANG_NZ = -0.72    # a facet whose normal points further down than this needs a look (a 45 deg chamfer is -0.707)

UP = {  # rotation that brings the named axis of the part to +Z (the print bed normal)
    "+Z": np.eye(3),
    "-Z": np.diag([1.0, -1.0, -1.0]),
    "+X": np.array([[0, 0, -1], [0, 1, 0], [1, 0, 0]], float),
    "-X": np.array([[0, 0, 1], [0, 1, 0], [-1, 0, 0]], float),
    "+Y": np.array([[1, 0, 0], [0, 0, -1], [0, 1, 0]], float),
    "-Y": np.array([[1, 0, 0], [0, 0, 1], [0, -1, 0]], float),
}


# ---------------------------------------------------------------- reading the STEP

def _clean(name):
    return re.sub(r"\s+v\d+$", "", name.strip())


def load_step(path):
    """Every component and named body of the assembly, each in the coordinates of its own component.

    Where a part sits in the assembly is dropped on purpose: a display case mounted at 25 degrees is still
    designed square, and its print orientation should not move when the assembly is re-posed.

    Returns a list of dicts: name, path (names from the root), kind ('part' or 'body'), shape,
    key (identifies the definition, so two instances of one part share it).
    """
    from OCP.IFSelect import IFSelect_RetDone
    from OCP.STEPCAFControl import STEPCAFControl_Reader
    from OCP.TCollection import TCollection_AsciiString, TCollection_ExtendedString
    from OCP.TDataStd import TDataStd_Name
    from OCP.TDF import TDF_Label, TDF_LabelSequence, TDF_Tool
    from OCP.TDocStd import TDocStd_Document
    from OCP.TopAbs import TopAbs_SOLID
    from OCP.TopExp import TopExp_Explorer
    from OCP.XCAFDoc import XCAFDoc_DocumentTool, XCAFDoc_ShapeTool

    doc = TDocStd_Document(TCollection_ExtendedString("XmlOcaf"))
    reader = STEPCAFControl_Reader()
    reader.SetNameMode(True)
    if reader.ReadFile(path) != IFSelect_RetDone:
        sys.exit(f"cannot read {path}")
    reader.Transfer(doc)
    tool = XCAFDoc_DocumentTool.ShapeTool_s(doc.Main())
    transfer = reader.Reader().WS().TransferReader()

    def name_of(label):
        if not label.IsAttribute(TDataStd_Name.GetID_s()):  # FindAttribute crashes on a label without a name
            return ""
        attr = TDataStd_Name()
        label.FindAttribute(TDataStd_Name.GetID_s(), attr)
        return attr.Get().ToExtString()

    def body_name(solid):
        # XCAF keeps component names only. The body name sits on the STEP entity the solid came from.
        ent = transfer.EntityFromShapeResult(solid, 1)
        return ent.Name().ToCString() if ent is not None and hasattr(ent, "Name") else ""

    def entry(label):
        s = TCollection_AsciiString()
        TDF_Tool.Entry_s(label, s)
        return s.ToCString()

    out = []

    def walk(label, trail):
        target = label
        if XCAFDoc_ShapeTool.IsReference_s(label):
            target = TDF_Label()
            XCAFDoc_ShapeTool.GetReferredShape_s(label, target)
        name = _clean(name_of(target))
        if XCAFDoc_ShapeTool.IsAssembly_s(target):
            kids = TDF_LabelSequence()
            XCAFDoc_ShapeTool.GetComponents_s(target, kids)
            for i in range(1, kids.Length() + 1):
                walk(kids.Value(i), trail + [name])
            return
        shape = XCAFDoc_ShapeTool.GetShape_s(target)
        # Fusion writes the loose bodies of an assembly as one unnamed compound: only its bodies matter
        loose = name in ("COMPOUND", "SOLID", "")
        if not loose:
            out.append(dict(name=name, path=trail, kind="part", shape=shape, key=entry(target)))
        ex, k = TopExp_Explorer(shape, TopAbs_SOLID), 0
        while ex.More():
            solid, k = ex.Current(), k + 1
            ex.Next()
            body = body_name(solid)
            if body and not re.fullmatch(r"Body\d+", body):
                out.append(dict(name=body, path=trail if loose else trail + [name], kind="body",
                                shape=solid, key=f"{entry(target)}#{k}"))

    roots = TDF_LabelSequence()
    tool.GetFreeShapes(roots)
    for i in range(1, roots.Length() + 1):
        walk(roots.Value(i), [])
    return out


def find(items, name):
    """The one part or body called `name` ("parent/name" narrows it). The first instance stands for all."""
    want = [_clean(p) for p in name.split("/")]
    hits = [it for it in items if (it["path"] + [it["name"]])[-len(want):] == want]
    parts = [it for it in hits if it["kind"] == "part"] or hits  # a component wins over its own body of the same name
    keys = {it["key"] for it in parts}
    if not parts:
        sys.exit(f"no part or body named {name!r} in the STEP (run `tree` to see the names)")
    if len(keys) > 1:
        where = sorted({"/".join(it["path"] + [it["name"]]) for it in parts})
        sys.exit(f"{name!r} names {len(keys)} different shapes; say which: {where}")
    return parts[0]


def brep_stats(shape):
    from OCP.BRepGProp import BRepGProp
    from OCP.GProp import GProp_GProps
    from OCP.TopAbs import TopAbs_SOLID
    from OCP.TopExp import TopExp_Explorer
    vol, area = GProp_GProps(), GProp_GProps()
    BRepGProp.VolumeProperties_s(shape, vol)
    BRepGProp.SurfaceProperties_s(shape, area)
    n, ex = 0, TopExp_Explorer(shape, TopAbs_SOLID)
    while ex.More():
        n, _ = n + 1, ex.Next()
    return dict(solids=n, vol=vol.Mass(), area=area.Mass())


def _triangulate(shape):
    """Triangles of every face, as OpenCascade meshes them. Returns (vertices, triangles)."""
    from OCP.BRep import BRep_Tool
    from OCP.BRepMesh import BRepMesh_IncrementalMesh
    from OCP.BRepTools import BRepTools
    from OCP.TopAbs import TopAbs_FACE, TopAbs_REVERSED
    from OCP.TopExp import TopExp_Explorer
    from OCP.TopLoc import TopLoc_Location
    from OCP.TopoDS import TopoDS

    def faces():
        ex = TopExp_Explorer(shape, TopAbs_FACE)
        while ex.More():
            face = TopoDS.Face_s(ex.Current())
            ex.Next()
            loc = TopLoc_Location()
            yield face, BRep_Tool.Triangulation_s(face, loc), loc

    BRepTools.Clean_s(shape)
    # absolute deflection (False): cadquery's own tessellate() takes it relative to the part size, which is far coarser
    BRepMesh_IncrementalMesh(shape, LIN_DEFLECTION, False, ANG_DEFLECTION, True)
    # OpenCascade gives up on some narrow cone bands (a 0.2 mm chamfer round a hole) at a fine deflection and
    # leaves the face empty. A coarser pass meshes only the faces still empty and keeps the fine edges they
    # share with their neighbours, so the solid stays closed; a band that narrow needs no inner points anyway.
    for coarser in (2, 4, 10):
        if all(poly is not None for _, poly, _ in faces()):
            break
        BRepMesh_IncrementalMesh(shape, LIN_DEFLECTION * coarser, False, ANG_DEFLECTION * coarser, True)
    verts, tris, offset = [], [], 0
    for face, poly, loc in faces():
        if poly is None:
            sys.exit("a face could not be tessellated")
        trsf = loc.Transformation()
        for i in range(1, poly.NbNodes() + 1):
            p = poly.Node(i).Transformed(trsf)
            verts.append((p.X(), p.Y(), p.Z()))
        flip = face.Orientation() == TopAbs_REVERSED
        for i in range(1, poly.NbTriangles() + 1):
            t = poly.Triangle(i)
            a, b, c = t.Value(1) + offset - 1, t.Value(2) + offset - 1, t.Value(3) + offset - 1
            tris.append((a, c, b) if flip else (a, b, c))
        offset += poly.NbNodes()
    return np.array(verts), np.array(tris)


def _weld(verts, tris):
    """Join vertices closer than WELD. Faces meet at corners a hair apart (the B-rep's own tolerance), which leaves pinholes."""
    from scipy.sparse import coo_matrix
    from scipy.sparse.csgraph import connected_components
    from scipy.spatial import cKDTree
    pairs = cKDTree(verts).query_pairs(WELD, output_type="ndarray")
    n = len(verts)
    graph = coo_matrix((np.ones(len(pairs)), (pairs[:, 0], pairs[:, 1])), shape=(n, n))
    count, label = connected_components(graph, directed=False)
    first = np.full(count, -1)
    first[label[::-1]] = np.arange(n)[::-1]  # every cluster keeps its first vertex
    tris = label[tris]
    tris = tris[(tris[:, 0] != tris[:, 1]) & (tris[:, 1] != tris[:, 2]) & (tris[:, 0] != tris[:, 2])]
    return verts[first], tris


def closed(tris):
    """(is the surface closed and consistently wound, how many edges carry more than two faces).

    Closed: every edge is walked once in each direction. An edge with four faces is where the CAD solid
    touches itself along a line; slicers cope, and the designer may want to know.
    """
    edges = np.concatenate([tris[:, [0, 1]], tris[:, [1, 2]], tris[:, [2, 0]]])
    fwd = np.unique(edges, axis=0, return_counts=True)
    back = np.unique(edges[:, ::-1], axis=0, return_counts=True)
    ok = fwd[0].shape == back[0].shape and (fwd[0] == back[0]).all() and (fwd[1] == back[1]).all()
    return bool(ok), int((np.unique(np.sort(edges, axis=1), axis=0, return_counts=True)[1] > 2).sum())


def to_mesh(shape):
    """Tessellate a B-rep into one welded triangle mesh.

    Stops unless the result is a closed solid with the volume and area of the B-rep: a mesh that lost a face
    still slices, and prints wrong.
    """
    verts, tris = _triangulate(shape)
    verts, tris = _weld(verts, tris)
    mesh = trimesh.Trimesh(verts, tris, process=False)
    if not closed(tris)[0]:
        sys.exit("the tessellation is not a closed solid")
    ref = brep_stats(shape)
    if abs(mesh.volume - ref["vol"]) > 1e-3 * ref["vol"] or abs(mesh.area - ref["area"]) > 2e-3 * ref["area"]:
        sys.exit(f"the tessellation lost shape: volume {mesh.volume:.1f} vs {ref['vol']:.1f} mm3, "
                 f"area {mesh.area:.1f} vs {ref['area']:.1f} mm2")
    return mesh


# ---------------------------------------------------------------- orientation

def place(mesh, up):
    """Turn the mesh so its axis `up` points up, stand it on z=0, centre it on the bed."""
    m = mesh.copy()
    t = np.eye(4)
    t[:3, :3] = UP[up]
    m.apply_transform(t)
    lo, hi = m.bounds
    m.apply_translation([-(lo[0] + hi[0]) / 2, -(lo[1] + hi[1]) / 2, -lo[2]])
    return m


def print_stats(m):
    """What a slicer would care about, for a mesh standing on z=0."""
    nz = m.face_normals[:, 2]
    zmin = m.triangles[:, :, 2].min(axis=1)
    zmax = m.triangles[:, :, 2].max(axis=1)
    on_bed = (zmax < 0.02) & (nz < -0.99)
    over = (nz < OVERHANG_NZ) & ~on_bed
    flat = over & (nz < -0.99)  # level ceilings: bridges, or faces that need support
    a = m.area_faces
    return dict(height=float(m.bounds[1][2]), bed=float(a[on_bed].sum()), overhang=float(a[over].sum()),
                ceiling=float(a[flat].sum()), on_bed=on_bed, over=over,
                first_layer_z=float(zmin[~on_bed].min()) if (~on_bed).any() else 0.0)


def render(m, png, title=""):
    """Two pictures side by side: from above and from below. Red = overhang, blue = on the bed."""
    import vtk
    from vtk.util import numpy_support as ns
    st = print_stats(m)
    colors = np.tile(np.array([[200, 200, 205]], np.uint8), (len(m.faces), 1))
    colors[st["over"]] = (220, 50, 40)
    colors[st["on_bed"]] = (60, 110, 220)
    poly = vtk.vtkPolyData()
    pts = vtk.vtkPoints()
    pts.SetData(ns.numpy_to_vtk(m.triangles.reshape(-1, 3).astype(np.float64), deep=True))
    poly.SetPoints(pts)
    cells = vtk.vtkCellArray()
    conn = np.arange(len(m.faces) * 3, dtype=np.int64)
    cells.SetData(3, ns.numpy_to_vtk(conn, deep=True, array_type=vtk.VTK_ID_TYPE))
    poly.SetPolys(cells)
    col = ns.numpy_to_vtk(colors, deep=True)
    col.SetNumberOfComponents(3)
    poly.GetCellData().SetScalars(col)

    win = vtk.vtkRenderWindow()
    win.SetOffScreenRendering(1)
    win.SetSize(1500, 750)
    size = float(np.linalg.norm(m.extents))
    centre = m.bounds.mean(axis=0)
    for i, (direction, label) in enumerate([((1, -1.3, 0.9), "from above"), ((1, -1.3, -0.9), "from below")]):
        ren = vtk.vtkRenderer()
        ren.SetViewport(i / 2, 0, (i + 1) / 2, 1)
        ren.SetBackground(1, 1, 1)
        mapper = vtk.vtkPolyDataMapper()
        mapper.SetInputData(poly)
        actor = vtk.vtkActor()
        actor.SetMapper(mapper)
        actor.GetProperty().SetInterpolationToFlat()
        actor.GetProperty().SetAmbient(0.35)
        actor.GetProperty().SetDiffuse(0.75)
        ren.AddActor(actor)
        edges = vtk.vtkFeatureEdges()
        edges.SetInputData(_welded(m))
        edges.BoundaryEdgesOff()
        edges.NonManifoldEdgesOff()
        edges.ManifoldEdgesOff()
        edges.FeatureEdgesOn()
        edges.SetFeatureAngle(25)
        em = vtk.vtkPolyDataMapper()
        em.SetInputConnection(edges.GetOutputPort())
        em.ScalarVisibilityOff()
        ea = vtk.vtkActor()
        ea.SetMapper(em)
        ea.GetProperty().SetColor(0.1, 0.1, 0.1)
        ea.GetProperty().SetLineWidth(1.2)
        ren.AddActor(ea)
        if i == 0:  # the bed
            plane = vtk.vtkPlaneSource()
            e = max(m.extents[0], m.extents[1]) * 0.65
            plane.SetOrigin(-e, -e, -0.05)
            plane.SetPoint1(e, -e, -0.05)
            plane.SetPoint2(-e, e, -0.05)
            pm = vtk.vtkPolyDataMapper()
            pm.SetInputConnection(plane.GetOutputPort())
            pa = vtk.vtkActor()
            pa.SetMapper(pm)
            pa.GetProperty().SetColor(0.85, 0.9, 0.85)
            ren.AddActor(pa)
        text = vtk.vtkTextActor()
        text.SetInput(f"{title}  {label}" if i else f"{title}  {label}   "
                      f"h={st['height']:.1f}  bed={st['bed']:.0f}  overhang={st['overhang']:.0f} mm2")
        text.GetTextProperty().SetColor(0, 0, 0)
        text.GetTextProperty().SetFontSize(20)
        text.SetPosition(10, 10)
        ren.AddViewProp(text)
        cam = ren.GetActiveCamera()
        cam.SetFocalPoint(*centre)
        d = np.array(direction, float)
        cam.SetPosition(*(centre + d / np.linalg.norm(d) * size * 2.2))
        cam.SetViewUp(0, 0, 1)
        ren.ResetCamera()
        cam.Zoom(1.25)
        win.AddRenderer(ren)
    win.Render()
    grab = vtk.vtkWindowToImageFilter()
    grab.SetInput(win)
    grab.Update()
    writer = vtk.vtkPNGWriter()
    writer.SetFileName(png)
    writer.SetInputConnection(grab.GetOutputPort())
    writer.Write()


def _welded(m):
    import vtk
    from vtk.util import numpy_support as ns
    poly = vtk.vtkPolyData()
    pts = vtk.vtkPoints()
    pts.SetData(ns.numpy_to_vtk(np.asarray(m.vertices, np.float64), deep=True))
    poly.SetPoints(pts)
    cells = vtk.vtkCellArray()
    cells.SetData(3, ns.numpy_to_vtk(np.asarray(m.faces, np.int64).ravel(), deep=True, array_type=vtk.VTK_ID_TYPE))
    poly.SetPolys(cells)
    return poly


# ---------------------------------------------------------------- comparing meshes

MATCH_TOL = 0.1  # mm: two tessellations of one part differ by less than this


def _spin(deg):
    c, s = np.cos(np.radians(deg)), np.sin(np.radians(deg))
    t = np.eye(4)
    t[:3, :3] = [[c, -s, 0], [s, c, 0], [0, 0, 1]]
    return t


def surface_gap(a, b, n=4000):
    """Largest distance from sampled points of each surface to the other surface (mm)."""
    pa = a.sample(n, seed=1)
    pb = b.sample(n, seed=1)
    return max(float(trimesh.proximity.closest_point(b, pa)[1].max()),
               float(trimesh.proximity.closest_point(a, pb)[1].max()))


def match_pose(cad, stl):
    """Is `stl` the CAD part lying on one of its six sides? Returns (up, spin in degrees, gap in mm) of the best pose.

    The spin about the vertical is free: it does not change the print.
    Quarter turns are tried first, then every degree.
    """
    target = stl.copy()
    target.apply_translation(-target.center_mass)
    best = None
    for up in UP:
        m = cad.copy()
        t = np.eye(4)
        t[:3, :3] = UP[up]
        m.apply_transform(t)
        if abs(m.extents[2] - stl.extents[2]) > 0.2:
            continue
        m.apply_translation(-m.center_mass)
        for deg in (0, 90, 180, 270):
            turned = m.copy().apply_transform(_spin(deg))
            if np.abs(turned.extents - stl.extents).max() > 0.2:
                continue
            gap = surface_gap(turned, target, 1500)
            if best is None or gap < best[2]:
                best = (up, deg, gap)
            if gap < MATCH_TOL:
                return best
        pts = m.sample(300, seed=2)
        coarse = [(float(trimesh.proximity.closest_point(target, pts @ _spin(d)[:3, :3].T)[1].max()), d) for d in range(360)]
        d0 = min(coarse)[1]
        fine = [(float(trimesh.proximity.closest_point(target, pts @ _spin(d)[:3, :3].T)[1].max()), d)
                for d in np.arange(d0 - 1, d0 + 1.01, 0.05)]
        deg = min(fine)[1]
        gap = surface_gap(m.copy().apply_transform(_spin(deg)), target, 1500)
        if best is None or gap < best[2]:
            best = (up, round(float(deg) % 360, 2), gap)
        if gap < MATCH_TOL:
            return best
    return best


# ---------------------------------------------------------------- commands

def cmd_tree(a):
    seen = set()
    for it in load_step(a.step):
        if len(it["path"]) > a.depth:
            continue
        s = brep_stats(it["shape"])
        if s["vol"] < a.min_volume:
            continue
        again = "  (again)" if it["key"] in seen else ""
        seen.add(it["key"])
        print(f"{'/'.join(it['path'])}/  {it['name']}   [{it['kind']}] solids={s['solids']} vol={s['vol']:.0f} mm3{again}")


def cmd_diff(a):
    def table(path):
        t = {}
        for it in load_step(path):
            t.setdefault(it["name"], brep_stats(it["shape"]))
        return t
    old, new = table(a.old), table(a.new)
    for name, s in new.items():
        if s["vol"] < a.min_volume:
            continue
        o = old.get(name)
        if o is None:
            print(f"NEW      {name}  vol={s['vol']:.1f} area={s['area']:.1f}")
        elif abs(o["vol"] - s["vol"]) > 0.5 or abs(o["area"] - s["area"]) > 0.5:
            print(f"CHANGED  {name}  vol {o['vol']:.1f} -> {s['vol']:.1f}  area {o['area']:.1f} -> {s['area']:.1f}")
        elif a.all:
            print(f"same     {name}")
    for name, s in old.items():
        if name not in new and s["vol"] >= a.min_volume:
            print(f"REMOVED  {name}  vol={s['vol']:.1f}")


def cmd_orient(a):
    items = load_step(a.step)
    for name in a.parts:
        mesh = to_mesh(find(items, name)["shape"])
        print(f"{name}: {len(mesh.faces)} triangles, volume={mesh.volume:.1f} mm3 size={np.round(mesh.extents, 2).tolist()}")
        print("   up   height   on bed   overhang   level ceilings   (mm, mm2)")
        for up in UP:
            m = place(mesh, up)
            s = print_stats(m)
            print(f"   {up}  {s['height']:7.1f} {s['bed']:8.0f} {s['overhang']:10.0f} {s['ceiling']:16.0f}")
            if a.png:
                os.makedirs(a.png, exist_ok=True)
                render(m, os.path.join(a.png, f"{name.replace('/', '_')}_{up[1]}{'pos' if up[0] == '+' else 'neg'}.png"),
                       f"{name} up={up}")


def _manifest(path):
    man = json.load(open(path, encoding="utf-8"))
    root = os.path.dirname(os.path.abspath(path))
    while not os.path.exists(os.path.join(root, man["step"])):
        parent = os.path.dirname(root)
        if parent == root:
            sys.exit(f"cannot find {man['step']} above {path}")
        root = parent
    return man, root


def cmd_check(a):
    man, root = _manifest(a.manifest)
    items = load_step(os.path.join(root, man["step"]))
    bad = 0
    for p in man["parts"]:
        cad = to_mesh(find(items, p["part"])["shape"])
        path = os.path.join(root, man["out"], p["stl"])
        if not os.path.exists(path):
            print(f"MISSING  {p['stl']}")
            bad += 1
            continue
        stl = trimesh.load_mesh(path)
        stl.merge_vertices()
        if not closed(stl.faces)[0]:
            print(f"OPEN     {p['stl']}: the file is not a closed solid")
            bad += 1
            continue
        pose = match_pose(cad, stl)
        if pose is None or pose[2] > MATCH_TOL:
            why = "different height on every side" if pose is None else f"surfaces up to {pose[2]:.2f} mm apart"
            print(f"DIFFERS  {p['stl']}: {why} (CAD vol {cad.volume:.0f}, STL vol {stl.volume:.0f})")
            bad += 1
        else:
            note = "" if pose[0] == p["up"] else f"  but the STL has {pose[0]} up, the manifest says {p['up']}"
            if note:
                bad += 1
            print(f"{'ok      ' if not note else 'POSE    '} {p['stl']}: matches the CAD within {pose[2]:.3f} mm, {pose[0]} up{note}")
    print("PASS: every STL is the CAD part in the manifest's orientation" if not bad else f"FAIL: {bad} of {len(man['parts'])} need a look")
    return 1 if bad else 0


def cmd_export(a):
    man, root = _manifest(a.manifest)
    items = load_step(os.path.join(root, man["step"]))
    out = a.out or os.path.join(root, man["out"])
    os.makedirs(out, exist_ok=True)
    for p in man["parts"]:
        if a.only and p["stl"] not in a.only:
            continue
        cad = to_mesh(find(items, p["part"])["shape"])
        m = place(cad, p["up"])
        m.export(os.path.join(out, p["stl"]))
        s = print_stats(m)
        touching = closed(m.faces)[1]
        print(f"wrote {p['stl']}: {len(m.faces)} triangles, {np.round(m.extents, 2).tolist()} mm, "
              f"vol {m.volume:.0f} mm3, {p['up']} up, on bed {s['bed']:.0f} mm2, overhang {s['overhang']:.0f} mm2"
              + (f"  (the CAD solid touches itself along {touching} edges)" if touching else ""))
        if s["bed"] < 1:
            print(f"   WARNING {p['stl']}: nothing flat lies on the bed with {p['up']} up")
        if a.png:
            os.makedirs(a.png, exist_ok=True)
            render(m, os.path.join(a.png, p["stl"][:-4] + ".png"), p["stl"])


BAMBU = r"C:\Program Files\Bambu Studio"


def cmd_slice(a):
    """Slice every STL as it is, alone on the plate, with Bambu Studio's stock X1C 0.20 mm profile.

    This is the test on the real tool chain: the slicer must take the file and report no warning.
    The profile is only a yardstick. It leaves supports off, so it cannot tell that none are needed.
    """
    import subprocess
    import tempfile
    man, root = _manifest(a.manifest)
    exe = os.path.join(a.bambu, "bambu-studio.exe")
    prof = os.path.join(a.bambu, "resources", "profiles", "BBL")
    if not os.path.exists(exe):
        sys.exit(f"no Bambu Studio at {a.bambu} (--bambu DIR)")
    settings = ";".join([os.path.join(prof, "machine", "Bambu Lab X1 Carbon 0.4 nozzle.json"),
                         os.path.join(prof, "process", "0.20mm Standard @BBL X1C.json")])
    filament = os.path.join(prof, "filament", "Bambu PETG Basic @BBL X1C.json")
    bad = 0
    for p in man["parts"]:
        if a.only and p["stl"] not in a.only:
            continue
        with tempfile.TemporaryDirectory() as tmp:
            subprocess.run([exe, "--load-settings", settings, "--load-filaments", filament, "--arrange", "1",
                            "--slice", "0", "--outputdir", tmp, os.path.join(root, man["out"], p["stl"])],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=900)
            result = os.path.join(tmp, "result.json")  # the program prints nothing; this file is its answer
            if not os.path.exists(result):
                print(f"FAILED   {p['stl']}: Bambu Studio wrote no result")
                bad += 1
                continue
            r = json.load(open(result, encoding="utf-8"))
        plate = (r.get("sliced_plates") or [{}])[0]
        times = plate.get("feature_type_times", {})
        warn = plate.get("warning_message", "")
        ok = r.get("return_code") == 0 and not warn
        bad += not ok
        print(f"{'ok      ' if ok else 'FAILED  '} {p['stl']}: {r.get('error_string')} {plate.get('main_predication', 0) / 60:.0f} min, "
              f"bridges {times.get('Bridge', 0):.0f} s, overhanging walls {times.get('Overhang wall', 0):.0f} s"
              + (f", warning: {warn}" if warn else ""))
    print("PASS: every STL slices without a warning" if not bad else f"FAIL: {bad} did not slice cleanly")
    return 1 if bad else 0


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("tree", help="list the parts and named bodies of a STEP")
    s.add_argument("step")
    s.add_argument("--depth", type=int, default=9, help="how many assembly levels down to list")
    s.add_argument("--min-volume", type=float, default=500, help="hide screws and inserts (mm3)")
    s.set_defaults(fn=cmd_tree)
    s = sub.add_parser("diff", help="which parts changed between two STEP files")
    s.add_argument("old")
    s.add_argument("new")
    s.add_argument("--all", action="store_true", help="also list the unchanged parts")
    s.add_argument("--min-volume", type=float, default=500)
    s.set_defaults(fn=cmd_diff)
    s = sub.add_parser("orient", help="rate the six print orientations of a part")
    s.add_argument("step")
    s.add_argument("parts", nargs="+")
    s.add_argument("--png", help="folder for a picture of every orientation")
    s.set_defaults(fn=cmd_orient)
    s = sub.add_parser("check", help="compare the published STLs with the CAD")
    s.add_argument("manifest")
    s.set_defaults(fn=cmd_check)
    s = sub.add_parser("export", help="write the STLs of a manifest")
    s.add_argument("manifest")
    s.add_argument("--only", nargs="+", help="STL file names to write (default: all)")
    s.add_argument("--out", help="write here instead of the manifest's folder")
    s.add_argument("--png", help="folder for a picture of every STL as it will print")
    s.set_defaults(fn=cmd_export)
    s = sub.add_parser("slice", help="slice every STL of a manifest with Bambu Studio")
    s.add_argument("manifest")
    s.add_argument("--only", nargs="+", help="STL file names to slice (default: all)")
    s.add_argument("--bambu", default=BAMBU, help="Bambu Studio's folder")
    s.set_defaults(fn=cmd_slice)
    a = ap.parse_args()
    sys.exit(a.fn(a) or 0)


if __name__ == "__main__":
    main()
