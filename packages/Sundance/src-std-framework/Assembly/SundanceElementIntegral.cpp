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

#include "SundanceElementIntegral.hpp"
#include "SundanceOut.hpp"
#include "SundanceTabs.hpp"
#include "Teuchos_TimeMonitor.hpp"

using namespace SundanceStdFwk;
using namespace SundanceStdFwk::Internal;
using namespace SundanceCore;
using namespace SundanceCore;
using namespace SundanceStdMesh;
using namespace SundanceStdMesh::Internal;
using namespace SundanceUtils;
using namespace Teuchos;
using namespace TSFExtended;
using std::endl;

static Time& transCreationTimer() 
{
  static RefCountPtr<Time> rtn 
    = TimeMonitor::getNewTimer("building integral transformation matrices"); 
  return *rtn;
}


bool& ElementIntegral::alwaysUseCofacets()
{
  static bool rtn = true;
  return rtn;
}

ElementIntegral::ElementIntegral(int spatialDim,
  const CellType& maxCellType,
  int dim, 
  const CellType& cellType,
  bool isInternalBdry,
  int verb)
  : setupVerb_(verb),
    integrationVerb_(0),
    transformVerb_(0),
    spatialDim_(spatialDim),
    dim_(dim),
    isInternalBdry_(isInternalBdry),
    nFacetCases_(1),
    testDerivOrder_(-1), 
    nRefDerivTest_(-1),
    nNodesTest_(-1),
    unkDerivOrder_(-1), 
    nRefDerivUnk_(-1),
    nNodesUnk_(-1),
    nNodes_(-1),
    order_(0),
    alpha_(),
    beta_(),
    cellType_(cellType),
    maxCellType_(maxCellType),
    evalCellType_(cellType),
    testBasis_(),
    unkBasis_()
{
  /* if we're integrating a derivative along a facet, we need to refer back
   * to the maximal cell. */
  if (alwaysUseCofacets() && dim != spatialDim)
  {
    evalCellType_ = maxCellType_;
    nFacetCases_ = numFacets(maxCellType, dim);
  }
}

ElementIntegral::ElementIntegral(int spatialDim,
  const CellType& maxCellType,
  int dim, 
  const CellType& cellType,
  const BasisFamily& testBasis,
  int alpha,
  int testDerivOrder,
  bool isInternalBdry,
  int verb)
  : setupVerb_(verb),
    integrationVerb_(0),
    transformVerb_(0),
    spatialDim_(spatialDim),
    dim_(dim),
    isInternalBdry_(isInternalBdry),
    nFacetCases_(1),
    testDerivOrder_(testDerivOrder), 
    nRefDerivTest_(ipow(spatialDim, testDerivOrder)),
    nNodesTest_(testBasis.nReferenceDOFs(maxCellType, cellType)),
    unkDerivOrder_(-1), 
    nRefDerivUnk_(-1),
    nNodesUnk_(-1),
    nNodes_(nNodesTest_),
    order_(1),
    alpha_(alpha),
    beta_(-1),
    cellType_(cellType),
    maxCellType_(maxCellType),
    evalCellType_(cellType),
    testBasis_(testBasis),
    unkBasis_()
{
  /* if we're integrating a derivative along a facet, we 
   * may need to refer back to the maximal cell. */
  bool okToRestrictTestToBdry = basisRestrictableToBoundary(testBasis);
    
  if (dim != spatialDim)
  {
    if (isInternalBdry)
    {
      TEST_FOR_EXCEPT(!okToRestrictTestToBdry);
    }
    else if (alwaysUseCofacets() || testDerivOrder>0)
    {
      evalCellType_ = maxCellType_;
      nFacetCases_ = numFacets(maxCellType, dim);
      nNodesTest_ = testBasis.nReferenceDOFs(maxCellType, maxCellType);
      nNodes_ = nNodesTest_;
    }
    else
    {
      TEST_FOR_EXCEPT(!okToRestrictTestToBdry);
    }
  }
}



ElementIntegral::ElementIntegral(int spatialDim,
  const CellType& maxCellType,
  int dim,
  const CellType& cellType,
  const BasisFamily& testBasis,
  int alpha,
  int testDerivOrder,
  const BasisFamily& unkBasis,
  int beta,
  int unkDerivOrder,
  bool isInternalBdry,
  int verb)
  : setupVerb_(verb),
    integrationVerb_(0),
    transformVerb_(0),
    spatialDim_(spatialDim),
    dim_(dim),
    isInternalBdry_(isInternalBdry),
    nFacetCases_(1),
    testDerivOrder_(testDerivOrder), 
    nRefDerivTest_(ipow(spatialDim, testDerivOrder)),
    nNodesTest_(testBasis.nReferenceDOFs(maxCellType, cellType)), 
    unkDerivOrder_(unkDerivOrder), 
    nRefDerivUnk_(ipow(spatialDim, unkDerivOrder)),
    nNodesUnk_(unkBasis.nReferenceDOFs(maxCellType, cellType)), 
    nNodes_(nNodesTest_*nNodesUnk_),
    order_(2),
    alpha_(alpha),
    beta_(beta),
    cellType_(cellType),
    maxCellType_(maxCellType),
    evalCellType_(cellType),
    testBasis_(testBasis),
    unkBasis_(unkBasis)
{
  /* if we're integrating a derivative along a facet, we may need to refer back
   * to the maximal cell. */
  bool okToRestrictTestToBdry = basisRestrictableToBoundary(testBasis);
  bool okToRestrictUnkToBdry = basisRestrictableToBoundary(unkBasis);


  if (dim != spatialDim)
  {
    if (isInternalBdry)
    {
      TEST_FOR_EXCEPT(!(okToRestrictTestToBdry && okToRestrictUnkToBdry));   
    }
    else if (alwaysUseCofacets() || testDerivOrder>0 || unkDerivOrder>0)
    {
      evalCellType_ = maxCellType_;
      nFacetCases_ = numFacets(maxCellType, dim);
      nNodesTest_ = testBasis.nReferenceDOFs(maxCellType, maxCellType);
      nNodesUnk_ = unkBasis.nReferenceDOFs(maxCellType, maxCellType);
      nNodes_ = nNodesTest_ * nNodesUnk_;
    }
    else
    {
      TEST_FOR_EXCEPT(okToRestrictTestToBdry != okToRestrictUnkToBdry);
    }
  }
}


void ElementIntegral::setVerbosity(
  int integrationVerb,
  int transformVerb)
{
  integrationVerb_ = integrationVerb;
  transformVerb_ = transformVerb;
}

void ElementIntegral::describe(ostream& os) const 
{
  Tabs tab(0);
  bool hasTest = testDerivOrder() >= 0;
  bool hasUnk = unkDerivOrder() >= 0;

  if (hasTest)
  {
    os << tab << "Test functions:" << endl;
    Tabs tab1;
    os << tab1 << "test basis = " << testBasis() << endl;
    os << tab1 << "test differentiation order = " << testDerivOrder() << endl;
    os << tab1 << "alpha = " << alpha() << endl;
    os << tab1 << "num test gradient components=" << nRefDerivTest() << endl;
  }
  if (hasUnk)
  {
    os << tab << "Unknown functions:" << endl;
    Tabs tab1;
    os << tab1 << "unk basis = " << unkBasis() << endl;
    os << tab1 << "unk differentiation order = " << unkDerivOrder() << endl;
    os << tab1 << "beta = " << beta() << endl;
    os << tab1  << "num unk gradient components=" << nRefDerivUnk() << endl;
  }
  os << tab << "Geometry:" << endl;
  Tabs tab1;
  os << tab1 << "cell type on which integral is defined: " << cellType()
     << endl;
  os << tab1 << "maximal cell type: " << maxCellType() << endl;
  os << tab1 << "cell type on which quad pts defined: " << evalCellType()
     << endl;
  os << tab << "Number of evaluation cases: " << nFacetCases() << endl;
}


void ElementIntegral::assertBilinearForm() const 
{
  TEST_FOR_EXCEPTION(testDerivOrder() < 0 || testDerivOrder() > 1,
    InternalError,
    "Test function derivative order=" << testDerivOrder()
    << " must be 0 or 1");
  
  TEST_FOR_EXCEPTION(unkDerivOrder() < 0 || unkDerivOrder() > 1,
    InternalError,
    "Unknown function derivative order=" << unkDerivOrder()
    << " must be 0 or 1");
}

void ElementIntegral::assertLinearForm() const 
{
  TEST_FOR_EXCEPTION(testDerivOrder() < 0 || testDerivOrder() > 1,
    InternalError,
    "Test function derivative order=" << testDerivOrder()
    << " must be 0 or 1");
}


void ElementIntegral::getQuad(const QuadratureFamily& quad,
  int evalCase, Array<Point>& quadPts, Array<double>& quadWeights) const 
{
  Tabs tab(0);

  SUNDANCE_MSG2(setupVerb(), tab << "getting quad points for rule "
    << quad);

  if (nFacetCases()==1) 
  {
    quad.getPoints(cellType(), quadPts, quadWeights);
  }
  else 
  {
    int dim = dimension(cellType());
    quad.getFacetPoints(maxCellType(), dim, evalCase, 
      quadPts, quadWeights);
  }

  if (setupVerb() >= 4)
  {
    Tabs tab1;
    SUNDANCE_MSG4(setupVerb(), 
      tab1 << "quadrature points on ref cell are:");
    printQuad(Out::os(), quadPts, quadWeights);
  }
}
  


Array<double>& ElementIntegral::G(int alpha)
{
  static Array<Array<double> > rtn(3);

  return rtn[alpha];
}

Array<double>& ElementIntegral::G(int alpha, int beta)
{
  static Array<Array<double> > rtn(9);

  return rtn[3*alpha + beta];
}

int& ElementIntegral::transformationMatrixIsValid(int alpha, int beta)
{
  static Array<int> rtn(9, false);
  return rtn[3*alpha + beta];
}

int& ElementIntegral::transformationMatrixIsValid(int alpha)
{
  static Array<int> rtn(3, false);
  return rtn[alpha];
}

void ElementIntegral::invalidateTransformationMatrices()
{
  for (int i=0; i<3; i++)
  {
    transformationMatrixIsValid(i) = false;
    for (int j=0; j<3; j++)
    {
      transformationMatrixIsValid(i, j) = false;
    }
  }
}




int ElementIntegral::ipow(int base, int power) 
{
  int rtn = 1;
  for (int i=0; i<power; i++) rtn *= base;
  return rtn;
}

void ElementIntegral
::createTwoFormTransformationMatrix(const CellJacobianBatch& JTrans,
  const CellJacobianBatch& JVol) const
{
  TimeMonitor timer(transCreationTimer());
  Tabs tab;

  int flops = 0;

  int maxDim = JTrans.cellDim();

  if (testDerivOrder() == 1 && unkDerivOrder() == 1)
  {
    Tabs tab2;
    if (transformationMatrixIsValid(alpha(), beta())) return;
    transformationMatrixIsValid(alpha(), beta()) = true;

    G(alpha(), beta()).resize(JTrans.numCells() * JTrans.cellDim() * JTrans.cellDim());

    double* GPtr = &(G(alpha(),beta())[0]);
    int k = 0;

    for (int c=0; c<JTrans.numCells(); c++)
    {
      static Array<double> invJ;
      JTrans.getInvJ(c, invJ);
      double detJ = fabs(JVol.detJ()[c]);
      for (int gamma=0; gamma<maxDim; gamma++)
      {
        for (int delta=0; delta<maxDim; delta++, k++)
        {
          GPtr[k] =  detJ*invJ[alpha() + gamma*maxDim]
            * invJ[beta() + maxDim*delta];
        }
      }
    }
    flops = 2 * JTrans.numCells() * maxDim * maxDim + JTrans.numCells();
  }

  else if (testDerivOrder() == 1 && unkDerivOrder() == 0)
  {
    if (transformationMatrixIsValid(alpha())) return;
    transformationMatrixIsValid(alpha()) = true;

    G(alpha()).resize(JTrans.numCells() * JTrans.cellDim());

    int k = 0;
    double* GPtr = &(G(alpha())[0]);

    for (int c=0; c<JTrans.numCells(); c++)
    {
      static Array<double> invJ;
      JTrans.getInvJ(c, invJ);
      double detJ = fabs(JVol.detJ()[c]);
      for (int gamma=0; gamma<maxDim; gamma++,k++)
      {
        GPtr[k] = detJ*invJ[alpha() + maxDim * gamma];
      }
    }
    flops = JTrans.numCells() * maxDim + JTrans.numCells();
  }

  else 
  {
    if (transformationMatrixIsValid(beta())) return;
    transformationMatrixIsValid(beta()) = true;

    G(beta()).resize(JTrans.numCells() * JTrans.cellDim());

    int k = 0;
    double* GPtr = &(G(beta())[0]);

    for (int c=0; c<JTrans.numCells(); c++)
    {
      static Array<double> invJ;
      JTrans.getInvJ(c, invJ);
      double detJ = fabs(JVol.detJ()[c]);
      for (int gamma=0; gamma<maxDim; gamma++,k++)
      {
        GPtr[k] = detJ*invJ[beta() + maxDim * gamma];
      }
    }
    flops = JTrans.numCells() * maxDim + JTrans.numCells();
  }

  addFlops(flops);
}


void ElementIntegral
::createOneFormTransformationMatrix(const CellJacobianBatch& JTrans,
  const CellJacobianBatch& JVol) const 
{
  TimeMonitor timer(transCreationTimer());
  Tabs tab;
  SUNDANCE_MSG2(transformVerb(), 
    tab << "ElementIntegral creating linear form trans matrices");

  int maxDim = JTrans.cellDim();

  if (transformationMatrixIsValid(alpha())) return;
  transformationMatrixIsValid(alpha()) = true;

  int flops = JTrans.numCells() * maxDim + JTrans.numCells();

  G(alpha()).resize(JTrans.numCells() * JTrans.cellDim());

  int k = 0;
  double* GPtr = &(G(alpha())[0]);

  for (int c=0; c<JTrans.numCells(); c++)
  {
    Array<double> invJ;
    JTrans.getInvJ(c, invJ);
    double detJ = fabs(JVol.detJ()[c]);
    for (int gamma=0; gamma<maxDim; gamma++, k++)
    {
      GPtr[k] = detJ*invJ[alpha() + maxDim * gamma]; 
    }
  }
  
  addFlops(flops);
}

