// @HEADER
// ***********************************************************************
// 
//    Thyra: Interfaces and Support for Abstract Numerical Algorithms
//                 Copyright (2004) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
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
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ***********************************************************************
// @HEADER

#ifndef THYRA_LINEAR_OP_TESTER_DECL_HPP
#define THYRA_LINEAR_OP_TESTER_DECL_HPP

#include "Thyra_OperatorVectorTypes.hpp"
#include "Teuchos_ScalarTraits.hpp"
#include "Teuchos_StandardMemberCompositionMacros.hpp"

namespace Thyra {

/** \brief Testing class for <tt>LinearOpBase</tt>.
 *
 * This testing class performs many different tests just given a
 * <tt>LinearOpBase</tt> object using the function <tt>check()</tt>.
 *
 * This testing class also can check if two linear operators are the same
 * using random vectors by using the function <tt>compare()</tt>.
 *
 * ToDo: Finish documentation!
 *
 * The default compilier-generated copy constructor and assignment operators
 * are allowed since they have the correct simantics which are to simply copy
 * control parameters.
 *
 * \ingroup Thyra_Op_Vec_ANA_Development_grp
 */
template<class Scalar>
class LinearOpTester {
public:

  /** \brief Local typedef for scalar magnitude */
  typedef typename Teuchos::ScalarTraits<Scalar>::magnitudeType ScalarMag;

  /** \brief Set if to check for linear properties <tt>alpha*op*(x + y) ==
   * op(alpha*x) + op(alpha*y)</tt>
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( bool, check_linear_properties )

  /** \brief Set the tolerance above which a relative error will generate a
   * warning message for the check of the linear properties.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( ScalarMag, linear_properties_warning_tol )

  /** \brief Set the tolerance above which a relative error will generate a
   * error message and result in test failure for the check of the linear
   * properties.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( ScalarMag, linear_properties_error_tol )

  /** \brief Set if to check for adjoint property <tt>x'*(op*y) ==
   * y'*(op'*x)</tt> if adjoint is supported.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( bool, check_adjoint  )

  /** \brief Set the tolerance above which a relative error will generate a
   * warning message for the check of the adjoint.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( ScalarMag, adjoint_warning_tol )

  /** \brief Set the tolerance above which a relative error will generate a
   * error message and result in test failure for the check of the adjoint.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( ScalarMag, adjoint_error_tol )

  /** \brief Set if to check for symmetry property <tt>x'*(op*y) ==
   * y'*(op*x)</tt> for symmetric operators.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( bool, check_for_symmetry  )

  /** \brief Set the tolerance above which a relative error will generate a
   * warning message for the check of symmetry.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( ScalarMag, symmetry_warning_tol )

  /** \brief Set the tolerance above which a relative error will generate a
   * error message and result in test failure for the check of symmetry.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( ScalarMag, symmetry_error_tol )

  /** \brief Set the number random vectors that is generated during each test.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( int, num_random_vectors )

  /** \brief Set if all tests are shown or just summaries.
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( bool, show_all_tests )

  /** \brief Set if all of the vectors are dumped or not (only relevant if
   * <tt>show_all_tests()==true</tt>).
   */
  STANDARD_MEMBER_COMPOSITION_MEMBERS( bool, dump_all )

  /** \breif Default constructor which sets default parameter values.
   *
   * Note: It is not recommended that the client pass in values in this
   * constructor since the argument list may change in the near future but
   * instead use the above set functions to change an option after
   * construction.
   */
  LinearOpTester(
    const bool          check_linear_properties         = true
    ,const ScalarMag    linear_properties_warning_tol   = 1e-13
    ,const ScalarMag    linear_properties_error_tol     = 1e-10
    ,const bool         check_adjoint                   = true
    ,const ScalarMag    adjoint_warning_tol             = 1e-13
    ,const ScalarMag    adjoint_error_tol               = 1e-10
    ,const bool         check_for_symmetry              = false
    ,const ScalarMag    symmetry_warning_tol            = 1e-13
    ,const ScalarMag    symmetry_error_tol              = 1e-10
    ,const int          num_random_vectors              = 1
    ,const bool         show_all_tests                  = false
    ,const bool         dump_all                        = false
    );

  /** \brief Set all the warning tolerances to the same value.
   *
   * Postconditions:<ul>
   * <li><tt>this->linear_properties_warning_tol()==warning_tol</tt>
   * <li><tt>this->adjoint_warning_tol()==warning_tol</tt>
   * <li><tt>this->symmetry_warning_tol()==warning_tol</tt>
   * </ul>
   */
  void set_all_warning_tol( const ScalarMag warning_tol );

  /** \brief Set all the error tolerances to the same value.
   *
   * Postconditions:<ul>
   * <li><tt>this->linear_properties_error_tol()==error_tol</tt>
   * <li><tt>this->adjoint_error_tol()==error_tol</tt>
   * <li><tt>this->symmetry_error_tol()==error_tol</tt>
   * </ul>
   */
  void set_all_error_tol( const ScalarMag error_tol );

  /** \brief Check a linear operator.
   *
   * @param  op    [in] The linear operator to check.
   * @param  out   [in/out] If <tt>out!=NULL</tt> then trace output
   *               about the tests performed will be sent to <tt>*out</tt>.
   * @param  leadingIndent [in] All output to <tt>*out</tt> will insert this spacer
   *                      before each new line is printed.  Default value <tt>""</tt>.
   * @param  indentSpacer [in] All output to <tt>*out</tt> that is further indented
   8                      will use this indentation.  Default value <tt>"  "</tt>.
   *
   * This function performs a number of tests on <tt>op</tt>:<ul>
   *
   * <li>Checks that the domain and range spaces are valid.
   *
   * <li>Creates temporary vectors using the domain and range spaces.
   *
   * <li>If <tt>this->check_linear_properties()==true</tt> then checks that
   *     \f$\alpha A ( u  + v ) = \alpha A u + \alpha A v\f$ to a
   *     relative tolerance defined by <tt>error_tol()</tt>.  Note, if the clinet
   *     wants to check the linear properties of the adjoint then the client
   *     should first create an implicit adjoint using <tt>adjoint()</tt>
   *     (or transpose using <tt>transpose()</tt>) which wraps the operation in
   *     a <tt>ScaledAdjointLinearOp</tt>.  Using this method a client can check
   *     all the various values of the enum <tt>ETransp</tt>.
   *
   * <li>If <tt>this->check_adjoint()==true</tt> then, checks that the non-transposed
   *     operator and the adjoint operator agree.
   *     The operator and adjoint operator must obey the defined
   *     scalar product.  Specifically, for any two vectors \f$w\f$ (in the
   *     domain space \f$\mathcal{D}\f$) and \f$u\f$ (in the range space
   *     \f$\mathcal{R}\f$), the adjoint operation must obey:
   *     \f[<u,A v>_{\mathcal{R}} = <A^H u, v>_{\mathcal{D}}\f]
   *     where \f$<.,.>_{\mathcal{R}}\f$ is the scalar product defined by
   *     <tt>op.range()->scalarProd()</tt> and \f$<.,.>_{\mathcal{D}}\f$
   *     is the scalar product defined by
   *     <tt>op.domain()->scalarProd()</tt>.
   *
   * <li>If <tt>this->check_for_symmetry()==true</tt> the the operator will
   *     be checked to see if it is symmetric.  Specifically, for any two random
   *     vectors \f$w\f$ and \f$u\f$ in the operator's space \f$\mathcal{S}\f$,
   *     the following property is checked:
   *     \f[<u,A v>_{\mathcal{S}} = <A u, v>_{\mathcal{S}}\f]
   *     where \f$<.,.>_{\mathcal{S}}\f$ is the scalar product defined by
   *     <tt>op.domain()->scalarProd()</tt> and <tt>op.domain()->scalarProd()</tt>.
   *
   * </ul>
   *
   * All relative errors that exceed <tt>xxx_warning_tol()</tt> but do not
   * exceed <tt>xxx_error_tol</tt> will result in special warning messages
   * printed to <tt>*out</tt> (if <tt>out!=NULL</tt>).
   *
   * @return The function returns <tt>true</tt> if all of the tests
   * where within the <tt>xxx_error_tol()</tt> and returns <tt>false</tt>
   * if not.
   *
   * The best way to see what this testing function is doing is to run
   * the test with <tt>out!=NULL</tt> and to look at the
   * implementation by clicking on the following link to the source code:
   */
  bool check(
    const LinearOpBase<Scalar>  &op
    ,std::ostream               *out
    ,const std::string          &leadingIndent  = ""
    ,const std::string          &indentSpacer   = "  "
    ) const;


  /** \brief Check if two linear operators are the same or not.
   *
   * @param  op1    [in] The first linear operator
   * @param  op2    [in] The second linear operator
   * @param  out    [in/out] If <tt>out!=NULL</tt> then trace output
   *                about the tests performed will be sent to <tt>*out</tt>.
   * @param  leadingIndent [in] All output to <tt>*out</tt> will insert this spacer
   *                      before each new line is printed.  Default value <tt>""</tt>.
   * @param  indentSpacer [in] All output to <tt>*out</tt> that is further indented
   8                      will use this indentation.  Default value <tt>"  "</tt>.
   *
   * This function checks if <tt>op1</tt> and <tt>op2</tt> are the same by
   * checking that the range and domain spaces are compatible and then
   * checking that <tt>sum(op1*v) == sum(op2*v)</tt> for various random
   * vectors.  The allowed warning and error tolerances are taken from
   * <tt>linear_properties_warning_tol()</tt> and
   * <tt>linear_properties_error_tol()</tt>.
   *
   * All relative errors that exceed <tt>xxx_warning_tol()</tt> but do not
   * exceed <tt>xxx_error_tol</tt> will result in special warning messages
   * printed to <tt>*out</tt> (if <tt>out!=NULL</tt>).
   *
   * @return The function returns <tt>true</tt> if all of the tests
   * where within the <tt>xxx_error_tol()</tt> and returns <tt>false</tt>
   * if not.
   *
   * The best way to see what this testing function is doing is to run
   * the test with <tt>out!=NULL</tt> and to look at the
   * implementation by clicking on the following link to the source code:
   */
  bool compare(
    const LinearOpBase<Scalar>  &op1
    ,const LinearOpBase<Scalar> &op2
    ,std::ostream               *out
    ,const std::string          &leadingIndent  = ""
    ,const std::string          &indentSpacer   = "  "
    ) const;

}; // class LinearOpTester

} // namespace Thyra

#endif // THYRA_LINEAR_OP_TESTER_DECL_HPP
