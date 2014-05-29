#include <cups/cups.h>

// st.h is needed for ST_CONTINUE constant
#ifdef __APPLE__
  #include <ruby/ruby.h>
  #include <ruby/st.h>
#else
  #include <ruby.h>
  #include <st.h>
#endif

#ifndef MAXOPTS
  #define MAXOPTS 100
#endif
