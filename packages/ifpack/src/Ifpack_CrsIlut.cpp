/* NOTICE:  The United States Government is granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable worldwide
 * license in ths data to reproduce, prepare derivative works, and
 * perform publicly and display publicly.  Beginning five (5) years from
 * October 20, 2002, the United States Government is granted for itself and
 * others acting on its behalf a paid-up, nonexclusive, irrevocable
 * worldwide license in this data to reproduce, prepare derivative works,
 * distribute copies to the public, perform publicly and display
 * publicly, and to permit others to do so.
 * 
 * NEITHER THE UNITED STATES GOVERNMENT, NOR THE UNITED STATES DEPARTMENT
 * OF ENERGY, NOR SANDIA CORPORATION, NOR ANY OF THEIR EMPLOYEES, MAKES
 * ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR
 * RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY
 * INFORMATION, APPARATUS, PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS
 * THAT ITS USE WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS. */

#include "Ifpack_CrsIlut.h"
#include "Epetra_Comm.h"
#include "Epetra_Map.h"
#include "Epetra_CrsGraph.h"
#include "Epetra_CrsMatrix.h"
#include "Epetra_VbrMatrix.h"
#include "Epetra_RowMatrix.h"
#include "Epetra_Vector.h"
#include "Epetra_MultiVector.h"

//==============================================================================
Ifpack_CrsIlut::Ifpack_CrsIlut(const Ifpack_OverlapGraph * OverlapGraph, double DropTol, 
			       double FillTol) 
  : Epetra_Object("Ifpack::CrsIlut"),
    Epetra_CompObject(),
    Ifpack_OverlapFactorObject(OverlapGraph),
    Ifpack_OverlapSolveObject(Epetra_Object::Label(), OverlapGraph->OverlapGraph().Comm()),
    DropTol_(DropTol),
    FillTol_(FillTol)
{
}
//==============================================================================
Ifpack_CrsIlut::Ifpack_CrsIlut(const Epetra_RowMatrix * UserMatrix, double DropTol, 
			       double FillTol) 
  : Epetra_Object("Ifpack::CrsIlut"),
    Epetra_CompObject(),
    Ifpack_OverlapFactorObject(UserMatrix),
    Ifpack_OverlapSolveObject(Epetra_Object::Label(), UserMatrix->Comm()),
    DropTol_(DropTol),
    FillTol_(FillTol)
{
}
//==============================================================================
Ifpack_CrsIlut::Ifpack_CrsIlut(const Ifpack_CrsIlut & Source) 
  : Epetra_Object(Source),
    Epetra_CompObject(Source),
    Ifpack_OverlapFactorObject(Source),
    Ifpack_OverlapSolveObject(Source),
    DropTol_(Source.DropTol_),
    FillTol_(Source.FillTol_)
{
}
//==============================================================================
int Ifpack_CrsIlut::ProcessOverlapMatrix(const Epetra_RowMatrix &A)
{

  return(0);
}
//==============================================================================
int Ifpack_CrsIlut::DerivedFactor()
{

  return(0);
}
