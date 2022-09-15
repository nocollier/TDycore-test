"""Create gmsh mesh files for analytic terrain"""

import argparse

import numpy as np

import gmsh


def terrain(node_x, node_y):
    """An analytic terrain x,y in [0,2] x [0,1] with realistic features for
    testing purposes."""
    node_zx = np.sin(2 * np.pi * node_x / 4 - np.pi / 2) + 1
    node_zy = np.sin(2 * np.pi * node_y + np.pi / 2) + 1
    node_zp = node_zy * (np.cos(2 * np.pi * node_x / 0.5) + 1)
    return 0.1 * (node_zx + node_zy + 0.5 * node_zp)


parser = argparse.ArgumentParser(description="Create gmsh files for analytic terrain")
parser.add_argument(
    "-p", "--preview", action="store_true", default=False, dest="preview"
)
parser.add_argument(
    "-ms", "--mesh_size", type=float, action="store", default=0.5, dest="mesh_size"
)
parser.add_argument(
    "-ar",
    "--aspect_ratio",
    type=float,
    action="store",
    default=0.1,
    dest="aspect_ratio",
)
parser.add_argument(
    "-d",
    "--modeled_depth",
    type=float,
    action="store",
    default=0.3,
    dest="modeled_depth",
)
parser.add_argument(
    "-e",
    "--elem_type",
    type=str,
    action="store",
    choices=["tet", "hex", "prism"],
    default="hex",
    dest="elem_type",
)
args = parser.parse_args()

gmsh.initialize()
gmsh.model.add("terrain")

# Create a B-spline surface representing the terrain
N = 20
ps = []
for i in range(2 * N + 1):
    x = 2 * float(i) / (2 * N)
    for j in range(N + 1):
        y = float(j) / N
        ps.append(gmsh.model.occ.addPoint(x, y, terrain(x, y)))
srf = gmsh.model.occ.addBSplineSurface(ps, numPointsU=N + 1)
gmsh.model.occ.synchronize()
gmsh.model.occ.remove([(0, p) for p in ps])  # don't need these points now

# For hexes we need to set more recombine options
if args.elem_type == "hex":
    gmsh.model.mesh.setTransfiniteSurface(1)
    gmsh.option.setNumber("Mesh.RecombineAll", 1)
    gmsh.option.setNumber("Mesh.RecombinationAlgorithm", 1)
    gmsh.option.setNumber("Mesh.Recombine3DLevel", 2)
    gmsh.option.setNumber("Mesh.ElementOrder", 1)

# Extrude the surface and mesh downward
num_layers = max(
    1, int(round(args.modeled_depth / (args.mesh_size * args.aspect_ratio)))
)
ext = gmsh.model.occ.extrude(
    [(2, srf)],
    0,
    0,
    -args.modeled_depth,
    [num_layers],
    recombine=(args.elem_type in ["prism", "hex"]),
)

# Mesh and save
gmsh.model.occ.synchronize()
gmsh.option.setNumber("Mesh.MeshSizeMin", args.mesh_size)
gmsh.option.setNumber("Mesh.MeshSizeMax", args.mesh_size)
gmsh.model.mesh.generate(3)
gmsh.write(f"mesh_{args.elem_type}.msh")
if args.preview:
    gmsh.fltk.run()
gmsh.finalize()
