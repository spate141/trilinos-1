// //////////////////////////////////////////////////////////////
// check_nan_inf.cpp
//
// Copyright (C) 2001 Roscoe Ainsworth Bartlett
//
// This is free software; you can redistribute it and/or modify it
// under the terms of the "Artistic License" (see the web site
//   http://www.opensource.org/licenses/artistic-license.html).
// This license is spelled out in the file COPYING.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// above mentioned "Artistic License" for more details.

#include <float.h>
#include <math.h>

#include "check_nan_inf.h"

// Computed values to compare to
static RTOp_value_type
    pos_inf = +1.0/sin(0.0),
    neg_inf = -1.0/sin(0.0),
    pos_nan = +0.0/sin(0.0),
    neg_nan = -0.0/sin(0.0);

int RTOp_is_nan( RTOp_value_type val )
{
#if defined(_INTEL_CXX)
  return _isnan(val) != 0;
#else
  return val == pos_nan || val == neg_nan || val != val;
#endif
}

int RTOp_is_inf( RTOp_value_type val )
{
    return val == pos_inf || val == neg_inf; // IEEE math
}

int RTOp_is_nan_inf( RTOp_value_type val )
{
  return
#if defined(_INTEL_CXX)
    _isnan(val) != 0
#else
	  val == pos_nan || val == neg_nan || val != val
#endif
	  || val == pos_inf || val == neg_inf;
}
