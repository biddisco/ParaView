vtk_module(vtkPVServerManagerRendering
  GROUPS
    ParaViewRendering
  DEPENDS
    vtkPVServerManagerCore
    vtkPVServerImplementationRendering
    vtkPVClientServerCoreDefault
  PRIVATE_DEPENDS
    vtksys
  TEST_LABELS
    PARAVIEW
)
