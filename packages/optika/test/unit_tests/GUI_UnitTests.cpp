// @HEADER
// ***********************************************************************
// 
//         Optika: A Tool For Developing Parameter Obtaining GUIs
//                Copyright (2009) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, with Sandia Corporation, the 
// U.S. Government retains certain rights in this software.
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
// Questions? Contact Kurtis Nusbaum (klnusbaum@gmail.com) 
// 
// ***********************************************************************
// @HEADER

#include <QtGui>
#include <QtTest/QtTest>
#include "Teuchos_UnitTestHarness.hpp"
#include "Optika_treemodel.hpp"
#include "Optika_delegate.hpp"
#include "Optika_treeview.hpp"
#include <QApplication>
#include <QSpinBox>
#include "Optika_metawindow.hpp"

namespace Optika{

class ModalClicker : public QThread{
public:
  void run();
};

void ModalClicker::run(){
  QWidget* modalDialog = QApplication::activeModalWidget();
  while(modalDialog == NULL){
    modalDialog = QApplication::activeModalWidget();
    msleep(100);
  }
  QTest::keyClick(QApplication::activeModalWidget(), Qt::Key_Return);
}

class OptikaGUITests: public QObject{
Q_OBJECT
private slots:
  void typeTest();
  void dependencyTests();
  void arrayEditorTest();
  void twoDEditorTest();
  void twoDSymmetryTest();
  void modelLoadTest();
  void validatorApplierTests();
  void settingsTest();
  void cleanupTestCase();
private:
  static inline QModelIndex getWidgetIndex(const QModelIndex& index);
  QObjectCleanupHandler cleaner;
  ModalClicker clicker;
};

void OptikaGUITests::cleanupTestCase(){
  cleaner.clear();
  if(clicker.isRunning()){
    clicker.terminate();
    clicker.wait();
  }
  QVERIFY(!clicker.isRunning());
}
  


//QModelIndex OptikaGUITests::getEntryIndex(

#define GET_ENTRY_INDEX(\
  PL, \
  NAME, \
  MODEL) \
  RCP<ParameterEntry> NAME##Entry = PL->getEntryRCP( #NAME  ); \
  QVERIFY(nonnull( NAME##Entry )); \
  QModelIndex NAME##Index = MODEL->findParameterEntryIndex( NAME##Entry ); \
  QVERIFY( NAME##Index.isValid());

#define VERIFY_PARAMETER_TYPE(PL, NAME, TYPE, MODEL) \
  GET_ENTRY_INDEX( PL , NAME , MODEL ) \
  QCOMPARE( MODEL->data( NAME##Index, Qt::DisplayRole).toString(), \
    QString::fromStdString( #NAME) );  \
  QModelIndex NAME##TypeIndex = NAME##Index.sibling(NAME##Index.row(),2); \
  QVERIFY( NAME##TypeIndex.isValid()); \
  QCOMPARE( MODEL->data( NAME##TypeIndex, Qt::DisplayRole).toString(), TYPE );


void OptikaGUITests::typeTest(){
  cleaner.clear();
  RCP<ParameterList> My_List = 
    RCP<ParameterList>(new ParameterList);

  RCP<EnhancedNumberValidator<int> > intVali =
    Teuchos::rcp(new EnhancedNumberValidator<int>(0,2000,3));

  double *pointer = 0;
  Array<double*> doubleStarArray;
  Array<int> intArray;
  My_List->set("Doublepointer", pointer);
  My_List->set(
    "MaxIters", 
    1550, 
    "Determines the maximum number of iterations in the solver",
    intVali);
  My_List->set(
    "Tolerance", 1e-10, "The tolerance used for the convergence check");
  My_List->set("DoublePointerArray", doubleStarArray);
  My_List->set("IntArray", intArray);
  
  TreeModel* model = new TreeModel(My_List);
  cleaner.add(model);

  VERIFY_PARAMETER_TYPE(My_List, MaxIters, intId, model)
  VERIFY_PARAMETER_TYPE(My_List, Doublepointer, unrecognizedId, model)
  VERIFY_PARAMETER_TYPE(My_List, Tolerance, doubleId, model)
  VERIFY_PARAMETER_TYPE(My_List, DoublePointerArray, unrecognizedId, model)
  VERIFY_PARAMETER_TYPE(My_List, IntArray, arrayId + " " + intId, model);
  Delegate* delegate = new Delegate;
  cleaner.add(delegate);

  QStyleOptionViewItem genericStyleItem;
  QModelIndex widgetIndex = getWidgetIndex(MaxItersIndex);
  QSpinBox* intSpin = ((QSpinBox*)delegate->createEditor(0, genericStyleItem, widgetIndex));
  QCOMPARE(intSpin->maximum(), 2000);
  QCOMPARE(intSpin->minimum(),0);
  QCOMPARE(intSpin->singleStep(),3);


  cleaner.remove(model);
  cleaner.remove(delegate);
  delete model; 
  delete delegate;
}


void OptikaGUITests::arrayEditorTest(){
  Array<double> testArray(4,4.5);
  ParameterEntry testEntry(testArray);
  DoubleArrayWidget* testWidget = new DoubleArrayWidget("tester", doubleId, null);
  cleaner.add(testWidget);

  testWidget->initData(testArray);
  Array<double> retrievedArray = testWidget->getArrayFromWidgets();
  QVERIFY(testArray == retrievedArray);

  cleaner.remove(testWidget);
  delete testWidget;
}

void OptikaGUITests::twoDEditorTest(){
  TwoDArray<double> testArray(4,2,4.5);
  ParameterEntry testEntry(testArray);
  Double2DArrayWidget* testWidget = 
    new Double2DArrayWidget("tester", doubleId, null);
  cleaner.add(testWidget);

  testWidget->initData(testArray);
  testWidget->accept();
  TwoDArray<double> retrievedArray = testWidget->getData();
  QVERIFY(testArray == retrievedArray);


  cleaner.remove(testWidget);
  delete testWidget;

}

inline QModelIndex OptikaGUITests::getWidgetIndex(const QModelIndex& index){
  return index.sibling(index.row(),1);
}
  
  
#define VERIFY_HIDDEN_ROW(INDEX) \
  QVERIFY(treeView->isRowHidden(  INDEX.row(), INDEX.parent()));

#define VERIFY_SHOWN_ROW(INDEX) \
  QVERIFY(!treeView->isRowHidden(  INDEX.row(), INDEX.parent()));

void OptikaGUITests::dependencyTests(){
  cleaner.clear();
  RCP<DependencySheet> dependencySheet = rcp(new DependencySheet);
  RCP<ParameterList> validParameters = 
    getParametersFromXmlFile("deptests.xml", dependencySheet);
  TreeModel* model = new TreeModel(validParameters, dependencySheet);
  Delegate* delegate = new Delegate;
  TreeView* treeView = new TreeView(model, delegate);
  cleaner.add(model);
  cleaner.add(delegate);
  cleaner.add(treeView);
  QStyleOptionViewItem genericStyleItem;
  
//Testing Bool visual dependency
  GET_ENTRY_INDEX(validParameters, Preconditioner, model)
  GET_ENTRY_INDEX(validParameters, ShowPrecs, model)

  VERIFY_HIDDEN_ROW(PreconditionerIndex)
  QModelIndex precWidgetIndex = getWidgetIndex(ShowPrecsIndex);
  QComboBox* precBox = (QComboBox*)delegate->createEditor(
    0, genericStyleItem, precWidgetIndex);
  precBox->setCurrentIndex(precBox->findText(Delegate::getBoolEditorTrue()));
  delegate->setModelData(precBox, model, precWidgetIndex);
  VERIFY_SHOWN_ROW(PreconditionerIndex)


//StringVisualDependency testing
  GET_ENTRY_INDEX(validParameters, Favorite_Cheese, model)
  GET_ENTRY_INDEX(validParameters, Swiss_rating, model)

  VERIFY_HIDDEN_ROW(Swiss_ratingIndex)
  QModelIndex cheeseWidgetIndex = getWidgetIndex(Favorite_CheeseIndex);
  QComboBox* cheeseBox = (QComboBox*)delegate->createEditor(
    0,genericStyleItem, cheeseWidgetIndex);
  cheeseBox->setCurrentIndex(cheeseBox->findText("Swiss"));
  delegate->setModelData(cheeseBox, model, cheeseWidgetIndex);
  VERIFY_SHOWN_ROW(Swiss_ratingIndex)
 
//Testing Number Visual Dependencies
  GET_ENTRY_INDEX(validParameters, Temp, model)
  GET_ENTRY_INDEX(validParameters, Num_ice_cubes, model)
  VERIFY_SHOWN_ROW(Num_ice_cubesIndex)
  QModelIndex tempWidgetIndex = getWidgetIndex(TempIndex);
  QLineEdit* tempLineEdit = (QLineEdit*)delegate->createEditor(
    0,genericStyleItem, tempWidgetIndex);
  tempLineEdit->setText("33.0");
  delegate->setModelData(tempLineEdit, model, tempWidgetIndex);
  VERIFY_HIDDEN_ROW(Num_ice_cubesIndex)

  //Test condition visual dependency
  GET_ENTRY_INDEX(validParameters, ParamA, model)
  GET_ENTRY_INDEX(validParameters, ParamB, model)
  GET_ENTRY_INDEX(validParameters, OptParam, model)
  VERIFY_SHOWN_ROW(OptParamIndex)
  QModelIndex paramAWidgetIndex = getWidgetIndex(ParamAIndex);
  QModelIndex paramBWidgetIndex = getWidgetIndex(ParamBIndex);
  QSpinBox* paramASpinner = (QSpinBox*)delegate->createEditor(
    0, genericStyleItem, paramAWidgetIndex);
  QSpinBox* paramBSpinner = (QSpinBox*)delegate->createEditor(
    0, genericStyleItem, paramBWidgetIndex);
  paramASpinner->setValue(0);
  delegate->setModelData(paramASpinner, model, paramAWidgetIndex);
  VERIFY_SHOWN_ROW(OptParamIndex)
  paramBSpinner->setValue(0);
  delegate->setModelData(paramBSpinner, model, paramBWidgetIndex);
  VERIFY_HIDDEN_ROW(OptParamIndex)
  paramBSpinner->setValue(1);
  delegate->setModelData(paramBSpinner, model, paramBWidgetIndex);
  VERIFY_SHOWN_ROW(OptParamIndex)


  //Test Number Array Length Dependency
  GET_ENTRY_INDEX(validParameters, NumBuckets, model)
  GET_ENTRY_INDEX(validParameters, AmtInBuckets, model)
  Array<double> bucketsArray = model->getArray<double>(AmtInBucketsIndex);
  QCOMPARE(bucketsArray.size(),(Array<double>::size_type)3);
  QModelIndex numBucketsWidgetIndex = getWidgetIndex(NumBucketsIndex);
  QSpinBox* numBucketsSpinner = (QSpinBox*)delegate->createEditor(
    0, genericStyleItem, numBucketsWidgetIndex);
  numBucketsSpinner->setValue(5);
  delegate->setModelData(numBucketsSpinner, model, numBucketsWidgetIndex);
  bucketsArray = model->getArray<double>(AmtInBucketsIndex);
  QCOMPARE(bucketsArray.size(),(Array<double>::size_type)5);
  numBucketsSpinner->setValue(0);
  delegate->setModelData(numBucketsSpinner, model, numBucketsWidgetIndex);
  bucketsArray = model->getArray<double>(AmtInBucketsIndex);
  VERIFY_HIDDEN_ROW(AmtInBucketsIndex)
  QCOMPARE(bucketsArray.size(),(Array<double>::size_type)0);
  numBucketsSpinner->setValue(2);
  delegate->setModelData(numBucketsSpinner, model, numBucketsWidgetIndex);
  bucketsArray = model->getArray<double>(AmtInBucketsIndex);
  VERIFY_SHOWN_ROW(AmtInBucketsIndex)
  QCOMPARE(bucketsArray.size(),(Array<double>::size_type)2);


  //Testing for Bool ValidatorDependency
  GET_ENTRY_INDEX(validParameters, TempConst, model)
  GET_ENTRY_INDEX(validParameters, BoolTemp, model)
  QVERIFY(nonnull(model->getValidator(BoolTempIndex)));
  QModelIndex tempConstWidgetIndex = getWidgetIndex(TempConstIndex);
  QModelIndex boolTempWidgetIndex = getWidgetIndex(BoolTempIndex);
  QLineEdit* boolTempEdit = (QLineEdit*)delegate->createEditor(
    0, genericStyleItem, boolTempWidgetIndex);
  QCOMPARE(((QDoubleValidator*)boolTempEdit->validator())->bottom(), 0.0);
  QCOMPARE(((QDoubleValidator*)boolTempEdit->validator())->top(), 50.0);
  QComboBox* tempConstCombo = (QComboBox*)delegate->createEditor(
    0, genericStyleItem, tempConstWidgetIndex);
  tempConstCombo->setCurrentIndex(
    tempConstCombo->findText(Delegate::getBoolEditorFalse()));
  delegate->setModelData(tempConstCombo, model, tempConstWidgetIndex);
  QVERIFY(model->getValidator(BoolTempIndex).is_null());
  boolTempEdit = (QLineEdit*)delegate->createEditor(
    0, genericStyleItem, boolTempWidgetIndex);
  QCOMPARE(((QDoubleValidator*)boolTempEdit->validator())->bottom(), EnhancedNumberTraits<double>::min());
  QCOMPARE(((QDoubleValidator*)boolTempEdit->validator())->top(), EnhancedNumberTraits<double>::max());


  //StringValidatorDepenecy tests
  GET_ENTRY_INDEX(validParameters, FavFoodType, model)
  GET_ENTRY_INDEX(validParameters, FavFood, model)
  QVERIFY(nonnull(model->getValidator(FavFoodIndex)));
  QModelIndex favFoodWidgetIndex = getWidgetIndex(FavFoodIndex);
  QComboBox* favFoodCombo = (QComboBox*)delegate->createEditor(
    0, genericStyleItem, favFoodWidgetIndex);
  QCOMPARE(favFoodCombo->count(),3);
  QVERIFY(favFoodCombo->findText("American") != -1);
  QVERIFY(favFoodCombo->findText("Swiss") != -1);
  QVERIFY(favFoodCombo->findText("Pepperjack") != -1);

  //Change to chips and verify new valid values
  QSignalSpy badValSpy(model, SIGNAL(badValue(QModelIndex, QString)));
  QModelIndex favFoodTypeWidgetIndex = getWidgetIndex(FavFoodTypeIndex);
  QLineEdit* favFoodTypeEdit = (QLineEdit*)delegate->createEditor(
    0, genericStyleItem, favFoodTypeWidgetIndex);
  favFoodTypeEdit->setText("Chips");
  clicker.start(QThread::IdlePriority);
  delegate->setModelData(favFoodTypeEdit, model, favFoodTypeWidgetIndex);
  QCOMPARE(badValSpy.count(), 1);
  favFoodCombo = (QComboBox*)delegate->createEditor(
    0, genericStyleItem, favFoodWidgetIndex);
  QCOMPARE(favFoodCombo->count(),3);
  QVERIFY(favFoodCombo->findText("Lays") != -1);
  QVERIFY(favFoodCombo->findText("Ruffles") != -1);
  QVERIFY(favFoodCombo->findText("Pringles") != -1);

  //Change to blah and validate ther validator is now null
  favFoodTypeEdit->setText("blah");
  delegate->setModelData(favFoodTypeEdit, model, favFoodTypeWidgetIndex);
  QVERIFY(model->getValidator(FavFoodIndex).is_null());

  //Change Back to chips and verify that the valid vlues have returned.
  favFoodTypeEdit->setText("Chips");
  delegate->setModelData(favFoodTypeEdit, model, favFoodTypeWidgetIndex);
  favFoodCombo = (QComboBox*)delegate->createEditor(
    0, genericStyleItem, favFoodWidgetIndex);
  QCOMPARE(favFoodCombo->count(),3);
  QVERIFY(favFoodCombo->findText("Lays") != -1);
  QVERIFY(favFoodCombo->findText("Ruffles") != -1);
  QVERIFY(favFoodCombo->findText("Pringles") != -1);


  //Test RangeValidatorDependency
  GET_ENTRY_INDEX(validParameters, FondTemp, model)
  GET_ENTRY_INDEX(validParameters, FondFood, model)
  QVERIFY(nonnull(model->getValidator(FondFoodIndex)));
  QModelIndex fondFoodWidgetIndex = getWidgetIndex(FondFoodIndex);
  QComboBox* fondFoodCombo = (QComboBox*)delegate->createEditor(
    0, genericStyleItem, fondFoodWidgetIndex);
  QCOMPARE(fondFoodCombo->count(), 2);
  QVERIFY(fondFoodCombo->findText("Cheese") != -1);
  QVERIFY(fondFoodCombo->findText("Bread") != -1);

  QModelIndex fondTempWidgetIndex = getWidgetIndex(FondTempIndex);
  QLineEdit* fondTempEdit = (QLineEdit*)delegate->createEditor(
    0, genericStyleItem, fondTempWidgetIndex);
  fondTempEdit->setText("120.1");
  delegate->setModelData(fondTempEdit, model, fondTempWidgetIndex);
  fondFoodCombo = (QComboBox*)delegate->createEditor(
    0, genericStyleItem, fondFoodWidgetIndex);
  QCOMPARE(fondFoodCombo->count(), 2);
  QVERIFY(fondFoodCombo->findText("Chicken") != -1);
  QVERIFY(fondFoodCombo->findText("Beef") != -1);

  fondTempEdit->setText("180.1");
  delegate->setModelData(fondTempEdit, model, fondTempWidgetIndex);
  QVERIFY(model->getValidator(FondFoodIndex).is_null());
  QLineEdit* fondFoodLineEdit = (QLineEdit*)delegate->createEditor(
    0, genericStyleItem, fondFoodWidgetIndex);
  QVERIFY(fondFoodLineEdit != NULL);
  

  fondTempEdit->setText("90");
  delegate->setModelData(fondTempEdit, model, fondTempWidgetIndex);
  fondFoodCombo = (QComboBox*)delegate->createEditor(
    0, genericStyleItem, fondFoodWidgetIndex);
  QCOMPARE(fondFoodCombo->count(), 2);
  QVERIFY(fondFoodCombo->findText("Cheese") != -1);
  QVERIFY(fondFoodCombo->findText("Bread") != -1);


  //Test TwoDRowDependency
  GET_ENTRY_INDEX(validParameters, NumRows, model)
  GET_ENTRY_INDEX(validParameters, RowArray, model)
  QModelIndex numRowsWidgetIndex = getWidgetIndex(NumRowsIndex);
  QSpinBox* numRowSpin = (QSpinBox*)delegate->createEditor(
    0, genericStyleItem, numRowsWidgetIndex);
  numRowSpin->setValue(2);
  delegate->setModelData(numRowSpin, model, numRowsWidgetIndex);
  TwoDArray<double> rowArray = model->getTwoDArray<double>(RowArrayIndex);
  QCOMPARE(rowArray.getNumRows(), (TwoDArray<double>::size_type)2);

  //Test TwoDColDependency
  GET_ENTRY_INDEX(validParameters, NumCols, model)
  GET_ENTRY_INDEX(validParameters, ColArray, model)
  QModelIndex numColsWidgetIndex = getWidgetIndex(NumColsIndex);
  QSpinBox* numColSpin = (QSpinBox*)delegate->createEditor(
    0, genericStyleItem, numColsWidgetIndex);
  numColSpin->setValue(2);
  delegate->setModelData(numColSpin, model, numColsWidgetIndex);
  TwoDArray<double> colArray = model->getTwoDArray<double>(ColArrayIndex);
  QCOMPARE(colArray.getNumCols(), (TwoDArray<double>::size_type)2);
 

  

  cleaner.remove(model);
  cleaner.remove(treeView);
  cleaner.remove(delegate);
  delete model;
  delete treeView;
  delete delegate;
}
  

void OptikaGUITests::twoDSymmetryTest(){
  cleaner.clear();
  TwoDArray<double> testArray(4,4,4.5);
  testArray.setSymmetrical(true);
  Double2DArrayWidget* testWidget = 
    new Double2DArrayWidget("tester", doubleId, null);
  cleaner.add(testWidget);

  testWidget->initData(testArray);
  QGridLayout* layout = (QGridLayout*)testWidget->layout();
  QScrollArea* scrollArea = (QScrollArea*)(layout->itemAtPosition(0,0)->widget());
  QWidget* actualWidget = scrollArea->widget();
  QCOMPARE(((QGridLayout*)actualWidget->layout())->itemAtPosition(1,1), (QLayoutItem*)0);

  testWidget->accept();
  TwoDArray<double> retrievedArray = testWidget->getData();
  QVERIFY(testArray == retrievedArray);


  cleaner.remove(testWidget);
  delete testWidget;


}

void OptikaGUITests::modelLoadTest(){
  cleaner.clear();
  RCP<ParameterList> validParameters = 
    getParametersFromXmlFile("loadtest.xml");
  TreeModel* model = new TreeModel(validParameters);
  Delegate* delegate = new Delegate;
  TreeView* treeView = new TreeView(model, delegate);
  cleaner.add(model);
  cleaner.add(delegate);
  cleaner.add(treeView);

  QCOMPARE(model->getCurrentParameters()->get<int>("Steve"), 4);
  QCOMPARE(model->getCurrentParameters()->get<int>("Sam"), 4);
  QCOMPARE(model->getCurrentParameters()->sublist("Prec").get<int>("Sam"), 9);
  QCOMPARE(model->getCurrentParameters()->sublist("Prec").get<int>("Blah"), 1);
  model->readInput("loadtest.in.xml");
  QCOMPARE(model->getCurrentParameters()->get<int>("Steve"), 0);
  QCOMPARE(model->getCurrentParameters()->sublist("Prec").get<int>("Blah"), 80);
  QCOMPARE(model->getCurrentParameters()->sublist("Prec").get<int>("Sam"), 99);
  QCOMPARE(model->getCurrentParameters()->get<int>("Sam"), 50);

  cleaner.remove(model);
  cleaner.remove(treeView);
  cleaner.remove(delegate);
  delete model;
  delete treeView;
  delete delegate;

}

template<class T>
void testingSpinBoxApply(
  const RCP<EnhancedNumberValidator<T> > validator,
  QAbstractSpinBox* spinner)
{
  ValidatorApplier<T>::applyToSpinBox(validator, (QSpinBox*)spinner);
}

template<>
void testingSpinBoxApply(
  const RCP<EnhancedNumberValidator<double> > validator,
  QAbstractSpinBox* spinner)
{
  ValidatorApplier<double>::applyToSpinBox(validator, (QDoubleSpinBox*)spinner);
}

template<>
void testingSpinBoxApply(
  const RCP<EnhancedNumberValidator<float> > validator,
  QAbstractSpinBox* spinner)
{
  ValidatorApplier<float>::applyToSpinBox(validator, (QDoubleSpinBox*)spinner);
}

template<class T>
void valApplyTestTemplate(){
  EnhancedNumberValidator<T> validator(Teuchos::as<T>(0), Teuchos::as<T>(10));
  RCP<EnhancedNumberValidator<T> > valPtr = rcpFromRef(validator);
  std::string myType = Teuchos::TypeNameTraits<T>::name();
  bool isFloatingType = 
    myType == Teuchos::TypeNameTraits<double>::name() ||
    myType == Teuchos::TypeNameTraits<float>::name();

  QString val10("10");
  QString val20("20");
  QString val5("5");
  QString val0("0");
  QString valneg1("-1");


  /** Do spinner testing */
  QAbstractSpinBox* spinner = NULL;
  if(isFloatingType)
  {
    spinner = new QDoubleSpinBox();
  }
  else{
    spinner = new QSpinBox();
  }
  testingSpinBoxApply(valPtr, spinner);
  int pos=0;
  QCOMPARE(spinner->validate(val10,pos), QValidator::Acceptable);
  QCOMPARE(spinner->validate(val20,pos), QValidator::Invalid);
  QCOMPARE(spinner->validate(val5,pos), QValidator::Acceptable);
  QCOMPARE(spinner->validate(val0,pos), QValidator::Acceptable);
  QCOMPARE(spinner->validate(valneg1,pos), QValidator::Invalid);
  if(isFloatingType){
    QDoubleSpinBox* actualSpinner = (QDoubleSpinBox*)spinner;
    QCOMPARE(actualSpinner->decimals(), Teuchos::as<int>(EnhancedNumberTraits<T>::defaultPrecision()));
    QCOMPARE(Teuchos::as<T>(actualSpinner->singleStep()), EnhancedNumberTraits<T>::defaultStep());
  }
  if(isFloatingType){
    QSpinBox* actualSpinner = (QSpinBox*)spinner;
    QCOMPARE(Teuchos::as<T>(actualSpinner->singleStep()), EnhancedNumberTraits<T>::defaultStep());
  }
  delete spinner;

  /** Do lineedit testing */
  QLineEdit lineEdit;
  ValidatorApplier<T>::applyToLineEdit(valPtr, &lineEdit);
  const QValidator* appliedValidator = lineEdit.validator();
  QCOMPARE(appliedValidator->validate(val10,pos), QValidator::Acceptable);
  QCOMPARE(appliedValidator->validate(val5,pos), QValidator::Acceptable);
  QCOMPARE(appliedValidator->validate(val0,pos), QValidator::Acceptable);
  if(isFloatingType){
    QCOMPARE(appliedValidator->validate(val20,pos), QValidator::Intermediate);
    QCOMPARE(appliedValidator->validate(valneg1,pos), QValidator::Invalid);
    const QDoubleValidator* doubleValidator = (QDoubleValidator*)appliedValidator;
    QCOMPARE(doubleValidator->decimals(), Teuchos::as<int>(EnhancedNumberTraits<T>::defaultPrecision()));
  }
  else{
    QCOMPARE(appliedValidator->validate(val20,pos), QValidator::Invalid);
    QCOMPARE(appliedValidator->validate(valneg1,pos), QValidator::Invalid);
  }
}

void OptikaGUITests::validatorApplierTests(){
  valApplyTestTemplate<int>();
  valApplyTestTemplate<short>();
  valApplyTestTemplate<double>();
  valApplyTestTemplate<float>();
}

void OptikaGUITests::settingsTest(){
  RCP<ParameterList> validParameters = rcp(new ParameterList("Steve"));
  validParameters->set("Don't care", "don't care");
  MetaWindow* m1 = new MetaWindow(validParameters);
  m1->move(30,99);
  m1->resize(673,823);
  delete m1;
  MetaWindow* m2 = new MetaWindow(validParameters);
  QCOMPARE(m2->width(),673);
  QCOMPARE(m2->height(),823);
  QCOMPARE(m2->x(),30);
  QCOMPARE(m2->y(),99);
  delete m2;
}

} //namespace Optika

QTEST_MAIN(Optika::OptikaGUITests)
#include "GUI_UnitTests.moc"

