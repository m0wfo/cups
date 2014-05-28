#include <cups/cups.h>

// st.h is needed for ST_CONTINUE constant
#ifdef __APPLE__
  #include <ruby.h>
  #include <st.h>
#else
  #include <ruby.h>
  #include <st.h>
#endif

#ifndef MAXOPTS
  #define MAXOPTS 100
#endif
