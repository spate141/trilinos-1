MACRO(TRILINOS_ADD_EXECUTABLE EXECUTABLE_DIR)

  SET(EXECUTABLE_NAME "${PROJECT_NAME}${EXECUTABLE_DIR}")
  SET(TEST_NAME "${PROJECT_NAME}-${EXECUTABLE_DIR}")
  SET(MPI_TEST_NAME "${PROJECT_NAME}-${EXECUTABLE_DIR}-MPI")

  SET(KEYWORD )
  SET(EXECUTABLE_SOURCES )
  SET(EXECUTABLE_ARGS )
  SET(INSTALL_EXECUTABLE 0)
  SET(TEST_EXECUTABLE 0)
  SET(MPI_TEST_EXECUTABLE 0)

  FOREACH(ARGUMENT ${ARGN})
    IF(ARGUMENT STREQUAL "SOURCES")
      SET(KEYWORD ${ARGUMENT})
    ELSEIF(ARGUMENT STREQUAL "ARGS")
      SET(KEYWORD ${ARGUMENT})
    ELSEIF(ARGUMENT STREQUAL "INSTALL")
      SET(INSTALL_EXECUTABLE 1)
    ELSEIF(ARGUMENT STREQUAL "TEST")
      SET(TEST_EXECUTABLE 1)
    ELSEIF(ARGUMENT STREQUAL "MPITEST")
      SET(MPI_TEST_EXECUTABLE 1)
    ELSE(ARGUMENT STREQUAL "SOURCES")
      IF(KEYWORD STREQUAL "SOURCES")
        SET(EXECUTABLE_SOURCES ${EXECUTABLE_SOURCES} ${EXECUTABLE_DIR}/${ARGUMENT})
      ELSEIF(KEYWORD STREQUAL "ARGS")
        SET(EXECUTABLE_ARGS ${EXECUTABLE_ARGS} ${ARGUMENT})
      ELSE(KEYWORD STREQUAL "SOURCES")
      ENDIF(KEYWORD STREQUAL "SOURCES")
    ENDIF(ARGUMENT STREQUAL "SOURCES")
  ENDFOREACH(ARGUMENT)

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_DIR})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/${EXECUTABLE_DIR})

  ADD_EXECUTABLE(${EXECUTABLE_NAME} ${EXECUTABLE_SOURCES})

  IF(INSTALL_EXECUTABLE)
    INSTALL(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION bin)
  ENDIF(INSTALL_EXECUTABLE)

  IF(TEST_EXECUTABLE)
    ADD_TEST(${TEST_NAME} ${EXECUTABLE_NAME} ${EXECUTABLE_ARGS})

    IF(MPI_TEST_EXECUTABLE)
      ADD_TEST(${MPI_TEST_NAME} ${MPI_EXECUTABLE} ${MPI_EXECUTABLE_FLAGS} ${EXECUTABLE_NAME} ${EXECUTABLE_ARGS})
    ENDIF(MPI_TEST_EXECUTABLE)
  ENDIF(TEST_EXECUTABLE)

ENDMACRO(TRILINOS_ADD_EXECUTABLE) 

