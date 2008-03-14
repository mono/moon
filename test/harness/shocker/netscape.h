
#ifndef __NETSCAPE_H__
#define __NETSCAPE_H__


#include <stdio.h>
#include "npapi.h"
#include "npupp.h"


#define NP_VERSION_MIN_SUPPORTED  13
#define NP_VERSION_HAS_RUNTIME    14
#define NP_VERSION_HAS_POPUP      16

#define STR_FROM_VARIANT(v) ((char *) NPVARIANT_TO_STRING (v).utf8characters)



#endif  // __NETSCAPE_H__


