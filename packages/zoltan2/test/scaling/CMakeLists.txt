INCLUDE(TribitsAddExecutableAndTest)

IF (${PACKAGE_NAME}_ENABLE_Experimental)
  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    rcbPerformance
    SOURCES rcbPerformance.cpp
    NUM_MPI_PROCS 4
    COMM serial mpi
    PASS_REGULAR_EXPRESSION "PASS"
    FAIL_REGULAR_EXPRESSION "FAIL"
  )
ENDIF()

IF (${PROJECT_NAME}_ENABLE_Zoltan)
  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    rcbPerformanceZ1
    SOURCES rcbPerformanceZ1.cpp
    NUM_MPI_PROCS 4
    COMM serial mpi
    PASS_REGULAR_EXPRESSION "PASS"
    FAIL_REGULAR_EXPRESSION "FAIL"
  )
ENDIF()
