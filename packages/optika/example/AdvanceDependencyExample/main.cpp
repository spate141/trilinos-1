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
#include "Optika_GUI.hpp"
#include "Optika_SpecificParameterEntryValidators.hpp"
#include "Teuchos_StandardParameterEntryValidators.hpp"
#include "Optika_StandardDependencies.hpp"
#include "Optika_DependencySheet.hpp"
#include "Teuchos_FancyOStream.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Optika_StandardConditions.hpp"

  /*
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!              ATTENTION              !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   * !!!!  PLEASE VIEW THE BASIC EXAMPLE AND DEPENDENCY EXAMPLE FIRST BEFORE READING THIS EXAMPLE.      !!!! 
   * !!!!  THEY PROVIDE FUNDAMENTAL KNOWLEDGE THAT WILL BE VERY HELPFUL IN UNDERSTANDING THIS EXAMPLE.  !!!!
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   */ 

//You'll notice we do things a little differently in this example from time to time. That's because I'm showing you now how to write more concise code.
//I made all the previous examples very verbose because I wanted to desmostrate what's going on, but here I'm gonna use some short cuts. I'll try to point them out
//to you as I use them.  

//using this namespaces means we don't have to type Teuchos:: and Optika:: before everything now.
using namespace Teuchos;
using namespace Optika;

//ignore this for now. we'll use it later.
int intFunc(int argument){
	return argument-9;
}

int main(int argc, char* argv[])
{
  //Set up parameterlist and sublist
  //You can name ParameterLists to by passing their name as argument to the constructor.
  RCP<ParameterList> Tramonto_List = RCP<ParameterList>(new ParameterList("Root Tramonto List"));
  //Creating and accessing a sublist are pretty much one in the same. RCPs are good for you, so try to use them whenever you can.
  //Here, instead of just using sublist to access an already defined sublist, we'll actually usin it to create one.
  RCP<ParameterList> surfDefParams = sublist(Tramonto_List, "Surface Definition Parameters");
  //create our dependency sheet. This should look a little different to you. We're using rcp instead of RCP. I'll explain that in a bit.
  RCP<DependencySheet> depSheet1 = rcp(new DependencySheet(Tramonto_List));


  /*
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   Part I: Using Dependencies in Tandem    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   */
  

  //add parameters and validators


  //Teuchos::rcp is a short cut. We bascially don't have to template the rcp function now. It just knows what to template based on the argument it's given.
  //Normally the this line would read:
  //Teuchos::RCP<Optika::EnhancedNumberValidator<int> > nSurfTypesValidator  = Teuchos::RCP<Optika::EnhancedNumberValidator<int> >(new Optika::EnhancedNumberValidator<int>());
  //but by using the namespace stuff I should you above, and the rcp function, we can write it concisely like this:
  RCP<EnhancedNumberValidator<int> > nSurfTypesValidator  = rcp(new EnhancedNumberValidator<int>());
  //Let's say in this case you only want to set a minimum on the validator. We used the empty constructor above (which means no mins or maxs have been set), so we cna just set the min right now.
  nSurfTypesValidator->setMin(0);
  surfDefParams->set("Nsurf_types", 1, "The number of different surface types you have", nSurfTypesValidator);

  //Just setting the min and max now for the nDim Validator
  RCP<EnhancedNumberValidator<int> > nDimVali = rcp(new EnhancedNumberValidator<int>(1,3));
  surfDefParams->set("Ndim", 1, "Number of dimensions", nDimVali);

  //Now for the fun part. We're gonna set up the array. We want to use three different validators, so this array needs to implement: a validatord if Ndim=1, a validator if ndim=2, and a validator for when ndim=3.
  //Lets's start by making those three validators
  RCP<StringValidator> surfTypeVali1 = rcp<StringValidator>(new StringValidator(tuple<std::string>("Infinite Planar Wall", "Finite Planar Wall")));
  RCP<StringValidator> surfTypeVali2 = rcp<StringValidator>(new StringValidator(tuple<std::string>("Infinite Planar Wall", "Finite Planar Wall", "Colloids", "Pore", "Finite Pore", "Tapered Pore")));
  RCP<StringValidator> surfTypeVali3 = rcp<StringValidator>(new StringValidator(tuple<std::string>("Infinite Planar Wall", "Finite Planar Wall", "Colloids", "Atoms", "Point Atoms", "Finite Length Cylinder", "Cylinder with Periodic function for the radius", "Pore", "Finite Pore", "Tapered Pore")));
  //Since we're gonna use them on an array we need to wrap an ArrayStringValidator around each of them. We could've done this in one step, but I think it looks cleaner this way, plus if you ever wanna use just these regular validators again, you can do that.
  RCP<ArrayStringValidator> arraySurfTypeVali1 = rcp(new ArrayStringValidator(surfTypeVali1));
  RCP<ArrayStringValidator> arraySurfTypeVali2 = rcp(new ArrayStringValidator(surfTypeVali2));
  RCP<ArrayStringValidator> arraySurfTypeVali3 = rcp(new ArrayStringValidator(surfTypeVali3));

  //setup the array in accordance with 1-d and 1 surfaces (the defaults we setup above). By doing it this way, if we ever change the default Value for Nsurf_types, we won't have to change it again down here. This is good because it'll also prevent errors (for instance when we
  //change the value above but now down here)
  Array<std::string> surfTypeArray(Teuchos::getValue<int>(surfDefParams->getEntry("Nsurf_types")), "Infinite Planar Wall");
  //same thing here. If we ever change Ndim, won't have to make any changes down here
  switch(Teuchos::getValue<int>(surfDefParams->getEntry("Ndim"))){
	case 1:
  		surfDefParams->set("Surf_Type", surfTypeArray, "Array of surface types", arraySurfTypeVali1);
		break;
	case 2:
  		surfDefParams->set("Surf_Type", surfTypeArray, "Array of surface types", arraySurfTypeVali2);
		break;
	case 3:
  		surfDefParams->set("Surf_Type", surfTypeArray, "Array of surface types", arraySurfTypeVali3);
		break;
	default:
  		surfDefParams->set("Surf_Type", surfTypeArray, "Array of surface types", arraySurfTypeVali1);
		break;
	}

	//Ok, so now for the really really fun part. setting up the dependencies. Let's do the ArrayLengthDependency first.
	//since Nsurf_types and Ndim have the same parent list, we can use a short-cut for the constructors.
	RCP<NumberArrayLengthDependency> surfTypeLengthDep = rcp(
		new NumberArrayLengthDependency(
			"Nsurf_types",
			"Surf_Type",
			surfDefParams
		)
	);
  
  //Now for one of the trickier dependencies, RangeValidatorDependency. First we make a map of ranges to validators. It's kind of overkill for our situation here because our ranges are small. But you get the idea, it scales.
  RangeValidatorDependency<int>::RangeToValidatorMap dimranges;
  dimranges[std::pair<int,int>(1,1)] = arraySurfTypeVali1;
  dimranges[std::pair<int,int>(2,2)] = arraySurfTypeVali2;
  dimranges[std::pair<int,int>(3,3)] = arraySurfTypeVali3;
	
  //here we actually make the dependency. Take a look at the documentation for an explainiation of each argument
  RCP<RangeValidatorDependency<int> > 
	surfTypeValiDep = rcp(
		new RangeValidatorDependency<int>(
			"Ndim", 
			"Surf_Type", 
			surfDefParams,
			dimranges, 
			arraySurfTypeVali1
		)
	);

  //add dependencies
  depSheet1->addDependency(surfTypeLengthDep);
  depSheet1->addDependency(surfTypeValiDep);

  //BOOM, you're good to go. What all this will do is:
  //	-Use different validators on the array based on how many dimensions the user has selected
  //	-Adjust the length of the array based on how many surface types the user has specified.
	

  /*
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!   Part II: Multiple Dependenees and Dependents  !!!!!!!!!!!!!!!!!!!!!!!!!!!
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   */

  /*
   * Up until this point, all the dependencies you've seen have had only one dependee and one dependent.
   * But you've  made it so far in these examples and I'm so proud of you. So I'm gonna let you in on a
   * seceret, all dependencies can actually have multiple dependents. And, if you use the super special
   * ConditionVisualDependency, you can actually have multiple dependees! Holy Cow! I know right?
   * Just blew you mind didn't I?
   */

  //Alright lets start off by creating a new parameter list with some more parametersh
  RCP<ParameterList> crazyDepList = sublist(Tramonto_List, "CRAZY AWESOME DEPENDENCY STUFF...list"); 
  //A number validator
  RCP<EnhancedNumberValidator<int> > scaleValidator = rcp(new EnhancedNumberValidator<int>(1,10));
  // a few parameters
  crazyDepList->set("Are you at all cool?", true, "Hey, it's a valid question.");
  crazyDepList->set("Sweetness", 7, "How sweet are you?", scaleValidator);
  crazyDepList->set("Awesomeness", 7, "How awesome are you?", scaleValidator);
  crazyDepList->set("Special parameter", "", "A parameter that only the coolest of the cool can see");


  //Alrright, first multiple dependents. Let's set up a BoolVisualDependency...WITH MULTIPLE DEPENDENTS!
  //First we make a map that maps the dependents to their parents
  Dependency::ParameterParentMap dependents;
  dependents.insert(std::pair<std::string, Teuchos::RCP<Teuchos::ParameterList> >("Sweetness", crazyDepList));
  dependents.insert(std::pair<std::string, Teuchos::RCP<Teuchos::ParameterList> >("Awesomeness", crazyDepList));
  //Now we only wanna show these parameters if the user is at leaste some one cool. So well make the actual dependency.
  RCP<BoolVisualDependency> boolDep = rcp(new BoolVisualDependency("Are you at all cool?", crazyDepList, dependents, true));
  depSheet1->addDependency(boolDep);
  //There, now those two parameters will only show if the users is at least some what cool.

  //Alright, now that we've got that out off the way, it's time for the creme de la creme,
  //multiple dependees. This one was a doozy for me to implement and requires a little bit
  //of work for both of us.

  //There is only one type of dependency that allows for multiple dependees. The 
  //ConditionVisualDependency. The first thing we gotta do is make to make a condition.
  //This will actually take the place our dependee. There are a few types of conditions.
  //The first and most basic condition is a ParameterCondition: it simply tests the
  //condition of a Parameter.

  //Here's a NumberCondition. It simply checks to see if the value of some number parameter
  //is greater than 0. If greater than 0, the condition evaluate to true. Otherwise,
  //it evaluates false.

  RCP<NumberCondition<int> > intCon1 = rcp(new NumberCondition<int>("Sweetness", crazyDepList, intFunc));
  RCP<NumberCondition<int> > intCon2 = rcp(new NumberCondition<int>("Awesomeness", crazyDepList, intFunc));

  //And here's a Bool Condition.
  RCP<BoolCondition> boolCon1 = rcp(new BoolCondition("Are you at all cool?", crazyDepList));

  //You can then combine conditions using a BinaryLOgicalCondition. A BinaryLogicCondition
  //takes multiple conditions and combines them together.
  
  //Here's an And condition. If all the conditions it's assigned are true, it evaluates
  //to true. First we have to put all our conditions in a list.
  Condition::ConditionList conList1 = tuple<RCP<Condition> >(intCon1, intCon2, boolCon1);

  //And now we make the And Condition
  RCP<AndCondition> andCon1 = rcp(new AndCondition(conList1));
  
  //Now we're ready.
  RCP<ConditionVisualDependency> conVis1 = rcp(new ConditionVisualDependency(andCon1, "Special parameter", crazyDepList, true));

  //There you have it. Now the Special parameter will only be shwon if the Are you cool
  //at all parameter is true and both the Sweetness and Aweseomness parameters are set
  //to 10.
  depSheet1->addDependency(conVis1);

  //Now we run it and bask in all it's rediculous glory.
  getInput(Tramonto_List, depSheet1);
	
  return 0;
  /*
   * Alright, so a few final notes. 
   *  -All of the Parameter Conditions have a bool.
   *   switch that allow you to say "evaluate to true if the condition is false".
   *  -In addition to the Bool and Number Parameter, there's also a String Parameter.
   *  -The other BinaryLogicConditions are Or and Equals.
   *  -There's also a NotCondition that negates the condition given to it.
   *  -You can chain together an arbitrary amount of Conditions. You could give an And condition
   *   a list of conditions that contained some Or coniditoins, which contained some Not conditions
   *   which contained some...we'll you get the idea.
   * That's it! You're now an Optika expert!
   */
}
