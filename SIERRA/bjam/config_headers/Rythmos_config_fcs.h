
#define HAVE_MPI

/* #undef HAVE_RYTHMOS_EXPERIMENTAL */

/* #undef HAVE_RYTHMOS_EXPLICIT_INSTANTIATION */

/* #undef HAVE_RYTHMOS_DEBUG */

#define Rythmos_ENABLE_Sacado

#define Rythmos_ENABLE_NOX

#define Rythmos_ENABLE_Stratimikos

#ifndef RYTHMOS_FUNC_TIME_MONITOR
#  define RYTHMOS_TEUCHOS_TIME_MONITOR
#  define RYTHMOS_FUNC_TIME_MONITOR(FUNCNAME) \
     TEUCHOS_FUNC_TIME_MONITOR_DIFF(FUNCNAME, RYTHMOS)
#  define RYTHMOS_FUNC_TIME_MONITOR_DIFF(FUNCNAME, DIFF) \
     TEUCHOS_FUNC_TIME_MONITOR_DIFF(FUNCNAME, DIFF)
#endif

