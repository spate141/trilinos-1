
INCLUDE(TribitsLibraryMacros)

#
# B) Define the header and source files (and directories)
#

#
# src
#

SET(HEADERS "")
SET(SOURCES "")

#
# Core Files
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# general headers, sources
APPEND_SET(HEADERS
    # standard
    Kokkos_LinAlgVersion.hpp
    # data structures
    Kokkos_CrsGraphBase.hpp
    Kokkos_CrsMatrixBase.hpp
    Kokkos_MultiVector.hpp
    Kokkos_VbrMatrix.hpp
    # Default LocalMapOps template parameters
    Kokkos_DefaultKernels.hpp
    # kernels
    Kokkos_DefaultBlockSparseKernelOps.hpp
    Kokkos_DefaultSparseMultiplyKernelOps.hpp
    Kokkos_DefaultSparseSolveKernelOps.hpp
    Kokkos_DefaultRelaxationKernelOps.hpp
    Kokkos_MultiVectorKernelOps.hpp
    # traits and kernel wrappers
    Kokkos_DefaultArithmetic.hpp
    Kokkos_DefaultBlockSparseOps.hpp
    Kokkos_DefaultSparseOps.hpp
    Kokkos_DefaultRelaxation.hpp
    Kokkos_AltSparseOps.hpp
    # MKL helper class; doesn't depend on MKL library.
    Kokkos_Mkl_MatrixDescriptor.hpp
    # raw sequential and OpenMP kernels
    Kokkos_Raw_addSparseMatrices_decl.hpp
    Kokkos_Raw_addSparseMatrices_def.hpp
    Kokkos_Raw_SparseMatVec_decl.hpp
    Kokkos_Raw_SparseMatVec_def.hpp
    Kokkos_Raw_SparseTriangularSolve_decl.hpp
    Kokkos_Raw_SparseTriangularSolve_def.hpp
  )

APPEND_SET(SOURCES
    Kokkos_LinAlgVersion.cpp
    # MKL helper class; doesn't depend on MKL library.
    Kokkos_Mkl_MatrixDescriptor.cpp
  )

IF (KokkosClassic_ENABLE_CUSPARSE)
  APPEND_SET(SOURCES Kokkos_CUSPARSEOps.cpp)
  APPEND_SET(HEADERS Kokkos_CUSPARSEOps.hpp)
ENDIF()

IF (KokkosClassic_ENABLE_Cusp)
  APPEND_SET(HEADERS Kokkos_CuspOps.hpp)
ENDIF()

SET(CUDA_SOURCES Kokkos_DefaultArithmetic.cu 
#                Kokkos_DefaultBlockSparseOps.cu 
#                Kokkos_DefaultSparseOps.cu 
#                Kokkos_DefaultRelaxation.cu
   )
IF (KokkosClassic_ENABLE_Cusp) 
  APPEND_SET(CUDA_SOURCES Kokkos_CuspOps.cu)
ENDIF()

ASSERT_DEFINED(KokkosClassic_ENABLE_Thrust)
IF (KokkosClassic_ENABLE_Thrust)
  PACKAGE_ADD_CUDA_LIBRARY(kokkoslinalg_cuda SOURCES ${CUDA_SOURCES})
ENDIF()

ASSERT_DEFINED(KokkosClassic_ENABLE_MKL)
IF (KokkosClassic_ENABLE_MKL)
  APPEND_SET(HEADERS
    Kokkos_Mkl_RawSparseKernels.hpp
    Kokkos_MklSparseOps.hpp
    )
  APPEND_SET(SOURCES
    Kokkos_Mkl_RawSparseKernels.cpp
    )
ENDIF ()

#
# C) Define the targets for package's library(s)
#

TRIBITS_ADD_LIBRARY(
  kokkoslinalg
  HEADERS ${HEADERS}
  SOURCES ${SOURCES}
  #DEPLIBS kokkos kokkosnodeapi ${KOKKOS_LINALG_CUDALIBS}
  )

TRIBITS_ADD_TEST_DIRECTORIES(examples)
TRIBITS_ADD_TEST_DIRECTORIES(test)
