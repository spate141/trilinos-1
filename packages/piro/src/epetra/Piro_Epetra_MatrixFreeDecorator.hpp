#ifndef PIRO_EPETRA_MATRIXFREEDECORATOR_H
#define PIRO_EPETRA_MATRIXFREEDECORATOR_H

#include <iostream>

#include "Epetra_CrsMatrix.h"
#include "Epetra_Vector.h"
#include "Epetra_LocalMap.h"

#include "EpetraExt_ModelEvaluator.h"
#include "Thyra_LinearOpWithSolveFactoryHelpers.hpp"

/** \brief  Decorator class that creates a W (Jacobian) operator
 *          using Matrix-Free directional derivatives. 
 *
 * This class takes a model evaluator that supports a residual
 * calculation, and adds support for the W matrix as an
 * epetra_operator.
 *
 * This class is the ModelEvaluator version of NOX_Epetra_MatrixFree.
 * One difference, is that it supports time-dependent problems
 * when x_dot != null, and using alpha and beta. 
 * 
 */

namespace Piro {
namespace Epetra {

class MatrixFreeDecorator
    : public EpetraExt::ModelEvaluator
{

  public:

  /** \name Constructors/initializers */
  //@{

  /** \brief Takes the number of elements in the discretization . */
  MatrixFreeDecorator( 
                Teuchos::RCP<EpetraExt::ModelEvaluator>& model,
                double lambda_ = 1.0e-6
                );

  //@}

  ~MatrixFreeDecorator();


  /** \name Overridden from EpetraExt::ModelEvaluator . */
  //@{

  Teuchos::RCP<const Epetra_Map> get_g_map(int j) const;
  /** \brief . */
  Teuchos::RCP<const Epetra_Vector> get_p_init(int l) const;
  /** \brief . */
  Teuchos::RCP<Epetra_Operator> create_W() const;
 /** \brief . */
  Teuchos::RCP<Epetra_Operator> create_M() const;
  /** \brief . */
  EpetraExt::ModelEvaluator::InArgs createInArgs() const;
  /** \brief . */
  EpetraExt::ModelEvaluator::OutArgs createOutArgs() const;
  /** \brief . */
  void evalModel( const InArgs& inArgs, const OutArgs& outArgs ) const;
  //@}

  private:
  /** \brief . */
  Teuchos::RCP<const Epetra_Map> get_x_map() const;
  /** \brief . */
  Teuchos::RCP<const Epetra_Map> get_f_map() const;
  /** \brief . */
  Teuchos::RCP<const Epetra_Vector> get_x_init() const;
  /** \brief . */
  Teuchos::RCP<const Epetra_Map> get_p_map(int l) const;
  /** \brief . */
  Teuchos::RCP<const Teuchos::Array<std::string> > get_p_names(int l) const;

  private:

   //These are set in the constructor and used in evalModel
   Teuchos::RCP<EpetraExt::ModelEvaluator> model;
   // Storage for Base resid vector for MatrixFree 
   Teuchos::RCP<Epetra_Vector> fBase;

   // Constant used in formulas to pick perturbation, typically 1.0e-6
   double lambda;

};

class MatrixFreeOperator
    : public Epetra_Operator
{

  public:

  /** \name Constructors/initializers */
  //@{

  /** \brief  */
  MatrixFreeOperator(const Teuchos::RCP<EpetraExt::ModelEvaluator>& model,
                     double lambda_ = 1.0e-6);

  //@}

  ~MatrixFreeOperator() { modelInArgs.set_x(Teuchos::null); modelInArgs.set_p(0,Teuchos::null); };

  //!  Method to save base solution and residual
  void setBase(const EpetraExt::ModelEvaluator::InArgs modelInArgs_,
               Teuchos::RCP<Epetra_Vector> fBase);

  /** \name Overridden from Epetra_Operator . */
  //@{
  
  int SetUseTranspose(bool UseTranspose);
  int Apply(const Epetra_MultiVector& V, Epetra_MultiVector& Y) const;
  int ApplyInverse(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const;
  double NormInf() const;
  const char* Label () const;
  bool UseTranspose() const;
  bool HasNormInf() const;
  const Epetra_Comm & Comm() const;
  const Epetra_Map& OperatorDomainMap () const;
  const Epetra_Map& OperatorRangeMap () const;

  //@}

  private:

   // Underlying model
   Teuchos::RCP<EpetraExt::ModelEvaluator> model;
   // Storage for Base solution vector for MatrixFree 
   mutable EpetraExt::ModelEvaluator::InArgs modelInArgs;
   // Storage for Perturbed solution vector for MatrixFree 
   Teuchos::RCP<Epetra_Vector> xPert;
   // Storage for Perturbed solution vector for MatrixFree 
   Teuchos::RCP<Epetra_Vector> xdotPert;
   // Storage for Base resid vector for MatrixFree 
   Teuchos::RCP<Epetra_Vector> fBase;
   // Storage for Perturbed resid vector for MatrixFree 
   Teuchos::RCP<Epetra_Vector> fPert;

   double solutionNorm;
   bool  baseIsSet;
   bool  haveXdot;
   const double lambda;

};

}
}
#endif