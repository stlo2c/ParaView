PROJECT(Array)

IF(NOT VTK_BINARY_DIR)
FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})
ENDIF(NOT VTK_BINARY_DIR)

IF(NOT VTK_USE_N_WAY_ARRAYS)
  MESSAGE(FATAL_ERROR "Example ${PROJECT_NAME} requires VTK_USE_N_WAY_ARRAYS.")
ENDIF(NOT VTK_USE_N_WAY_ARRAYS)

ADD_EXECUTABLE(ArrayBasics ArrayBasics.cxx)
TARGET_LINK_LIBRARIES(ArrayBasics vtkCommon)

ADD_EXECUTABLE(IdentityMatrix IdentityMatrix.cxx)
TARGET_LINK_LIBRARIES(IdentityMatrix vtkCommon)

ADD_EXECUTABLE(ArrayIteration ArrayIteration.cxx)
TARGET_LINK_LIBRARIES(ArrayIteration vtkCommon)

ADD_EXECUTABLE(AdjacencyMatrix AdjacencyMatrix.cxx)
TARGET_LINK_LIBRARIES(AdjacencyMatrix vtkInfovis vtkViews)

