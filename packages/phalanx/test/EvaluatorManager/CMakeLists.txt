
INCLUDE(TribitsAddExecutableAndTest)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../Utilities)

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  evaluator_manager
  SOURCES EvaluatorManager.cpp
  DEPLIBS phalanx_test_utilities
  COMM serial mpi
  NUM_MPI_PROCS 1
  PASS_REGULAR_EXPRESSION "Test passed!"
  )
