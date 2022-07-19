
all:
	make -f ${PETSC_DIR}/share/petsc/Makefile.user poisson

clean:
	rm -f poisson *~
