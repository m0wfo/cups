// Enable access to IPP private structures post Cups 1.5
// (In Mac OS 10.8 thru 10.9 specifically enabled by Apple but not 10.10)
#define _IPP_PRIVATE_STRUCTURES 1
#include <cups/cups.h>

// st.h is needed for ST_CONTINUE constant
#include <ruby.h>
#include <st.h>

#ifndef MAXOPTS
  #define MAXOPTS 100
#endif
