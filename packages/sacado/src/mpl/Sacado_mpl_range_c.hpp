// $Id$ 
// $Source$ 
// @HEADER
// ***********************************************************************
// 
//                           Sacado Package
//                 Copyright (2006) Sandia Corporation
// 
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact David M. Gay (dmgay@sandia.gov) or Eric T. Phipps
// (etphipp@sandia.gov).
// 
// ***********************************************************************
// @HEADER

#ifndef SACADO_MPL_RANGE_C_HPP
#define SACADO_MPL_RANGE_C_HPP

#include "Sacado_mpl_none.hpp"
#include "Sacado_mpl_size.hpp"
#include "Sacado_mpl_begin.hpp"
#include "Sacado_mpl_end.hpp"
#include "Sacado_mpl_next.hpp"
#include "Sacado_mpl_at.hpp"
#include "Sacado_mpl_deref.hpp"
#include "Sacado_mpl_integral_c.hpp"

namespace Sacado {

  namespace mpl {

    // range_c tag for mpl operations
    struct range_c_tag {};

    // range_c
    template <class T, T N, T M>
    struct range_c {
      typedef range_c_tag tag;
      typedef range_c type;
      static const int sz = M-N;
      typedef T integral_type;
      static const int start_value = N;
      static const int end_value = M;
    };
    
    // iterator
    template <class Range, int Pos>
    struct range_c_iterator {
      static const int value = Pos;
    };

    // size
    template <> 
    struct size_impl<range_c_tag> {
      template <class Range>
      struct apply {
	static const int value = Range::sz;
      };
    };

    // begin
    template <> 
    struct begin_impl<range_c_tag> {
      template <class Range>
      struct apply {
	typedef range_c_iterator<Range,0> type;
      };
    };

    // end
    template <> 
    struct end_impl<range_c_tag> {
      template <class Range>
      struct apply {
	typedef range_c_iterator<Range,Range::sz> type;
      };
    };

    // next
    template <class Range, int Pos>
    struct next< range_c_iterator<Range,Pos> > {
      typedef range_c_iterator<Range,Pos+1> type;
    };

    

    // at
    template <int Pos> 
    struct at_impl<range_c_tag, Pos> {
      template <class Range>
      struct apply {
	typedef integral_c<typename Range::integral_type, Range::start_value+Pos> type;
      };
    };

    // deref
    template <class Range, int Pos>
    struct deref< range_c_iterator<Range,Pos> > : mpl::at<Range,Pos> {};
    
  }
}

#endif
