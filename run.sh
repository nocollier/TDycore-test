
OPTS=" -pc_type lu -ksp_type preonly -snes_converged_reason -petscspace_degree 1 -dm_view "

echo "-------------------------Tri---------------------------"
./poisson ${OPTS} -snes_view_solution vtk:tri.vtu
echo "------------------------Quad---------------------------"
./poisson ${OPTS} -dm_plex_simplex 0 -snes_view_solution vtk:quad.vtu
echo "-------------------------Tet---------------------------"
./poisson ${OPTS} -dm_plex_dim 3 -dm_refine 1 -snes_view_solution vtk:tet.vtu
echo "-------------------------Hex---------------------------"
./poisson ${OPTS} -dm_plex_simplex 0 -dm_plex_dim 3 -dm_refine 1 -snes_view_solution vtk:hex.vtu
echo "------------------------Prism--------------------------"
./poisson ${OPTS} -dm_plex_dim 3 -dm_plex_cell triangular_prism -snes_view_solution vtk:prism.vtu

  
