#
# Build all Secondary Stable CASL VRI add-on Trilnos packages with GCC 4.5.1 compiler
#

INCLUDE("${CTEST_SCRIPT_DIRECTORY}/TrilinosCTestDriverCore.pu241.gcc.4.5.1.cmake")
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/SubmitToCaslDev.cmake")
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/casl-vri-packages-coupled.cmake")

SET(COMM_TYPE MPI)
SET(BUILD_TYPE DEBUG)
SET(BUILD_DIR_NAME MPI_DEBUG_GCC_CASLDEV)
SET(CTEST_TEST_TYPE Continuous)
#SET(CTEST_TEST_TIMEOUT 900)
SET(Trilinos_PACKAGES STARCCM DeCART CASLBOA LIME VRIPSS)
SET(Trilinos_ENABLE_SECONDARY_STABLE_CODE ON)
SET(EXTRA_CONFIGURE_OPTIONS
  -DTrilinos_ENABLE_CASLRAVE=OFF
  ${EXTRA_CONFIGURE_OPTIONS}
  )

TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER()
