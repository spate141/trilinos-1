// -*- Mode : c++; tab-width: 3; c-tab-always-indent: t; indent-tabs-mode: nil; c-basic-offset: 3 -*-
//
//   SUMMARY: 
//     USAGE:
//
//    AUTHOR: Thomas Leurent <tleurent@mcs.anl.gov>
//       ORG: Argonne National Laboratory
//    E-MAIL: tleurent@mcs.anl.gov
//
// ORIG-DATE: 13-Nov-02 at 18:05:56
//  LAST-MOD: 16-Dec-02 at 17:42:10 by Thomas Leurent
//
// DESCRIPTION:
// ============
/*! \file ObjectiveFunctionTest.cpp

Unit testing of various functions in the ObjectiveFunction class. 

*/
// DESCRIP-END.
//



#include "Mesquite.hpp"
#include "ObjectiveFunction.hpp"
#include "LPTemplate.hpp"
#include "CompositeOFAdd.hpp"
#include "CompositeOFScalarMultiply.hpp"
#include "GeneralizedConditionNumberQualityMetric.hpp"
#include "MeanRatioQualityMetric.hpp"

#include "PatchDataInstances.hpp"

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/SignalException.h"

#include <list>
#include <iterator>

using namespace Mesquite;

class ObjectiveFunctionTest : public CppUnit::TestFixture
{
private:
  CPPUNIT_TEST_SUITE(ObjectiveFunctionTest);
  CPPUNIT_TEST (test_get_quality_metric_list);
  //  CPPUNIT_TEST (test_compute_gradient_2D_LPTemplate);
  CPPUNIT_TEST (test_compute_gradient_3D_LPTemplate);
  CPPUNIT_TEST_SUITE_END();
   
private:
   
  PatchData m4Quads;
  PatchData m6Quads;
  PatchData m12Hex;

public:
  void setUp()
  {
    MsqError err;
     
    /* our 2D set up: 4 quads, center vertex outcentered by (0,-0.5)
       7____6____5
       |    |    |
       | 2  |  3 |
       8-_  |  _-4       vertex 1 is at (0,0)
       |  -_0_-  |       vertex 5 is at (2,2)
       | 0  |  1 |
       1----2----3
    */
    create_four_quads_patch(m4Quads, err); MSQ_CHKERR(err);

    /*! \fn create_six_quads_patch(PatchData &four_quads, MsqError &err)
      our 2D set up: 6 quads, 1 center vertex outcentered by (0,-0.5), the other centered
      7____6____5___11
      |    |    |    |
      | 2  |  3 | 5  |
      8-_  |  _-4---10       vertex 1 is at (0,0)
      |  -_0_-  |    |       vertex 11 is at (3,2)
      | 0  |  1 | 4  |
      1----2----3----9
    */
    create_six_quads_patch(m6Quads, err); MSQ_CHKERR(err);

    /*! \fn create_twelve_hex_patch(PatchData &pd, MsqError &err)
      3D set up: 12 quads, one center vertex outcentered by (0,-0.5),
      the other centered. Vertex 1 is at (0,0,-1). Vertex 35 is at (3,2,1).
     
      7____6____5___11     19___18____17__23     31___30___29___35
      |    |    |    |      |    |    |    |      |    |    |    |
      | 2  |  3 | 5  |      |    |    |    |      | 8  |  9 | 11 |
      8----0----4---10     20-_  |  _16---22     32---24---28---34       
      |    |    |    |      |  -12_-  |    |      |    |    |    |       
      | 0  |  1 | 4  |      |    |    |    |      | 6  |  7 | 10 |
      1----2----3----9     13---14---15---21     25---26---27---33
    */
    create_twelve_hex_patch(m12Hex, err); MSQ_CHKERR(err);
  }

  void tearDown()
  {
  }
  
public:
  ObjectiveFunctionTest()
  {}
  
  void test_get_quality_metric_list()
  {
    MsqError err;
      
    // instantiates a couple of QualityMetrics
    ShapeQualityMetric* mean_ratio = MeanRatioQualityMetric::create_new();
    ShapeQualityMetric* condition_nb = GeneralizedConditionNumberQualityMetric::create_new();

    // and creates a composite objective function.
    LPTemplate* LP2_mean_ratio = new LPTemplate(mean_ratio, 2, err); MSQ_CHKERR(err);
    LPTemplate* LP2_condition_nb = new LPTemplate(condition_nb, 2, err); MSQ_CHKERR(err);
    CompositeOFScalarMultiply* LP2_condition_nb_x3 = new CompositeOFScalarMultiply(3,LP2_condition_nb);   
    CompositeOFAdd comp_OF(LP2_mean_ratio, LP2_condition_nb_x3);

    // test the (simple) get_quality_metric function, on OF with 1 QM. 
    QualityMetric* QM_;
    QM_ = LP2_mean_ratio->get_quality_metric();
    CPPUNIT_ASSERT(QM_==mean_ratio);
    // test the (simple) get_quality_metric function, on OF with 2 QMs. 
    QM_ = comp_OF.get_quality_metric();
    CPPUNIT_ASSERT(QM_==NULL);

    // test the get_quality_metric_list function, on OF with 1 QM. 
    std::list<QualityMetric*> QM__;
    QM__ = LP2_mean_ratio->get_quality_metric_list();
    CPPUNIT_ASSERT( QM__.size()==1 );
    CPPUNIT_ASSERT( *(QM__.begin())==mean_ratio );
    // test the get_quality_metric_list function, on OF with 2 QMs. 
    QM__.clear();
    QM__ = comp_OF.get_quality_metric_list();
    CPPUNIT_ASSERT( QM__.size()==2 );
    std::list<QualityMetric*>::const_iterator QM2 = QM__.begin();
    std::list<QualityMetric*>::const_iterator QM1 = QM2++;;
    CPPUNIT_ASSERT( *QM1==mean_ratio   || *QM2==mean_ratio &&
                    *QM1==condition_nb || *QM2==condition_nb  );

    delete mean_ratio;
    delete condition_nb;
    delete LP2_mean_ratio;
    delete LP2_condition_nb;
    delete LP2_condition_nb_x3;
  }

  void test_compute_gradient_2D_LPTemplate()
  {
    MsqError err;
    Vector3D* grad = new Vector3D[2];
    
    // creates a mean ratio quality metric ...
    ShapeQualityMetric* mean_ratio = MeanRatioQualityMetric::create_new();
    // ... and builds an objective function with it
    LPTemplate* LP2 = new LPTemplate(mean_ratio, 2, err);

    LP2->set_gradient_type(ObjectiveFunction::NUMERICAL_GRADIENT);
    LP2->compute_gradient(m6Quads, grad, err);
    
    LP2->set_gradient_type(ObjectiveFunction::ANALYTICAL_GRADIENT);
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    LP2->compute_gradient(m6Quads, grad, err);
    

    delete grad;
  }
   
  void test_compute_gradient_3D_LPTemplate()
  {
    MsqError err;
    Vector3D* grad = new Vector3D[2];
    
    // creates a mean ratio quality metric ...
    ShapeQualityMetric* mean_ratio = MeanRatioQualityMetric::create_new();
    mean_ratio->set_averaging_method(QualityMetric::LINEAR, err);
    
    // ... and builds an objective function with it
    LPTemplate* LP2 = new LPTemplate(mean_ratio, 2, err);
    
    LP2->set_gradient_type(ObjectiveFunction::NUMERICAL_GRADIENT);
    LP2->compute_gradient(m12Hex, grad, err);
    std::cout << "NUMERICAL GRADIENT\n";
    for (int i=0; i<2; ++i)
      for (int j=0; j<3; ++j)
        std::cout << grad[i][j] << std::endl;
    
    LP2->set_gradient_type(ObjectiveFunction::ANALYTICAL_GRADIENT);
    mean_ratio->set_gradient_type(QualityMetric::ANALYTICAL_GRADIENT);
    LP2->compute_gradient(m12Hex, grad, err);
    std::cout << "ANALYTICAL GRADIENT\n";
    for (int i=0; i<2; ++i)
      for (int j=0; j<3; ++j)
        std::cout << grad[i][j] << std::endl;
  
    delete grad;
  }
   
};


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(ObjectiveFunctionTest, "ObjectiveFunctionTest");

