INCLUDE(TribitsAddExecutableAndTest)

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  FECrsGraph_test_LL
  SOURCES cxx_main.cpp ExecuteTestProblems.cpp
  ARGS -v
  COMM mpi serial
  NUM_MPI_PROCS 1
  FAIL_REGULAR_EXPRESSION "tests FAILED"
  )
