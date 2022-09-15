
all_sol: sol_tet.vtu sol_hex.vtu sol_prism.vtu

all_mesh: mesh_tet.msh mesh_hex.msh mesh_prism.msh

mesh_%.msh:
	python create_terrain_mesh.py \
	-e $* \
	--mesh_size 0.2 \
	--aspect_ratio 0.5 \
	--preview

sol_%.vtu: mesh_%.msh poisson
	./poisson ${OPTS} \
	-pc_type lu -ksp_type preonly \
	-snes_converged_reason \
	-petscspace_degree 1 \
	-dm_plex_filename $< \
	-snes_view_solution vtk:$@ \
	-dm_view

poisson: poisson.c
	make -f ${PETSC_DIR}/share/petsc/Makefile.user poisson

clean:
	rm -f poisson extrude *.msh *.vtu *~

