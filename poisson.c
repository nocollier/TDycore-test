const char help[] = "";

#include <petscdmplex.h>
#include <petscds.h>
#include <petscsnes.h>
#include <petscfe.h>

static PetscErrorCode dirichlet_bc(PetscInt dim, PetscReal time, const PetscReal x[], PetscInt Nc, PetscScalar bcval[], void *ctx)
{
  bcval[0] = 1.234;
  return 0;
}

static PetscReal body_force(PetscReal t, const PetscReal x[])
{
  return 0.0;
}

/*
  residual = int( phi * f0 ) + int( grad(phi) * f1 )
 */
static void f0(PetscInt dim, PetscInt Nf, PetscInt NfAux,
               const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
               const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
               PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f0[])
{
  f0[0] = -body_force(t,x);
}

static void f1(PetscInt dim, PetscInt Nf, PetscInt NfAux,
               const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
               const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
               PetscReal t, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar f1[])
{
  for (PetscInt d = 0; d < dim; ++d) f1[d] = u_x[d];
}

/*
  The f0 term contains no use of u and so has no contribution to the
  Jacobian. Since our f1 term uses gradient information from u, we
  only have 1 contribution in the jacobian:

  jacobian = int( grad(phi) * g3 * grad(psi) )

  For total generality, g3 is returned as a dim x dim tensor per
  component. In our case we need this to be the identity and so we set
  the diagonal to 1.
 */
static void g3(PetscInt dim, PetscInt Nf, PetscInt NfAux,
	       const PetscInt uOff[], const PetscInt uOff_x[], const PetscScalar u[], const PetscScalar u_t[], const PetscScalar u_x[],
	       const PetscInt aOff[], const PetscInt aOff_x[], const PetscScalar a[], const PetscScalar a_t[], const PetscScalar a_x[],
	       PetscReal t, PetscReal u_tShift, const PetscReal x[], PetscInt numConstants, const PetscScalar constants[], PetscScalar g3[])
{
  for (PetscInt d = 0; d < dim; ++d) g3[d*dim+d] = 1;
}

int main(int argc,char **argv)
{
  DM       dm;
  PetscFE  fe;
  PetscDS  ds;
  SNES     snes;
  Vec      u;
  DMLabel  label;
  const int id = 1;
  PetscCall(PetscInitialize(&argc,&argv,NULL,help));

  /* Initialize mesh */
  PetscCall(DMCreate(PETSC_COMM_WORLD, &dm));
  PetscCall(DMSetType(dm, DMPLEX));
  //PetscCall(DMPlexCreateWedgeBoxMesh(PETSC_COMM_WORLD,NULL,NULL,NULL,NULL,PETSC_TRUE,PETSC_TRUE,&dm)); // this creates a tensor wedge at the moment
  PetscCall(DMSetFromOptions(dm));
  PetscCall(DMViewFromOptions(dm, NULL, "-dm_view"));

  /* */
  PetscCall(DMCreateFEDefault(dm, 1, NULL, PETSC_DETERMINE, &fe));
  PetscCall(DMSetField(dm, 0, NULL, (PetscObject) fe));
  PetscCall(DMCreateDS(dm));  
  PetscCall(DMGetDS(dm, &ds));
  PetscCall(PetscDSSetResidual(ds, 0, f0, f1));
  PetscCall(PetscDSSetJacobian(ds, 0, 0, NULL, NULL, NULL, g3));

  /* Setup boundary conditions */
  PetscCall(DMGetLabel(dm, "marker", &label));
  PetscCall(DMAddBoundary(dm, DM_BC_ESSENTIAL, "all", label, 1, &id, 0, 0, NULL, (void (*)(void)) dirichlet_bc, NULL, NULL, NULL));
  
  /* Prepare the solution vector */
  PetscCall(DMGetGlobalVector(dm, &u));
  PetscCall(VecSet(u, 0.0));
  
  /* Create the nonlinear solver */
  PetscCall(SNESCreate(PETSC_COMM_WORLD, &snes));
  PetscCall(SNESSetDM(snes, dm));
  PetscCall(DMPlexSetSNESLocalFEM(dm, NULL, NULL, NULL));

  /* */
  PetscCall(SNESSetFromOptions(snes));
  PetscCall(DMSNESCheckFromOptions(snes, u));
  PetscCall(SNESSolve(snes, NULL, u));
  VecView(u,PETSC_VIEWER_STDOUT_WORLD);
  
  /* Cleanup */
  PetscCall(PetscFEDestroy(&fe));
  PetscCall(DMDestroy(&dm));
  PetscCall(PetscFinalize());
  return 0;
}

/*

  -pc_type lu -ksp_type preonly -snes_converged_reason

  tri:
    -petscspace_degree 1 

  quad:
    -petscspace_degree 1 -dm_plex_simplex 0

  tet:
    -petscspace_degree 1 -dm_plex_dim 3 -dm_refine 1

  hex:
    -petscspace_degree 1 -dm_plex_simplex 0 -dm_plex_dim 3 -dm_refine 1

  prism:
    -petscspace_degree 1 -dm_plex_cell triangular_prism 
 */
