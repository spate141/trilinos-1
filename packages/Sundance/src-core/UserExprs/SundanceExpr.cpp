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

#include "SundanceExpr.hpp"
#include "SundanceListExpr.hpp"
#include "SundanceSumExpr.hpp"
#include "SundanceProductExpr.hpp"
#include "SundanceConstantExpr.hpp"
#include "SundanceCoordExpr.hpp"
#include "SundanceDerivative.hpp"
#include "SundanceDiffOp.hpp"


#include "SundanceUnaryMinus.hpp"
#include "SundanceZeroExpr.hpp"
#include "SundanceComplexExpr.hpp"
#include "SundanceOut.hpp"
#include "SundanceTabs.hpp"
#include "SundanceStdSumTransformations.hpp"
#include "SundanceStdProductTransformations.hpp"
#include "SundanceNonlinearUnaryOp.hpp"
#include "SundanceStdMathOps.hpp"
#include "SundanceParameter.hpp"
#include "SundanceSpectralBasis.hpp"
#include "SundanceSpectralExpr.hpp"
#include "SundanceUnknownFuncElement.hpp"
#include "SundanceTestFuncElement.hpp"
#include "Teuchos_TimeMonitor.hpp"

using namespace SundanceCore;
using namespace SundanceUtils;

using namespace Teuchos;
using namespace SundanceCore;

static Time& sumTimer() 
{
  static RefCountPtr<Time> rtn 
    = TimeMonitor::getNewTimer("symbolic sum"); 
  return *rtn;
}

static Time& unaryMinusTimer() 
{
  static RefCountPtr<Time> rtn 
    = TimeMonitor::getNewTimer("symbolic unary minus"); 
  return *rtn;
}

static Time& trySumSpectralTimer() 
{
  static RefCountPtr<Time> rtn 
    = TimeMonitor::getNewTimer("test for spectral sum"); 
  return *rtn;
}
static Time& tryMultiplySpectralTimer() 
{
  static RefCountPtr<Time> rtn 
    = TimeMonitor::getNewTimer("test for spectral product"); 
  return *rtn;
}

static Time& trySumComplexTimer() 
{
  static RefCountPtr<Time> rtn 
    = TimeMonitor::getNewTimer("test for complex sum"); 
  return *rtn;
}

static Time& tryMultiplyComplexTimer() 
{
  static RefCountPtr<Time> rtn 
    = TimeMonitor::getNewTimer("test for complex product"); 
  return *rtn;
}



static Time& prodTimer() 
{
  static RefCountPtr<Time> rtn 
    = TimeMonitor::getNewTimer("symbolic product"); 
  return *rtn;
}


Expr::Expr(const double& c)
	: TSFExtended::Handle<ExprBase>(new ConstantExpr(c))
{}

Expr::Expr(const complex<double>& c)
	: TSFExtended::Handle<ExprBase>(new ComplexExpr(new ConstantExpr(c.real()),
      new ConstantExpr(c.imag())))
{}

bool Expr::isComplex() const
{
  return dynamic_cast<const ComplexExpr*>(ptr().get()) != 0;
}

/*****************************************************************************************************************************************/

bool Expr::isSpectral() const
{
  return dynamic_cast<const SpectralExpr*>(ptr().get()) != 0;
}
/****************************************************************************************************************************************/



XMLObject Expr::toXML() const
{
  TimeMonitor t(outputTimer());

	return ptr()->toXML();
}

string Expr::toString() const 
{
  TimeMonitor t(outputTimer());

	TeuchosOStringStream ss;
	ptr()->toText(ss, false);
	return TEUCHOS_OSTRINGSTREAM_GET_C_STR(ss);
}

string Expr::toLatex() const 
{
  TimeMonitor t(outputTimer());

  TeuchosOStringStream ss;
	ptr()->toLatex(ss, false);
  //	ss << ends;
	return TEUCHOS_OSTRINGSTREAM_GET_C_STR(ss);
}

bool Expr::sameAs(const Expr& other) const 
{
  if (this->lessThan(other)) return false;
  if (other.lessThan(*this)) return false;
  return true;
}

bool Expr::operator<(const Expr& other) const 
{
  return this->lessThan(other);
}

bool Expr::lessThan(const Expr& other) const
{
  RefCountPtr<ScalarExpr> sThis = rcp_dynamic_cast<ScalarExpr>(ptr());
  RefCountPtr<ScalarExpr> sOther = rcp_dynamic_cast<ScalarExpr>(other.ptr());

  TEST_FOR_EXCEPTION(sThis.get()==0, InternalError,
    "ordering not defined for non-scalar expression "
    << toString());

  TEST_FOR_EXCEPTION(sOther.get()==0, InternalError,
    "ordering not defined for non-scalar expressions"
    << other.toString());

  
  const ConstantExpr* cMe = dynamic_cast<const ConstantExpr*>(sThis.get());
  const ConstantExpr* cOther = dynamic_cast<const ConstantExpr*>(sOther.get());

  /* constants are placed before any other expr type */
  if (cMe != 0 && cOther==0) return true;
  if (cOther != 0 && cMe==0) return false;
  if (cOther != 0 && cMe != 0) return cMe->lessThan(cOther);

  /* Move generic spatial constants, e.g., parameters to the left.
   * Because the values might change with time, we can't order on values.  */
  const SpatiallyConstantExpr* scMe 
    = dynamic_cast<const SpatiallyConstantExpr*>(sThis.get());
  const SpatiallyConstantExpr* scOther 
    = dynamic_cast<const SpatiallyConstantExpr*>(sOther.get());
  if (scMe != 0 && scOther==0) return true;
  if (scOther != 0 && scMe==0) return false;


  /* try ordering non-constant exprs by type name */
  if (sThis->typeName() < sOther->typeName()) return true;
  if (sThis->typeName() > sOther->typeName()) return false;

  /* if type names are the same, go to base class to do comparison */
  return sThis->lessThan(sOther.get());
}

SundanceUtils::Map<Expr, int> Expr::getSumTree() const
{
  RefCountPtr<ScalarExpr> sThis = rcp_dynamic_cast<ScalarExpr>(ptr());

  TEST_FOR_EXCEPTION(sThis.get()==0, InternalError,
    "etSumTree() not defined for non-scalar expression "
    << toString());
  
  const SumExpr* s = dynamic_cast<const SumExpr*>(sThis.get());
  const UnaryMinus* u = dynamic_cast<const UnaryMinus*>(sThis.get());
  if (s != 0)
  {
    return s->getSumTree();
  }
  else if (u != 0)
  {
    SundanceUtils::Map<Expr, int> rtn;
    rtn.put(u->arg(), -1);
    return rtn;
  }
  else
  {
    SundanceUtils::Map<Expr, int> rtn;
    rtn.put(*this, 1);
    return rtn;
  }
  
}

bool Expr::tryAddComplex(const Expr& L, const Expr& R, int sign,
  Expr& rtn) const
{
  TimeMonitor t(trySumComplexTimer());
  TEST_FOR_EXCEPTION(L.size() != 1 || R.size() != 1, InternalError, 
    "non-scalar exprs should have been reduced before "
    "call to tryAddComplex(). Left=" << L << " right="
    << R);
  if (L.isComplex() || R.isComplex())
  {
    if (sign > 0) 
    {
      rtn = new ComplexExpr(L.real() + R.real(), 
        L.imag() + R.imag());
    }
    else
    {
      rtn = new ComplexExpr(L.real() - R.real(), 
        L.imag() - R.imag());
    }
    return true;
  }
  else
  {
    return false;
  }
}



bool Expr::tryMultiplyComplex(const Expr& L, const Expr& R,
  Expr& rtn) const
{
  TimeMonitor t(tryMultiplyComplexTimer());
  TEST_FOR_EXCEPTION(L.size() != 1 || R.size() != 1, InternalError, 
    "non-scalar exprs should have been reduced before "
    "call to tryMultiplyComplex(). Left=" << L << " right="
    << R);

  if (L.isComplex() || R.isComplex())
  {
    if (Re(L).sameAs(Re(R)) && Im(L).sameAs(-Im(R)))
    {
      rtn = Re(R)*Re(R) + Im(R)*Im(R);
    }
    else
    {
      Expr re = Re(L)*Re(R) - Im(L)*Im(R);
      Expr im = Re(L)*Im(R) + Im(L)*Re(R);
      rtn = new ComplexExpr(re, im);
    }
    return true;
  }
  else
  {
    return false;
  }
}

bool Expr::tryAddSpectral(const Expr& L, const Expr& R, int sign,
  Expr& rtn) const
{
  TimeMonitor t(trySumSpectralTimer());
  TEST_FOR_EXCEPTION(L.size() != 1 || R.size() != 1, InternalError, 
    "non-scalar exprs should have been reduced before "
    "call to tryAddSpectral(). Left=" << L << " right="
    << R);
  
  const SpectralExpr* lse 
    = dynamic_cast<const SpectralExpr*>(L.ptr().get());
  
  const SpectralExpr* rse 
    = dynamic_cast<const SpectralExpr*>(R.ptr().get());
  
  if (lse != 0 && rse != 0) /* spectral + spectral */
  {
    SpectralBasis basis = lse->getSpectralBasis();
    Array<Expr> coeff(basis.nterms());
    for (int i=0; i<basis.nterms(); i++) 
    {
      if (sign > 0) coeff[i] = lse->getCoeff(i) + rse->getCoeff(i);
      else coeff[i] = lse->getCoeff(i) - rse->getCoeff(i);
    }
    rtn = new SpectralExpr(basis, coeff);
    return true;
  }
  else if (lse != 0) /* spectral + scalar */
  {
    SpectralBasis basis = lse->getSpectralBasis();
    Array<Expr> coeff(basis.nterms());
    if (sign > 0) coeff[0] = lse->getCoeff(0) + R;
    else coeff[0] = lse->getCoeff(0) - R;
    for (int i=1; i<basis.nterms(); i++) 
    {
      coeff[i] = lse->getCoeff(i);
    }
    rtn = new SpectralExpr(basis, coeff);
    return true;
  }
  else if (rse != 0) /* scalar + spectral */
  {

    SpectralBasis basis = rse->getSpectralBasis();
    Array<Expr> coeff(basis.nterms());
    if (sign > 0) coeff[0] = L + rse->getCoeff(0);
    else coeff[0] = L - rse->getCoeff(0);
    for (int i=1; i<basis.nterms(); i++) 
    {
      if (sign > 0) coeff[i] = rse->getCoeff(i);
      else coeff[i] = -rse->getCoeff(i);
    }
    rtn = new SpectralExpr(basis, coeff);
    return true;
  }
  else
  {
    /* if neither is spectral, do nothing here */
    return false;
  }
}



bool Expr::tryMultiplySpectral(const Expr& L, const Expr& R, 
  Expr& rtn) const
{
  TimeMonitor t(tryMultiplySpectralTimer());
  TEST_FOR_EXCEPTION(L.size() != 1 || R.size() != 1, InternalError, 
    "non-scalar exprs should have been reduced before "
    "call to tryMultiplySpectral(). Left=" << L << " right="
    << R);
  
  const SpectralExpr* lse 
    = dynamic_cast<const SpectralExpr*>(L.ptr().get());
  
  const SpectralExpr* rse 
    = dynamic_cast<const SpectralExpr*>(R.ptr().get());

  
  if (lse != 0 && rse != 0) /* spectral * spectral */
  {
    TEST_FOR_EXCEPTION(lse->hasTestFunctions() && rse->hasTestFunctions(),
      RuntimeError,
      "product of two test functions detected: L=" << L
      << ", R=" << R);
    if (rse->hasTestFunctions()) /* move test function to left */
    {
      return tryMultiplySpectral(R, L, rtn);
    }
    if (lse->hasTestFunctions()) /* if left has a test function, do dot product */
    {
      rtn = lse->spectralDotProduct(rse);
      return true;
    }
    SpectralBasis basis1 = lse->getSpectralBasis();
    SpectralBasis basis2 = rse->getSpectralBasis();
    Array<Expr> coeff(basis1.nterms());

    TEST_FOR_EXCEPTION((basis1.nterms() != basis2.nterms()) 
      || (basis1.getDim() != basis2.getDim()) 
      || (basis1.getOrder() != basis2.getOrder()), 
      InternalError,
      "The two spectral expressions should have the "
      "same basis ");
      
    for(int i=0; i<basis1.nterms(); i++)
    {
      coeff[i] = 0.0;
      for (int n=0; n<basis1.nterms(); n++)
      {
        for(int m=0; m<basis1.nterms(); m++)
        {
          double w = basis1.expectation(basis1.getElement(n),
            basis1.getElement(m), i)
            /basis1.expectation(0,i,i);
          if (w==0.0) continue;
          coeff[i] = coeff[i]+w*lse->getCoeff(n) * rse->getCoeff(m);
        }
      }
    }
    rtn = new SpectralExpr(basis1, coeff);
    return true;
  }
  else if (lse != 0) /* spectral * scalar */
  {
    SpectralBasis basis = lse->getSpectralBasis();
    Array<Expr> coeff(basis.nterms());
    for (int i=0; i<basis.nterms(); i++) 
    {
      coeff[i] = lse->getCoeff(i) * R;
    }
    rtn = new SpectralExpr(basis, coeff);
    return true;
  }
  else if (rse != 0) /* scalar * spectral */
  {
    SpectralBasis basis = rse->getSpectralBasis();
    Array<Expr> coeff(basis.nterms());
    for (int i=0; i<basis.nterms(); i++) 
    {
      coeff[i] = L * rse->getCoeff(i);
    }
    rtn = new SpectralExpr(basis, coeff);
    return true;
  }
  else
  {
    /* if neither is spectral, do nothing here */
    return false;
  }
}




Expr Expr::operator+(const Expr& other) const 
{
  TimeMonitor t(opTimer());
  TimeMonitor ts(sumTimer());

  /* Thread addition over list elements */
  if (this->size()!=1 || other.size()!=1)
  {
    TEUCHOS_FUNC_TIME_MONITOR("plus branch 1");
    Array<Expr> rtn(this->size());
    TEST_FOR_EXCEPTION(this->size() != other.size(), RuntimeError,
      "mismatched list structures in operands L="
      << *this << ", R=" << other);
    for (unsigned int i=0; i<this->size(); i++)
    {
      rtn[i] = (*this)[i] + other[i];
    }
    return new ListExpr(rtn);
  }
  else
  {
    TEUCHOS_FUNC_TIME_MONITOR("plus branch 2");
    Expr rtn;
    /* Try to thread addition over the coefficients of a spectral 
     * expansion */
    if (tryAddSpectral((*this)[0], other[0], 1, rtn)) return rtn;
    /* Try to thread addition over real and imaginary parts */
    if (tryAddComplex((*this)[0], other[0], 1, rtn)) return rtn;
    /* Otherwise, do a simple scalar-scalar sum */
    return (*this)[0].sum(other[0], 1);
  }
}

Expr Expr::operator-(const Expr& other) const 
{
  TimeMonitor t(opTimer());
  TimeMonitor ts(sumTimer());

  /* Thread subtraction over list elements */
  if (this->size()!=1 || other.size()!=1)
  {
    TEUCHOS_FUNC_TIME_MONITOR("minus branch 1");
    Array<Expr> rtn(this->size());
    TEST_FOR_EXCEPTION(this->size() != other.size(), RuntimeError,
      "mismatched list structures in operands L="
      << *this << ", R=" << other);
    for (unsigned int i=0; i<this->size(); i++)
    {
      rtn[i] = (*this)[i] - other[i];
    }
    return new ListExpr(rtn);
  }
  else
  {
    TEUCHOS_FUNC_TIME_MONITOR("minus branch 2");
    Expr rtn;
    /* Try to thread subtraction over the coefficients of a spectral 
     * expansion */
    if (tryAddSpectral((*this)[0], other[0], -1, rtn)) return rtn;
    /* Try to thread subtraction over real and imaginary parts */
    if (tryAddComplex((*this)[0], other[0], -1, rtn)) return rtn;
    /* Otherwise, do a simple scalar-scalar sum */
    return (*this)[0].sum(other[0], -1);
  }
}


Expr Expr::sum(const Expr& other, int sign) const 
{
  TEUCHOS_FUNC_TIME_MONITOR("Expr::sum()");
  RefCountPtr<ScalarExpr> rtn;
  RefCountPtr<ScalarExpr> sThis = rcp_dynamic_cast<ScalarExpr>(ptr());
  RefCountPtr<ScalarExpr> sOther = rcp_dynamic_cast<ScalarExpr>(other.ptr());

  TEST_FOR_EXCEPTION(ptr().get()==NULL, InternalError,
    "Expr::sum() detected null this pointer");

  TEST_FOR_EXCEPTION(sThis.get()==NULL, InternalError,
    "Expr::sum(): Left operand " << toString() 
    << " is a non-scalar expression. All list structure "
    "should have been handled before this point");

  TEST_FOR_EXCEPTION(sOther.get()==NULL, InternalError,
    "Expr::sum(): Right operand " << other.toString() 
    << " is a non-scalar expression. All list structure "
    "should have been handled before this point");

  static StdSumTransformations trans;

  if (trans.doTransform(sThis, sOther, sign, rtn)) 
  {
    if (SymbolicTransformation::classVerbosity() > 0)
    {
      Out::println("Expr::sum() transformed sum\n[" 
        + toString() + "+"
        + other.toString() + "]\n to\n ["
        + rtn->toString() + "]");
    }
    return handle(rtn);
  }

  else return new SumExpr(sThis, sOther, sign);
}


Expr Expr::operator*(const Expr& other) const 
{
  TimeMonitor t(opTimer());
  TimeMonitor tp(prodTimer());
  Tabs tab1;

  /* if both operands are simple scalars, multiply them */
  if (this->size() == 1 && other.size()==1)
  {
    Expr rtn;
    /* Try to thread multiplication over the coefficients of a spectral 
     * expansion */
    if (tryMultiplySpectral((*this)[0], other[0], rtn)) 
    {
      return rtn;
    }
    /* Try to do complex multiplication */
    if (tryMultiplyComplex((*this)[0], other[0], rtn)) 
    {
      return rtn;
    }
    /* Otherwise, do a simple scalar-scalar product */
    rtn = (*this)[0].multiply(other[0]);
    return rtn;
  }

  /* if the left operand is a scalar, multiply it through */
  if (this->size()==1)
  {
    Array<Expr> rtn(other.size());
    for (unsigned int i=0; i<other.size(); i++)
    {
      rtn[i] = (*this)[0] * other[i];
    }
    return new ListExpr(rtn);
  }

  /* if the right operand is a scalar, multiply it through */
  if (other.size()==1)
  {
    Array<Expr> rtn(this->size());
    for (unsigned int i=0; i<this->size(); i++)
    {
      rtn[i] = (*this)[i] * other[0];
    }
    return new ListExpr(rtn);
  }

  /* if both operands are flat vectors, take their dot product */
  if (this->size() == totalSize() && other.size()==other.totalSize() 
    && this->size() == other.size())
  {
    Expr rtn = new ZeroExpr();

    for (unsigned int i=0; i<this->size(); i++)
    {
      rtn = rtn + (*this)[i]*other[i];
    }
    return rtn;
  }

  /* if the left operand is a rectangular matrix and the 
   * right operand is a vector */
  unsigned int cols = (*this)[0].size();
  bool rectangular = true;
  for (unsigned int i=0; i<this->size(); i++)
  {
    if ((*this)[i].size() != cols) rectangular = false;
  }
  TEST_FOR_EXCEPTION(!rectangular, BadSymbolicsError,
    "Expr::operator* detected list-list multiplication "
    "with a non-rectangular left operator " 
    << toString());
  
  TEST_FOR_EXCEPTION(cols != other.size(), BadSymbolicsError,
    "Expr::operator* detected mismatched dimensions in "
    "list-list multiplication. Left operator is "
    << toString() << ", right operator is "
    << other.toString());
  
  Array<Expr> rtn(this->size());
  for (unsigned int i=0; i<this->size(); i++)
  {
    rtn[i] = (*this)[i] * other;
  }

  return new ListExpr(rtn);
}

Expr Expr::divide(const Expr& other) const 
{
  RefCountPtr<ScalarExpr> sOther = rcp_dynamic_cast<ScalarExpr>(other.ptr());
  Expr recip = new NonlinearUnaryOp(sOther, rcp(new StdReciprocal()));
  return (*this)[0].multiply(recip);
}

Expr Expr::multiply(const Expr& other) const 
{
  RefCountPtr<ScalarExpr> rtn;
  RefCountPtr<ScalarExpr> sThis = rcp_dynamic_cast<ScalarExpr>(ptr());
  RefCountPtr<ScalarExpr> sOther = rcp_dynamic_cast<ScalarExpr>(other.ptr());

  TEST_FOR_EXCEPTION(sThis.get()==NULL, InternalError,
    "Expr::multiply(): Left operand " << toString() 
    << " is a non-scalar expression. All list structure "
    "should have been handled before this point");

  TEST_FOR_EXCEPTION(sOther.get()==NULL, InternalError,
    "Expr::multiply(): Right operand " << other.toString() 
    << " is a non-scalar expression. All list structure "
    "should have been handled before this point");

  static StdProductTransformations trans;

  if (trans.doTransform(sThis, sOther, rtn)) 
  {
    if (SymbolicTransformation::classVerbosity() > 0)
    {
      Out::println("Expr::operator*() transformed product\n[" 
        + toString() + "*"
        + other.toString() + "]\n to\n ["
        + rtn->toString() + "]");
    }
    return handle(rtn);
  }

  return new ProductExpr(sThis, sOther);
}

Expr Expr::operator-() const 
{
  TimeMonitor t(opTimer());
  TimeMonitor t1(unaryMinusTimer());
  Tabs tabs;

  if (this->isComplex())
  {
    return new ComplexExpr(-real(), -imag());
  }

  /* if we are a real scalar, process the unary minus here */
  if (this->size()==1)
  {
    /* if we are spectral, thread unary minus over coeffs */
    const SpectralExpr* se 
      = dynamic_cast<const SpectralExpr*>((*this)[0].ptr().get());
    if (se != 0)
    {
      SpectralBasis basis = se->getSpectralBasis();
      Array<Expr> coeff(basis.nterms());
	
      for(int i=0; i<basis.nterms(); i++)
      {
        coeff[i] = - se->getCoeff(i);
      }
      Expr rtn = new SpectralExpr( basis, coeff);
      return rtn;
    }

    /* Test for some special cases that can be dealt with efficiently */
    const ConstantExpr* c 
      = dynamic_cast<const ConstantExpr*>((*this)[0].ptr().get());
    const UnaryMinus* u 
      = dynamic_cast<const UnaryMinus*>((*this)[0].ptr().get());
    /* if we are a constant, just modify the constant */
    if (c != 0)
    {
      if (c->value()==0.0)
      {
        return new ZeroExpr();
      }
      else
      {
        return new ConstantExpr(-1.0 * c->value());
      }
    }
    else if (u != 0) /* if we are already a unary minus, apply -(-x) --> x */
    {
      return u->arg();
    }
    else /* no special structure, so return a UnaryMinusExpr */
    {
      RefCountPtr<ScalarExpr> sThis 
        = rcp_dynamic_cast<ScalarExpr>((*this)[0].ptr());
      TEST_FOR_EXCEPTION(sThis.get()==NULL, InternalError,
        "Expr::operator-(): Operand " << (*this)[0].toString() 
        << " is a non-scalar expression. All list structure "
        "should have been handled before this point");
      return new UnaryMinus(sThis);
    }
  }

  /* otherwise, distribute the sign change over the list */
  Array<Expr> rtn(this->size());
  for (unsigned int i=0; i<this->size(); i++)
  {
    rtn[i] = -((*this)[i]);
  }
  return new ListExpr(rtn);
}


Expr Expr::operator/(const Expr& other) const 
{
  TimeMonitor t(opTimer());

  /* if the right operand is a list, this operation
   * makes no sense */
  TEST_FOR_EXCEPTION(other.size() != 1,
    BadSymbolicsError, 
    "Expr::operator/ detected division by a non-scalar "
    "expression " << toString());

  TEST_FOR_EXCEPTION(other.isSpectral(), InternalError, 
    "Division by a Spectral Expr is not yet defined");

  /* If other is complex, transform to make the denominator real */
  if (other.isComplex())
  {
    Expr magSq = other.real()*other.real() + other.imag()*other.imag();
    return (*this) * other.conj() / magSq;
  }

  /* If I'm complex and the other is not, distribute division over re and im */
  if (isComplex() && !other.isComplex())
  {
    return new ComplexExpr(real()/other, imag()/other);
  }

  /* If I'm spectral and the other is not, distribute division over coefficients */
  if (isSpectral() && !other.isSpectral()  && !other.isComplex())
  {
    const SpectralExpr* se 
      = dynamic_cast<const SpectralExpr*>((*this)[0].ptr().get());

    SpectralBasis basis = se->getSpectralBasis();
    Array<Expr> coeff(basis.nterms());
	  
    for(int i=0; i<basis.nterms(); i++)
    {
      coeff[i] = se->getCoeff(i)/ other[0];
    }
    Expr rtn = new SpectralExpr( basis, coeff);
    return rtn;
  }

  /* if we are a scalar, do simple scalar division */
  if (this->size()==1)
  {
    return (*this)[0].divide(other[0]);
  }

  /* otherwise, divide each element of the left by the right operand */
  Array<Expr> rtn(this->size());
  for (unsigned int i=0; i<this->size(); i++)
  {
    rtn[i] = (*this)[i] / other;
  }
  return new ListExpr(rtn);
}

const Expr& Expr::operator[](int i) const
{
  TEST_FOR_EXCEPTION(ptr().get()==NULL, InternalError,
    "null this detected in Expr::operator[].");

  TEST_FOR_EXCEPTION(i<0 || i>=(int) this->size(), RuntimeError,
    "Expr::operator[]() index i=" << i << " out of range [0, "
    << this->size() << " in expr " << toString());

  const ListExpr* le = dynamic_cast<const ListExpr*>(ptr().get());

  if (le != 0)
  {
    const Expr& rtn = le->element(i);
    TEST_FOR_EXCEPTION(rtn.size() == 0, InternalError,
      "attempt to access an empty list; this should "
      "never happen because the case should have been "
      "dealt with earlier");
    if (rtn.size()==1) 
    {
      TEST_FOR_EXCEPTION(rtn[0].ptr().get()==NULL, InternalError,
        "null return detected in Expr::operator[]. This="
        << toString() << ", i=" << i);
      return rtn[0];
    }
    TEST_FOR_EXCEPTION(rtn.ptr().get()==NULL, InternalError,
      "null return detected in Expr::operator[]. This="
      << toString() << ", i=" << i);
    return rtn;
  }

  return *this;
}

void Expr::append(const Expr& expr)
{
  ListExpr* le = dynamic_cast<ListExpr*>(ptr().get());

  if (le != 0)
  {
    le->append(expr);
    return;
  }
  else
  {
    if (ptr().get()==0)
    {
      Array<Expr> e(1);
      e[0] = expr;
      ptr() = rcp(new ListExpr(e));
    }
    else
    {
      Array<Expr> e(2);
      e[0] = *this;
      e[1] = expr;
      ptr() = rcp(new ListExpr(e));
    }
  }
}

Expr Expr::flatten() const
{
  const ListExpr* le = dynamic_cast<const ListExpr*>(ptr().get());

  if (le != 0)
  {
    return le->flatten();
  }
  return *this;
}

Expr Expr::flattenSpectral() const
{
  Array<Expr> rtn(size());
  for (unsigned int i=0; i<size(); i++)
  {
    if ((*this)[i].size() == 1)
    {
      const SpectralExpr* se 
        = dynamic_cast<const SpectralExpr*>((*this)[i][0].ptr().get());
      if (se != 0)
      {
        int nt = se->getSpectralBasis().nterms();
        Array<Expr> e(nt);
        for (int j=0; j<nt; j++)
        {
          e[j] = se->getCoeff(j);
        }
        rtn[i] = new ListExpr(e);
      }
      else
      {
        rtn[i] = (*this)[i];
      }
    }
    else
    {
      rtn[i] = (*this)[i].flattenSpectral();
    }
  }
  Expr r = new ListExpr(rtn);
  return r.flatten();
}

unsigned int Expr::size() const
{
  if (ptr().get()==0) return 0;

  const ListExpr* le = dynamic_cast<const ListExpr*>(ptr().get());

  if (le != 0)
  {
    return le->size();
  }
  return 1;
}

unsigned int Expr::totalSize() const
{
  if (ptr().get()==0) return 0;

  const ListExpr* le = dynamic_cast<const ListExpr*>(ptr().get());

  if (le != 0)
  {
    return le->totalSize();
  }
  return 1;
}

Expr Expr::real() const
{
  if (isComplex()) 
  {
    const ComplexExpr* cx = dynamic_cast<const ComplexExpr*>(ptr().get());
    return cx->real();
  }
  else if (size() > 1)
  {
    Array<Expr> rtn(size());
    for (unsigned int i=0; i<size(); i++)
    {
      rtn[i] = (*this)[i].real();
    }
    return new ListExpr(rtn);
  }
  else 
  {
    return *this;
  }
}

Expr Expr::imag() const
{
  if (isComplex()) 
  {
    const ComplexExpr* cx = dynamic_cast<const ComplexExpr*>(ptr().get());
    return cx->imag();
  }
  else if (size() > 1)
  {
    Array<Expr> rtn(size());
    for (unsigned int i=0; i<size(); i++)
    {
      rtn[i] = (*this)[i].imag();
    }
    return new ListExpr(rtn);
  }
  else
  {
    return 0.0;
  }
  
}

Expr Expr::conj() const
{
  if (size()==1)
  {
    if (isComplex()) 
    {
      return new ComplexExpr(real(), -imag());
    }
    else return real();
  }
  else
  {
    Array<Expr> rtn(size());
    for (unsigned int i=0; i<size(); i++)
    {
      rtn[i] = (*this)[i].conj();
    }
    return new ListExpr(rtn);
  }
}

void Expr::setParameterValue(const double& value)
{
  Parameter* pe = dynamic_cast<Parameter*>(ptr().get());
  TEST_FOR_EXCEPTION(pe==0, RuntimeError, 
    "Expr " << *this << " is not a Parameter expr, and "
    "so setParameterValue() should not be called");
  pe->setValue(value);
}

double Expr::getParameterValue() const 
{
  const Parameter* pe = dynamic_cast<const Parameter*>(ptr().get());
  TEST_FOR_EXCEPTION(pe==0, RuntimeError, 
    "Expr " << *this << " is not a Parameter expr, and "
    "so getParameterValue() should not be called");
  return pe->value();
}


bool Expr::isIndependentOf(const Expr& u) const
{
  TEST_FOR_EXCEPTION(ptr().get()==0, RuntimeError, 
    "function called on null expression");
  
  return scalarExpr()->isIndependentOf(u);
}


bool Expr::isLinearForm(const Expr& u) const
{
  TEST_FOR_EXCEPTION(ptr().get()==0, RuntimeError, 
    "function called on null expression");

  return scalarExpr()->isLinearForm(u);
}



bool Expr::isQuadraticForm(const Expr& u) const
{
  TEST_FOR_EXCEPTION(ptr().get()==0, RuntimeError, 
    "function called on null expression");

  return scalarExpr()->isQuadraticForm(u);
}



bool Expr::isLinearInTests() const
{
  TEST_FOR_EXCEPTION(ptr().get()==0, RuntimeError, 
    "function called on null expression");

  return scalarExpr()->isLinearInTests();
}


bool Expr::hasTestFunctions() const
{
  TEST_FOR_EXCEPTION(ptr().get()==0, RuntimeError, 
    "function called on null expression");

  return scalarExpr()->hasTestFunctions();
}


bool Expr::everyTermHasTestFunctions() const
{
  TEST_FOR_EXCEPTION(ptr().get()==0, RuntimeError, 
    "function called on null expression");

  return scalarExpr()->everyTermHasTestFunctions();
}

bool Expr::isTestElement() const
{
  const TestFuncElement* uPtr
    = dynamic_cast<const TestFuncElement*>(ptr().get());
  return uPtr != 0;
}


bool Expr::isUnknownElement() const
{
  const UnknownFuncElement* uPtr
    = dynamic_cast<const UnknownFuncElement*>(ptr().get());
  return uPtr != 0;
}


void Expr::getUnknowns(Set<int>& unkID, 
  Array<Expr>& unks) const
{
  TEST_FOR_EXCEPTION(ptr().get()==0, RuntimeError, 
    "function called on null expression");

  const UnknownFuncElement* u 
    = dynamic_cast<const UnknownFuncElement*>(ptr().get());

  if (u != 0) 
  {
    if (!unkID.contains(u->fid().dofID())) 
    {
      unks.append(*this);
      unkID.put(u->fid().dofID());
    }
  }
  else
  {
    scalarExpr()->getUnknowns(unkID, unks);
  }
}


void Expr::getTests(Set<int>& varID, Array<Expr>& vars) const
{
  TEST_FOR_EXCEPTION(ptr().get()==0, RuntimeError, 
    "function called on null expression");

  const TestFuncElement* u 
    = dynamic_cast<const TestFuncElement*>(ptr().get());

  if (u != 0) 
  {
    if (!varID.contains(u->fid().dofID())) 
    {
      vars.append(*this);
      varID.put(u->fid().dofID());
    }
  }
  else
  {
    scalarExpr()->getTests(varID, vars);
  }
}


const ScalarExpr* Expr::scalarExpr() const 
{
  const ScalarExpr* se = dynamic_cast<const ScalarExpr*>(ptr().get());
  TEST_FOR_EXCEPTION(se==0, InternalError, "ScalarExpr expected here");
  return se;
}

const FuncElementBase* Expr::funcElement() const 
{
  const FuncElementBase* fe 
    = dynamic_cast<const FuncElementBase*>(ptr().get());
  return fe;
}




namespace SundanceCore
{
using namespace SundanceUtils;

Expr Complex(const Expr& re, const Expr& im)
{
  TEST_FOR_EXCEPTION(re.size() != im.size(), RuntimeError,
    "arguments mismatched in Complex(). Real part="
    << re << ", imaginary part=" << im);

  TEST_FOR_EXCEPTION(re.isComplex() || im.isComplex(), RuntimeError,
    "recursively defined complex number. Real part="
    << re << ", imaginary part=" << im);

  if (re.totalSize() > 1U)
  {
    Array<Expr> rtn(re.size());
    for (unsigned int i=0; i<re.size(); i++)
    {
      rtn[i] = Complex(re[i], im[i]);
    }
    return new ListExpr(rtn);
  }

  const ZeroExpr* zr = dynamic_cast<const ZeroExpr*>(re[0].ptr().get());
  const ZeroExpr* zi = dynamic_cast<const ZeroExpr*>(im[0].ptr().get());

  if (zr == 0) /* nonzero real part */
  {
    if (zi==0) /* nonzero imag part */
    {
      return new ComplexExpr(re, im);
    }
    else /* zero imag part */
    {
      return re;
    }
  }
  else /* zero real part */
  {
    if (zi != 0) /* both are zero */
    {
      return Expr(0.0);
    }
    else /* pure imaginary */
    {
      return new ComplexExpr(0.0, im);
    }
  }
  return new ComplexExpr(re, im);
}

Expr List(const Expr& a)
{
  return new ListExpr(tuple(a));
}

Expr List(const Expr& a, const Expr& b)
{
  return new ListExpr(tuple(a,b));
}

Expr List(const Expr& a, const Expr& b, const Expr& c)
{
  return new ListExpr(tuple(a,b,c));
}

Expr List(const Expr& a, const Expr& b, const Expr& c,
  const Expr& d)
{
  return new ListExpr(tuple(a,b,c,d));
}

Expr List(const Expr& a, const Expr& b, const Expr& c,
  const Expr& d, const Expr& e)
{
  return new ListExpr(tuple(a,b,c,d,e));
}

Expr List(const Expr& a, const Expr& b, const Expr& c,
  const Expr& d, const Expr& e, const Expr& f)
{
  return new ListExpr(tuple(a,b,c,d,e,f));
}

Expr List(const Expr& a, const Expr& b, const Expr& c,
  const Expr& d, const Expr& e, const Expr& f,
  const Expr& g)
{
  return new ListExpr(tuple(a,b,c,d,e,f,g));
}

Expr List(const Expr& a, const Expr& b, const Expr& c,
  const Expr& d, const Expr& e, const Expr& f,
  const Expr& g, const Expr& h)
{
  return new ListExpr(tuple(a,b,c,d,e,f,g,h));
}


Expr gradient(int dim)
{
  Array<Expr> rtn(dim);
  for (int i=0; i<dim; i++)
  {
    rtn[i] = new Derivative(i);
  }
  return new ListExpr(rtn);
}

Expr div(const Expr& f)
{
  Expr rtn = 0.0;
  for (unsigned int i=0; i<f.size(); i++)
  {
    Expr d = new Derivative(i);
    rtn = rtn + d*f[i];
  }
  return rtn;
}


Expr cross(const Expr& a, const Expr& b)
{
  TEST_FOR_EXCEPTION(a.size() != b.size(), RuntimeError,
    "mismatched vector sizes in cross(a,b): a.size()=" << a.size()
    << ", b.size()=" << b.size());

  TEST_FOR_EXCEPTION(a.size() < 2U || a.size() > 3U, RuntimeError,
    "cross(a,b) undefined for dim=" << a.size());

  if (a.size()==2U)
  {
    return a[0]*b[1] - a[1]*b[0];
  }
  else
  {
    return List(
      cross(List(a[1],a[2]), List(b[1],b[2])),
      -cross(List(a[0],a[2]), List(b[0],b[2])),
      cross(List(a[0],a[1]), List(b[0],b[1]))
      );
  }
}

Expr curl(const Expr& f)
{
  Expr del = gradient(f.size());

  return cross(del, f);
}

}

