INCLUDE(TribitsSubPackageMacros)

TRIBITS_SUBPACKAGE(Comm)

ADD_SUBDIRECTORY(src)

TRIBITS_ADD_TEST_DIRECTORIES(test)

TRIBITS_ADD_TEST_DIRECTORIES(example)

TRIBITS_SUBPACKAGE_POSTPROCESS()
