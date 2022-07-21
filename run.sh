
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
#
# This solves the problem correctly, but if you check the vtk file, it is tetrahedra
./poisson ${OPTS} -dm_plex_dim 3 -dm_plex_cell triangular_prism -dm_refine 1 -snes_view_solution vtk:prism.vtu

# 
# You have to refine twice because once still only gets you boundary nodes.
# Error: Nonlinear solve did not converge due to DIVERGED_FNORM_NAN iterations 0
./poisson ${OPTS} \
	  -dm_plex_dim 3 \
	  -dm_plex_reference_cell_domain \
	  -dm_plex_cell triangular_prism \
	  -dm_plex_boundary_label marker \
	  -dm_refine 2 \
	  -snes_monitor \
	  -snes_view_solution vtk:prism.vtu
