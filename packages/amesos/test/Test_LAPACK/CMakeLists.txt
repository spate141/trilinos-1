
INCLUDE(TribitsAddExecutableAndTest)

IF(${PACKAGE_NAME}_ENABLE_LAPACK)
  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    Test_LAPACK
    SOURCES cxx_main.cpp
    NUM_MPI_PROCS 4
    COMM serial mpi
    )
ENDIF()
