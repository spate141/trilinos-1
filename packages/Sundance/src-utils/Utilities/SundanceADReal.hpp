/* @HEADER@ */
/* @HEADER@ */

#ifndef ADREAL_H
#define ADREAL_H

#ifndef DOXYGEN_DEVELOPER_ONLY

#include "SundanceDefs.hpp"
#include "SundancePoint.hpp"
#include "SundanceStdMathFunctors.hpp"
#include "Teuchos_RefCountPtr.hpp"

namespace SundanceUtils
{
  /**
   * First-order automatic differentiation of a multivariable function.
   * @author Kevin Long
   */
  class ADReal
  {
  public:
    /** Create an ADReal with value and gradient = 0 */
    ADReal() : value_(0), gradient_(0.0, 0.0, 0.0){;}
    /** Create an ADReal object that varies linearly with
     * one coordinate direction  */
    ADReal(double value, int direction, int spatialDimension)
      : value_(value), gradient_()
    {
      if (spatialDimension==1) gradient_ = Point(0.0);
      if (spatialDimension==2) gradient_ = Point(0.0, 0.0);
      if (spatialDimension==3) gradient_ = Point(0.0, 0.0, 0.0);
      gradient_[direction] = 1.0;
    }
    /** Create a constant-valued ADReal object in a multidimensional space */
    ADReal(double value, int spatialDimension)
      : value_(value), gradient_()
    {
      if (spatialDimension==1) gradient_ = Point(0.0);
      if (spatialDimension==2) gradient_ = Point(0.0, 0.0);
      if (spatialDimension==3) gradient_ = Point(0.0, 0.0, 0.0);
    }

    /** Create an ADReal with the specified value and gradient */
    ADReal(double value, const Point& gradient)
      : value_(value), gradient_(gradient) {;}

    /** unary minus */
    ADReal operator-() const ;
    /** reflexive addition */
    ADReal& operator+=(const ADReal& other) ;
    /** reflexive subtraction */
    ADReal& operator-=(const ADReal& other) ;
    /** reflexive multiplication */
    ADReal& operator*=(const ADReal& other) ;
    /** reflexive division */
    ADReal& operator/=(const ADReal& other) ;

    /** reflexive scalar addition */
    ADReal& operator+=(const double& scalar) ;
    /** reflexive scalar subtraction */
    ADReal& operator-=(const double& scalar) ;
    /** reflexive scalar multiplication */
    ADReal& operator*=(const double& scalar) ;
    /** reflexive scalar division */
    ADReal& operator/=(const double& scalar) ;

    /** addition */
    ADReal operator+(const ADReal& other) const ;
    /** subtraction */
    ADReal operator-(const ADReal& other) const ;
    /** multiplication */
    ADReal operator*(const ADReal& other) const ;
    /** division */
    ADReal operator/(const ADReal& other) const ;

    /** scalar addition */
    ADReal operator+(const double& scalar) const ;
    /** scalar subtraction */
    ADReal operator-(const double& scalar) const ;
    /** scalar multiplication */
    ADReal operator*(const double& scalar) const ;
    /** scalar division */
    ADReal operator/(const double& scalar) const ;

    /** get the value */
    const double& value() const {return value_;}
    /** get the gradient */
    const Point& gradient() const {return gradient_;}

    void reciprocate() ;

  private:
    double value_;
    Point gradient_;
  };



  /** \relates ADReal */
  ADReal operator+(const double& scalar,
                   const ADReal& a);
  /** \relates ADReal */
  ADReal operator-(const double& scalar,
                   const ADReal& a);
  /** \relates ADReal */
  ADReal operator*(const double& scalar,
                   const ADReal& a);
  /** \relates ADReal */
  ADReal operator/(const double& scalar,
                   const ADReal& a);


#define SUNDANCE_AD_FUNCTOR(opName, functorName) \
inline ADReal opName(const ADReal& x)\
{\
Teuchos::RefCountPtr<UnaryFunctor> func = Teuchos::rcp(new functorName());\
double f;\
double df;\
double xVal = x.value();\
func->eval1(&xVal, 1, &f, &df);\
return ADReal(f, df*x.gradient());}


  SUNDANCE_AD_FUNCTOR(exp, StdExp);

  SUNDANCE_AD_FUNCTOR(log, StdLog);

  SUNDANCE_AD_FUNCTOR(sqrt, StdSqrt);

  SUNDANCE_AD_FUNCTOR(sin, StdSin);

  SUNDANCE_AD_FUNCTOR(cos, StdCos);

} 


namespace std
{
  inline ostream& operator<<(ostream& os, const SundanceUtils::ADReal& x)
  {
    cerr << "AD[" << x.value() << ", grad="<< x.gradient() << "]";
    return os;
  }
}


#endif  /* DOXYGEN_DEVELOPER_ONLY */   

#endif



