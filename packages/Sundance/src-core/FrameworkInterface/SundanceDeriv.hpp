/* @HEADER@ */
// ************************************************************************
// 
//                              Sundance
//                 Copyright (2005) Sandia Corporation
// 
// Copyright (year first published) Sandia Corporation.  Under the terms 
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government 
// retains certain rights in this software.
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
// Questions? Contact Kevin Long (krlong@sandia.gov), 
// Sandia National Laboratories, Livermore, California, USA
// 
// ************************************************************************
/* @HEADER@ */

#ifndef SUNDANCE_DERIV_H
#define SUNDANCE_DERIV_H

#include "SundanceDefs.hpp"
#include "SundanceSpatialDerivSpecifier.hpp"
#include "SundanceFunctionIdentifier.hpp"
#include "Teuchos_RefCountPtr.hpp"

namespace SundanceCore
{
using namespace SundanceUtils;
using namespace Teuchos;
class SymbolicFunc;
class SymbolicFuncElement;
class SymbolicFuncDescriptor;
class CommonFuncDataStub;

enum DerivType {CoordDT, FunctionalDT, NullDT};

/**
 * The purpose of this class is to represent first-order 
 * spatial or functional differential operators. This object 
 * should not be confused with the user-level Derivative
 * object, which represents spatial differential operators as appearing
 * in a user-level problem specification.
 *
 * <h4> Functional vs spatial derivatives </h4>
 *
 * The bridge theorem involves expressions of the form
 * \f[
 * \frac{\partial \mathcal{F}}{\partial L(u)} L(\phi_u)
 * \f]
 * and
 * \f[
 * \frac{\partial^2 \mathcal{F}}{\partial L(u)\partial M(v)} 
 * L(\phi_u)M(\psi_v)
 * \f]
 * where \f$L(u)\f$ is some spatial differential operator acting on the
 * function \f$u\f$, and \f$\phi_u\f$ is a basis function with which
 * \f$u\f$ is represented.
 * A derivative such as \f$\frac{\partial \mathcal{F}}{\partial L(u)}\f$ will
 * be called a functional derivative to distinguish it from derivatives
 * with respect to spatial coordinates, such as 
 * \f$\frac{\partial \mathcal{F}}{\partial x}\f$.
 * The following table shows the types of derivative currently supported.
 * <table>
 * <tr> 
 * <td>Operation</td>
 * <td>Geometry of derivative value</td> 
 * <td>Operating function geometry</td> 
 * <td>Operating spatial operator</td>
 * <td>Operating function subtype</td>
 * </tr>
 * <tr> 
 * <td>\f$ \frac{\partial}{\partial x}\f$ </td>
 * <td> Scalar </td>
 * <td> Coordinate component </td> 
 * <td> N/A </td>
 * <td> N/A </td> 
 * </tr>
 * <tr> 
 * <td>\f$ \frac{\partial}{\partial p}\f$ </td>
 * <td> Scalar </td>
 * <td> Scalar (\f$L^2\f$)</td> 
 * <td> Identity </td>
 * <td> SymbolicFuncElement </td> 
 * </tr>
 * <tr> 
 * <td>\f$ \frac{\partial}{\partial D_\alpha p}\f$ </td>
 * <td> Coordinate component </td>
 * <td> Scalar (\f$H^1\f$)</td> 
 * <td> Partial derivative \f$D_\alpha\f$ </td>
 * <td> SymbolicFuncElement </td> 
 * </tr>
 * <tr> 
 * <td>\f$ \frac{\partial}{\partial D_n p}\f$ </td>
 * <td> Normal component </td>
 * <td> Scalar (\f$H^1\f$) </td> 
 * <td> Normal derivative \f${\bf n}\cdot \nabla\f$ </td>
 * <td> SymbolicFuncElement </td> 
 * </tr>
 * <tr> 
 * <td>\f$ \frac{\partial}{\partial \mathrm{div}({\bf u})}\f$ </td>
 * <td> Scalar </td>
 * <td> Vector (\f${\bf H}(div)\f$)</td>
 * <td> Divergence </td> 
 * <td> SymbolicFunc </td> 
 * </tr>
 * <tr> 
 * <td>\f$ \frac{\partial}{\partial {\bf u\cdot e_\alpha}}\f$ </td>
 * <td> Coordinate component </td>
 * <td> Vector (\f${\bf H}^1\f$, \f${\bf H}(div)\f$, or \f${\bf H}(curl)\f$)</td> 
 * <td> Inner product with coordinate unit vector </td> 
 * <td> SymbolicFuncElement </td> 
 * </tr>
 * <tr> 
 * <td>\f$ \frac{\partial}{\partial {\bf u\cdot n}}\f$ </td>
 * <td> Normal component </td>
 * <td> Vector (\f${\bf H}^1\f$ or \f${\bf H}(div)\f$)</td> 
 * <td> Inner product with normal unit vector </td> 
 * <td> SymbolicFunc </td> 
 * </tr>
 * <tr> 
 * <td>\f$ \frac{\partial}{\partial D_\alpha ({\bf u\cdot e_\beta})}\f$ </td>
 * <td> Coordinate component </td>
 * <td> Vector (\f${\bf H}^1\f$) </td> 
 * <td> Inner product with normal unit vector </td> 
 * <td> SymbolicFuncElement </td> 
 * </tr>
 * </table>
 */
class Deriv : public EnumTypeField<DerivType>
{
public:
  /** An empty ctor is needed for use with std::vector. The empty
   * ctor creates a null derivative, representing an identity operator.*/
  Deriv();

  /** Construct a deriv wrt a coordinate, e.g., \f$D_x\f$. */
  Deriv(int coordDir);

  /** Construct a deriv wrt an object of length one, for example,
   * a scalar function or a single component of a vector function, or
   * possibly a spatial derivative of such an object. 
   * Examples: 
   * <ul>
   * <li> \f$T\f$ is a scalar-valued function
   * */
  Deriv(
    const SymbolicFuncElement* func,
    const SpatialDerivSpecifier& d
  );

  /** Construct a deriv wrt a function */
  Deriv( 
    const SymbolicFunc* func,
    const SpatialDerivSpecifier& d
    );

  /** Comparison operator for use in sets and maps */
  bool operator<(const Deriv& other) const ;

  /** Equality test operator */
  bool operator==(const Deriv& other) const ;

  /** Write to a string
   * \param verbose If true, write all details for the object. If false, 
   * write a simple description giving only the function name and the
   * specification of a derivative.
   */
  std::string toString(bool verbose=false) const ;

  /** True if this is a derivative wrt a spatial coordinate */
  bool isCoordDeriv() const {return type()==CoordDT;}

  /** True if this is a derivative wrt a function 
   * or a spatial derivative of a function */
  bool isFunctionalDeriv() const {return type()==FunctionalDT;}

  /** True if my operative function is a test function */
  bool isTestFunction() const ;

  /** True if my operative function is unknown */
  bool isUnknownFunction() const ;

  /** Create a new functional derivative in which the function
   * has been differentiated spatially by the given multi index, for example,
   * <tt>derivWrtMultiIndex(MultiIndex(0,0,1))</tt> transforms 
   * \f$\frac{\partial}{\partial p}\f$ to 
   * \f$\frac{\partial}{\partial D_z p}\f$
   * */
  Deriv derivWrtMultiIndex(const MultiIndex& mi) const ;

  /** Whether it's possible to take a spatial derivative of this object */
  bool canBeDifferentiated() const ;

  /** \brief Return the DOF ID of my operative function, for 
   example, if I am \f$\frac{\partial}{\partial D_y u_x}\f$ then 
   return <tt>dofID</tt>\f$({\bf u})\f$.
   If I'm not a functional derivative,
   throw an error.  
  */
  int dofID() const ;

  /** Return the ID for my operative function */
  const FunctionIdentifier& fid() const ;
  
  /** Return the spatial derivative acting on my operative function */
  const SpatialDerivSpecifier& opOnFunc() const ;

  /** Indicate my algebra type */
  const AlgebraSpecifier& algSpec() const {return myAlgSpec_;}

  /** Indicate the algebra type of my operative function */
  const AlgebraSpecifier& funcAlgSpec() const ;

  /** Return a pointer to my function's data */
  const RCP<const CommonFuncDataStub>& data() const ;

  /** If I am a coordinate derivative, return my direction */
  int coordDerivDir() const ;

  /** Return a pointer to my operative function */
  const SymbolicFuncElement* symbFuncElem() const {return symbFuncElem_;}

  /** Return a pointer to my operative function */
  const SymbolicFunc* symbFunc() const {return symbFunc_;}

private:
  /** */
  static AlgebraSpecifier derivAlgSpec(const AlgebraSpecifier& a,
    const SpatialDerivSpecifier& sds);

  /** */
  const SymbolicFuncDescriptor* sfdPtr() const ;

  /** */
  void checkConsistencyOfOperations() const ;

  AlgebraSpecifier myAlgSpec_;

  FunctionIdentifier fid_;

  SpatialDerivSpecifier sds_;

  const SymbolicFuncElement* symbFuncElem_;

  const SymbolicFunc* symbFunc_;

  int coordDerivDir_;

};

/** \relates Deriv */
Deriv coordDeriv(int d);

/** \relates Deriv */
Deriv coordDeriv(const MultiIndex& mi);

/** \relates Deriv */
Deriv funcDeriv(const SymbolicFuncElement* symbFunc);

/** \relates Deriv */
Deriv funcDeriv(const SymbolicFuncElement* symbFunc, const MultiIndex& mi);

/** \relates Deriv */
Deriv funcDeriv(const SymbolicFunc* symbFunc);

/** \relates Deriv */
Deriv funcDeriv(const SymbolicFunc* symbFunc, const MultiIndex& mi);

/** \relates Deriv */
Deriv normalDeriv(const SymbolicFuncElement* symbFunc);

Deriv divergenceDeriv(const SymbolicFunc* symbFunc);

 
}

namespace std
{
/** \relates Deriv */
inline ostream& operator<<(ostream& os,  const SundanceCore::Deriv& d)
{
  os << d.toString(); return os;
}
}


#endif
