/* This file includes the amiga specific expat files so 
   expat using is transparent */

#ifdef _AROS
#include <expat.h>
#else
#include <expat/expat.h>
#include <proto/expat.h>
#endif
