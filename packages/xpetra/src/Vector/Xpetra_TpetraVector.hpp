// @HEADER
//
// ***********************************************************************
//
//             Xpetra: A linear algebra interface package
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact
//                    Jeremie Gaidamour (jngaida@sandia.gov)
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef XPETRA_TPETRAVECTOR_HPP
#define XPETRA_TPETRAVECTOR_HPP

/* this file is automatically generated - do not edit (see script/tpetra.py) */

#include "Xpetra_TpetraConfigDefs.hpp"

#include "Xpetra_Vector.hpp"
#include "Xpetra_MultiVector.hpp"
#include "Xpetra_TpetraMultiVector.hpp"

#include "Xpetra_TpetraMap.hpp" //TMP
#include "Xpetra_Utils.hpp"
#include "Xpetra_TpetraImport.hpp"
#include "Xpetra_TpetraExport.hpp"

#include "Tpetra_Vector.hpp"

namespace Xpetra {

  // TODO: move that elsewhere
  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
  Tpetra::Vector<Scalar,LocalOrdinal, GlobalOrdinal, Node> & toTpetra(Vector<Scalar,LocalOrdinal, GlobalOrdinal, Node> &);

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
  const Tpetra::Vector<Scalar,LocalOrdinal, GlobalOrdinal, Node> & toTpetra(const Vector<Scalar,LocalOrdinal, GlobalOrdinal, Node> &);
  //

  template <class Scalar, class LocalOrdinal = int, class GlobalOrdinal = LocalOrdinal, class Node = Kokkos::DefaultNode::DefaultNodeType>
  class TpetraVector
    : public virtual Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node>, public TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>
  {

  public:

    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::dot;          // overloading, not hiding
    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::norm1;        // overloading, not hiding
    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::norm2;        // overloading, not hiding
    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::normInf;      // overloading, not hiding
    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::normWeighted; // overloading, not hiding
    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::meanValue;    // overloading, not hiding
    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::replaceGlobalValue;    // overloading, not hiding
    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::sumIntoGlobalValue;    // overloading, not hiding
    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::replaceLocalValue;    // overloading, not hiding
    using TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::sumIntoLocalValue;    // overloading, not hiding

    //! @name Constructor/Destructor Methods
    //@{ 

    //! Sets all vector entries to zero.
    TpetraVector(const Teuchos::RCP< const Map<LocalOrdinal,GlobalOrdinal,Node> > &map, bool zeroOut=true)
      : TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >(map,1,zeroOut) { }
    
    //! Set multi-vector values from an array using Teuchos memory management classes. (copy)
    TpetraVector(const Teuchos::RCP< const Map<LocalOrdinal,GlobalOrdinal,Node> > &map, const Teuchos::ArrayView< const Scalar > &A)
      : TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >(map,A,map->getNodeNumElements(),1) { }

    //! Destructor.
    virtual ~TpetraVector() { }

    //@}

    //! @name Post-construction modification routines
    //@{

    //! Replace current value at the specified location with specified value.
    void replaceGlobalValue(GlobalOrdinal globalRow, const Scalar &value) { XPETRA_MONITOR("TpetraVector::replaceGlobalValue"); getTpetra_Vector()->replaceGlobalValue(globalRow, value); }

    //! Adds specified value to existing value at the specified location.
    void sumIntoGlobalValue(GlobalOrdinal globalRow, const Scalar &value) { XPETRA_MONITOR("TpetraVector::sumIntoGlobalValue"); getTpetra_Vector()->sumIntoGlobalValue(globalRow, value); }

    //! Replace current value at the specified location with specified values.
    void replaceLocalValue(LocalOrdinal myRow, const Scalar &value) { XPETRA_MONITOR("TpetraVector::replaceLocalValue"); getTpetra_Vector()->replaceLocalValue(myRow, value); }

    //! Adds specified value to existing value at the specified location.
    void sumIntoLocalValue(LocalOrdinal myRow, const Scalar &value) { XPETRA_MONITOR("TpetraVector::sumIntoLocalValue"); getTpetra_Vector()->sumIntoLocalValue(myRow, value); }

    //@}

    //! @name Mathematical methods
    //@{

    //! Computes dot product of this Vector against input Vector x.
    Scalar dot(const Vector< Scalar, LocalOrdinal, GlobalOrdinal, Node > &a) const { XPETRA_MONITOR("TpetraVector::dot"); return getTpetra_Vector()->dot(toTpetra(a)); }

    //! Return 1-norm of this Vector.
    typename Teuchos::ScalarTraits< Scalar >::magnitudeType norm1() const { XPETRA_MONITOR("TpetraVector::norm1"); return getTpetra_Vector()->norm1(); }

    //! Compute 2-norm of this Vector.
    typename Teuchos::ScalarTraits< Scalar >::magnitudeType norm2() const { XPETRA_MONITOR("TpetraVector::norm2"); return getTpetra_Vector()->norm2(); }

    //! Compute Inf-norm of this Vector.
    typename Teuchos::ScalarTraits< Scalar >::magnitudeType normInf() const { XPETRA_MONITOR("TpetraVector::normInf"); return getTpetra_Vector()->normInf(); }

    //! Compute Weighted 2-norm (RMS Norm) of this Vector.
    typename Teuchos::ScalarTraits< Scalar >::magnitudeType normWeighted(const Vector< Scalar, LocalOrdinal, GlobalOrdinal, Node > &weights) const { XPETRA_MONITOR("TpetraVector::normWeighted"); return getTpetra_Vector()->normWeighted(toTpetra(weights)); }

    //! Compute mean (average) value of this Vector.
    Scalar meanValue() const { XPETRA_MONITOR("TpetraVector::meanValue"); return getTpetra_Vector()->meanValue(); }

    //@}

    //! @name Overridden from Teuchos::Describable
    //@{

    //! Return a simple one-line description of this object.
    std::string description() const { XPETRA_MONITOR("TpetraVector::description"); return getTpetra_Vector()->description(); }

    //! Print the object with some verbosity level to an FancyOStream object.
    void describe(Teuchos::FancyOStream &out, const Teuchos::EVerbosityLevel verbLevel=Teuchos::Describable::verbLevel_default) const { XPETRA_MONITOR("TpetraVector::describe"); getTpetra_Vector()->describe(out, verbLevel); }

    //@}

    //! @name Xpetra specific
    //@{

    //! TpetraMultiVector constructor to wrap a Tpetra::MultiVector object
    TpetraVector(const Teuchos::RCP<Tpetra::Vector< Scalar, LocalOrdinal, GlobalOrdinal, Node> > &vec) : TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >(vec) { } // TODO: removed const of Tpetra::Vector

    //! Get the underlying Tpetra multivector
    RCP< Tpetra::Vector< Scalar, LocalOrdinal, GlobalOrdinal, Node> > getTpetra_Vector() const { return this->TpetraMultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >::getTpetra_MultiVector()->getVectorNonConst(0); }

    //@}
    
  }; // TpetraVector class

  // TODO: move that elsewhere
  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
  Tpetra::Vector< Scalar,LocalOrdinal, GlobalOrdinal, Node> & toTpetra(Vector< Scalar,LocalOrdinal, GlobalOrdinal, Node> &x) {
    typedef TpetraVector< Scalar, LocalOrdinal, GlobalOrdinal, Node > TpetraVectorClass;
    XPETRA_DYNAMIC_CAST(      TpetraVectorClass, x, tX, "toTpetra");
    return *tX.getTpetra_Vector();
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
  const Tpetra::Vector< Scalar,LocalOrdinal, GlobalOrdinal, Node> & toTpetra(const Vector< Scalar,LocalOrdinal, GlobalOrdinal, Node> &x) {
    typedef TpetraVector< Scalar, LocalOrdinal, GlobalOrdinal, Node > TpetraVectorClass;
    XPETRA_DYNAMIC_CAST(const TpetraVectorClass, x, tX, "toTpetra");
    return *tX.getTpetra_Vector();
  }
  //

} // Xpetra namespace

#define XPETRA_TPETRAVECTOR_SHORT
#endif // XPETRA_TPETRAVECTOR_HPP
