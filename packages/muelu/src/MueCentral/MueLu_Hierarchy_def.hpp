#ifndef MUELU_HIERARCHY_DEF_HPP
#define MUELU_HIERARCHY_DEF_HPP

#include "MueLu_Hierarchy_decl.hpp"
#include "MueLu_Level.hpp"
#include "MueLu_HierarchyHelpers.hpp"
#include "MueLu_Monitor.hpp"
#include "MueLu_SmootherFactoryBase.hpp"
#include "MueLu_SmootherFactory.hpp"
#include "MueLu_SmootherBase.hpp"
#include "MueLu_FactoryManager.hpp"
#include "MueLu_PFactory.hpp"
#include "MueLu_RFactory.hpp"
#include "MueLu_Utilities.hpp"

namespace MueLu {

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Hierarchy()
    : maxCoarseSize_(50), implicitTranspose_(false)
  {
    AddLevel(rcp( new Level() ));
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Hierarchy(const RCP<Operator> & A)
    :  maxCoarseSize_(50), implicitTranspose_(false)
  {
    RCP<Level> Finest = rcp( new Level() );
    AddLevel(Finest);

    Finest->Set("A", A);
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::~Hierarchy() { }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::SetMaxCoarseSize(Xpetra::global_size_t const &maxCoarseSize) { maxCoarseSize_ = maxCoarseSize; }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  Xpetra::global_size_t Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::GetMaxCoarseSize() const { return maxCoarseSize_; }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  int Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::LastLevelID() const { return Levels_.size() - 1; }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::AddLevel(const RCP<Level> & level) {
    int levelID = LastLevelID() + 1; // ID of the inserted level

    if (level->GetLevelID() != -1 && (level->GetLevelID() != levelID))
      GetOStream(Warnings1, 0) << "Warning: Hierarchy::AddLevel(): Level with ID=" << level->GetLevelID() << " have been added at the end of the hierarchy" << std::endl
                               << "         but its ID have been redefined because last level ID of the hierarchy was " << LastLevelID() << "." << std::endl;

    Levels_.push_back(level);
    level->SetLevelID(levelID);

    if (levelID == 0)
      level->SetPreviousLevel(Teuchos::null);
    else
      level->SetPreviousLevel(Levels_[LastLevelID() - 1]);
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::AddNewLevel() {
    RCP<Level> newLevel = Levels_[LastLevelID()]->Build(); // new coarse level, using copy constructor
    this->AddLevel(newLevel);                              // add to hierarchy
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  RCP<Level> & Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::GetLevel(const int levelID) {
    TEUCHOS_TEST_FOR_EXCEPTION(levelID < 0 || levelID > LastLevelID(), Exceptions::RuntimeError, "MueLu::Hierarchy::GetLevel(): invalid input parameter value: LevelID = " << levelID);
    return Levels_[levelID];
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  LocalOrdinal Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::GetNumLevels() const { return Levels_.size(); }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::SetImplicitTranspose(const bool &implicit) { implicitTranspose_ = implicit; }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  bool Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::GetImplicitTranspose() const { return implicitTranspose_; }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  Teuchos::ParameterList Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::FullPopulate(const FactoryBase & PFact, 
                                                 const FactoryBase & RFact, 
                                                 const TwoLevelFactoryBase & AcFact, 
                                                 const SmootherFactory & SmooFact, 
                                                 const int &startLevel, const int &numDesiredLevels) {

    // Note: It's OK to use rcpFromRef here, because data will only be kept by the FactoryManager
    //       and the FactoryManager is deleted at the end of this function.

    FactoryManager manager(rcpFromRef(PFact), rcpFromRef(RFact), rcpFromRef(AcFact));
    manager.SetFactory("CoarseSolver", Teuchos::null);
    manager.SetFactory("Smoother", rcpFromRef(SmooFact));

    return Setup(manager, startLevel, numDesiredLevels);
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  Teuchos::ParameterList Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::FillHierarchy(const PFactory & PFact, const RFactory & RFact, 
                                       const TwoLevelFactoryBase & AcFact, 
                                       const int startLevel, const int numDesiredLevels) {
    FactoryManager manager(rcpFromRef(PFact), rcpFromRef(RFact), rcpFromRef(AcFact));
    manager.SetFactory("Smoother",     Teuchos::null); //? TODO remove
    manager.SetFactory("CoarseSolver", Teuchos::null);

    return Setup(manager, startLevel, numDesiredLevels);

  } // FillHierarchy
  
  
  
  // Coherence checks todo in Setup() (using an helper function):
  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::CheckLevel(Level& level, int levelID) {
    TEUCHOS_TEST_FOR_EXCEPTION(level.GetLevelID() != levelID, Exceptions::RuntimeError, "MueLu::Hierarchy::CheckLevel(): wrong level ID");
    TEUCHOS_TEST_FOR_EXCEPTION(levelID != 0 && level.GetPreviousLevel() != Levels_[levelID-1], Exceptions::RuntimeError, "MueLu::Hierarchy::Setup(): wrong level parent");
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  bool Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Setup(int coarseLevelID, const FactoryManager & fineLevelManager, const FactoryManager &coarseLevelManager, 
                                                                                const FactoryManager & nextLevelManager /*should be optional*/, 
                                                                                bool isFinestLevel /*to be removed (the finestLevelManager can be customized to do the same)*/,
                                                                                bool isLastLevel /* idem */) {

    //TODO
    typedef MueLu::TopRAPFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> TopRAPFactory;
    typedef MueLu::TopSmootherFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> TopSmootherFactory;

    //
    // Init
    //
    
    //    int coarseLevelID = LastLevelID() - 1; // Level built by this function
    TEUCHOS_TEST_FOR_EXCEPTION(LastLevelID() < coarseLevelID, Exceptions::RuntimeError, "MueLu::Hierarchy:Setup(): level " << coarseLevelID << " (specified by coarseLevelID argument) must be build before calling this function.");
    CheckLevel(*Levels_[coarseLevelID], coarseLevelID);

    // Safe to use rcpFromRef here as all objects using this RCP are destroyed at the end of this function.
    RCP<const FactoryManager> fineManager   = rcpFromRef(fineLevelManager);
    RCP<const FactoryManager> coarseManager = rcpFromRef(coarseLevelManager);
    RCP<const FactoryManager> nextManager   = rcpFromRef(nextLevelManager);
    
    //
    // Requests for finest level
    //
    if (isFinestLevel) {
      Levels_[coarseLevelID]->Request(TopSmootherFactory(coarseManager, "Smoother")); // TODO: skip this line if we know that it is the lastLevel
      Levels_[coarseLevelID]->Request(TopSmootherFactory(coarseManager, "CoarseSolver"));
    }

    //
    // Requests for next coarse level
    //

    int nextLevelID = coarseLevelID + 1;
    if (!isLastLevel) {
      if (nextLevelID > LastLevelID()) { AddNewLevel(); }
      CheckLevel(*Levels_[nextLevelID], nextLevelID);
      
      GetOStream(Debug, 0) << "Debug: Level: " << nextLevelID << " + R/S/C" << std::endl;
      Levels_[nextLevelID]->Request(TopRAPFactory(coarseManager, nextManager));
      Levels_[nextLevelID]->Request(TopSmootherFactory(nextManager, "Smoother")); // TODO: skip this line if we know that it is the lastLevel
      Levels_[nextLevelID]->Request(TopSmootherFactory(nextManager, "CoarseSolver"));
    }
    
    //
    // Build coarse level
    //
    
    SubMonitor m(*this, "Level " + Teuchos::toString(coarseLevelID));
    Level & level = *Levels_[coarseLevelID];

    // Build coarse level hierarchy
    if (!isFinestLevel) {
      TopRAPFactory coarseRAPFactory(fineManager, coarseManager);
      if (!level.IsAvailable("A")) { // build only if not available
        coarseRAPFactory.Build(*level.GetPreviousLevel(), level);
      }
      GetOStream(Debug, 0) << "Debug: Level: " << coarseLevelID << " - R" << std::endl;
      level.Release(coarseRAPFactory);
    }
     
    // Test if we reach the end of the hierarchy
    {
      RCP<Operator> Ac;
      if (level.IsAvailable("A")) {
        Ac = level.Get<RCP<Operator> >("A");
        // sumCoarseNnz += Ac->getGlobalNumEntries();
      } else {
        //TODO: happen when Ac factory = do nothing (ie: SetSmoothers)
      }
      
      if (Ac != Teuchos::null && Ac->getRowMap()->getGlobalNumElements() <= maxCoarseSize_) { // or if (coarseLevel == lastLevel
        if (isLastLevel == false) {
          GetOStream(Debug, 0) << "Debug: Level: " << nextLevelID << " - R/S/C" << std::endl;

          Levels_[nextLevelID]->Release(TopRAPFactory(coarseManager, nextManager));
          Levels_[nextLevelID]->Release(TopSmootherFactory(nextManager, "Smoother"));
          Levels_[nextLevelID]->Release(TopSmootherFactory(nextManager, "CoarseSolver"));
          Levels_.pop_back();
        }

        isLastLevel = true;
      }
    }

    // Build coarse level smoother
    TopSmootherFactory smootherFact      (coarseManager, "Smoother");
    TopSmootherFactory coarsestSolverFact(coarseManager, "CoarseSolver");
    
    if (!level.IsAvailable("PreSmoother") || !level.IsAvailable("PostSmoother")) { // build only if not available
      if (!isLastLevel) {
        smootherFact.Build(level);
      } else {
        coarsestSolverFact.Build(level); //TODO: PRE?POST
      }
    }
    
    GetOStream(Debug, 0) << "Debug: Level: " << coarseLevelID << " - S/C" << std::endl;
    level.Release(smootherFact);
    level.Release(coarsestSolverFact);

    return isLastLevel;

    //TODO: optimization: request should be posted only if data not yet available (reuse)
  }

  // new setup routine
  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  bool Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Setup(int coarseLevelID, const RCP<FactoryManager> & fineLevelManager, const RCP<FactoryManager> &coarseLevelManager,
             const RCP<FactoryManager> & nextLevelManager /*should be optional*/) {
    //TODO
    typedef MueLu::TopRAPFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> TopRAPFactory;
    typedef MueLu::TopSmootherFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> TopSmootherFactory;

    //
    // Init
    //

    //    int coarseLevelID = LastLevelID() - 1; // Level built by this function
    TEUCHOS_TEST_FOR_EXCEPTION(LastLevelID() < coarseLevelID, Exceptions::RuntimeError, "MueLu::Hierarchy:Setup(): level " << coarseLevelID << " (specified by coarseLevelID argument) must be build before calling this function.");
    CheckLevel(*Levels_[coarseLevelID], coarseLevelID);

    bool isLastLevel = false;
    bool isFinestLevel = false;
    if(fineLevelManager == Teuchos::null) isFinestLevel = true;
    if(nextLevelManager == Teuchos::null) isLastLevel = true;

    //
    // Requests for finest level
    //
    if (isFinestLevel) {
      Levels_[coarseLevelID]->Request(TopSmootherFactory(coarseLevelManager, "Smoother")); // TODO: skip this line if we know that it is the lastLevel
      Levels_[coarseLevelID]->Request(TopSmootherFactory(coarseLevelManager, "CoarseSolver"));
    }

    //
    // Requests for next coarse level
    //

    int nextLevelID = coarseLevelID + 1;
    if (!isLastLevel) {
      if (nextLevelID > LastLevelID()) { AddNewLevel(); }
      CheckLevel(*Levels_[nextLevelID], nextLevelID);

      GetOStream(Debug, 0) << "Debug: Level: " << nextLevelID << " + R/S/C" << std::endl;
      Levels_[nextLevelID]->Request(TopRAPFactory(coarseLevelManager, nextLevelManager));
      Levels_[nextLevelID]->Request(TopSmootherFactory(nextLevelManager, "Smoother")); // TODO: skip this line if we know that it is the lastLevel
      Levels_[nextLevelID]->Request(TopSmootherFactory(nextLevelManager, "CoarseSolver"));
    }

    //
    // Build coarse level
    //

    SubMonitor m(*this, "Level " + Teuchos::toString(coarseLevelID));
    Level & level = *Levels_[coarseLevelID];

    // Build coarse level hierarchy
    if (!isFinestLevel) {
      TopRAPFactory coarseRAPFactory(fineLevelManager, coarseLevelManager);
      coarseRAPFactory.Build(*level.GetPreviousLevel(), level);
      GetOStream(Debug, 0) << "Debug: Level: " << coarseLevelID << " - R" << std::endl;
      level.Release(coarseRAPFactory);
    }

    // Test if we reach the end of the hierarchy
    {
      RCP<Operator> Ac;
      if (level.IsAvailable("A")) {
        Ac = level.Get<RCP<Operator> >("A");
        // sumCoarseNnz += Ac->getGlobalNumEntries();
      } else {
        //TODO: happen when Ac factory = do nothing (ie: SetSmoothers)
      }

      if (Ac != Teuchos::null && Ac->getRowMap()->getGlobalNumElements() <= maxCoarseSize_) { // or if (coarseLevel == lastLevel
        if (isLastLevel == false) {
          GetOStream(Debug, 0) << "Debug: Level: " << nextLevelID << " - R/S/C" << std::endl;
          Levels_[nextLevelID]->Release(TopRAPFactory(coarseLevelManager, nextLevelManager));
          Levels_[nextLevelID]->Release(TopSmootherFactory(nextLevelManager, "Smoother"));
          Levels_[nextLevelID]->Release(TopSmootherFactory(nextLevelManager, "CoarseSolver"));
          Levels_.pop_back();
        }

        isLastLevel = true;
      }
    }

    // Build coarse level smoother
    TopSmootherFactory smootherFact (coarseLevelManager, "Smoother");
    TopSmootherFactory coarsestSolverFact(coarseLevelManager, "CoarseSolver");

    if (!isLastLevel) {
      smootherFact.Build(level);
    } else {
      coarsestSolverFact.Build(level); //TODO: PRE?POST
    }

    GetOStream(Debug, 0) << "Debug: Level: " << coarseLevelID << " - S/C" << std::endl;
    level.Release(smootherFact);
    level.Release(coarsestSolverFact);

    return isLastLevel;
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  Teuchos::ParameterList Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Setup(const FactoryManager & manager, const int &startLevel, const int &numDesiredLevels) {
    RCP<const FactoryManager> rcpManager = rcpFromRef(manager);

// // //     TopRAPFactory<SC, LO, GO, NO>      rapFactory           (rcpManager); //TODO: remove SC, LO, GO, NO
    TopSmootherFactory<SC, LO, GO, NO> smootherFactory      (rcpManager, "Smoother");
    TopSmootherFactory<SC, LO, GO, NO> coarsestSolverFactory(rcpManager, "CoarseSolver");

    Monitor h(*this, "Setup");
    //TODO Xpetra::global_size_t sumCoarseNnz = 0;

    TEUCHOS_TEST_FOR_EXCEPTION(numDesiredLevels < 2, Exceptions::RuntimeError, "MueLu::Hierarchy::Setup(): numDesiredLevels < 2");

    //TODO: check Levels_[startLevel] exists.

    // Check for fine level matrix A
    TEUCHOS_TEST_FOR_EXCEPTION(!Levels_[startLevel]->IsAvailable("A"), Exceptions::RuntimeError, "MueLu::Hierarchy::Setup(): no fine level matrix A! Set fine level matrix A using Level.Set()");

    // Check coarse levels
    // TODO: check if Ac available. If yes, issue a warning (bcse level already built...)
    GetOStream(Debug, 0) << "Debug: Level: " << startLevel << " + S/C" << std::endl;
    Levels_[startLevel]->Request(smootherFactory);
    Levels_[startLevel]->Request(coarsestSolverFactory);

    //
    const int lastLevel = startLevel + numDesiredLevels - 1;
    int iLevel;
    GetOStream(Runtime0, 0) << "Loop: startLevel=" << startLevel << ", lastLevel=" << lastLevel << " (stop if numLevels = " << numDesiredLevels << " or Ac.size() = " << maxCoarseSize_ << ")" << std::endl;

    for (iLevel = startLevel; iLevel <= lastLevel; iLevel++) {
      if(Setup(iLevel, manager, manager, manager, iLevel == startLevel, iLevel == lastLevel) == true) { 
        break;
      }
    }

    // Crop. TODO: add a warning
    Levels_.resize(iLevel + 1);

    // TODO: not exception safe: manager will still hold default factories if you exit this function with an exception
    manager.Clean();

    // Gather statistics
    //TODO    Xpetra::global_size_t fineNnz = Levels_[startLevel]->Get< RCP<Operator> >("A")->getGlobalNumEntries();
    //TODO    Xpetra::global_size_t totalNnz = fineNnz + sumCoarseNnz;

    Teuchos::ParameterList status;
    //TODO
    // status.set("fine nnz", fineNnz);
//     status.set("total nnz", totalNnz);
//     status.set("start level", startLevel);
//     status.set("end level", iLevel); //TODO: check if it's actually correct (exit by break vs. end of loop)
//     status.set("operator complexity", ((double)totalNnz) / fineNnz);

    return status;

  } // Setup()

#ifdef OLD_VERSION
  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  Teuchos::ParameterList Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Setup(const FactoryManager & manager, const int &startLevel, const int &numDesiredLevels) {
    RCP<const FactoryManager> rcpManager = rcpFromRef(manager);

    TopRAPFactory<SC, LO, GO, NO>      rapFactory           (rcpManager); //TODO: remove SC, LO, GO, NO
    TopSmootherFactory<SC, LO, GO, NO> smootherFactory      (rcpManager, "Smoother");
    TopSmootherFactory<SC, LO, GO, NO> coarsestSolverFactory(rcpManager, "CoarseSolver");

    Monitor h(*this, "Setup");
    Xpetra::global_size_t sumCoarseNnz = 0;

    TEUCHOS_TEST_FOR_EXCEPTION(numDesiredLevels < 2, Exceptions::RuntimeError, "MueLu::Hierarchy::Setup(): numDesiredLevels < 2");

    //TODO: check Levels_[startLevel] exists.

    // Check for fine level matrix A
    TEUCHOS_TEST_FOR_EXCEPTION(!Levels_[startLevel]->IsAvailable("A"), Exceptions::RuntimeError, "MueLu::Hierarchy::Setup(): no fine level matrix A! Set fine level matrix A using Level.Set()");

    // Check coarse levels
    // TODO: check if Ac available. If yes, issue a warning (bcse level already built...)
    GetOStream(Debug, 0) << "Debug: Level: " << startLevel << " + S/C" << std::endl;
    Levels_[startLevel]->Request(smootherFactory);
    Levels_[startLevel]->Request(coarsestSolverFactory);

    //
    const int lastLevel = startLevel + numDesiredLevels - 1;
    int iLevel;
    GetOStream(Runtime0, 0) << "Loop: startLevel=" << startLevel << ", lastLevel=" << lastLevel << " (stop if numLevels = " << numDesiredLevels << " or Ac.size() = " << maxCoarseSize_ << ")" << std::endl;

    for (iLevel = startLevel; iLevel <= lastLevel; iLevel++) {
      SubMonitor m(*this, "Level " + Teuchos::toString(iLevel));

      // Prepare next level (to keep useful data of current level!)
      int nextCoarseLevelID = iLevel + 1;
      if (nextCoarseLevelID <= lastLevel) {
        if (nextCoarseLevelID > LastLevelID()) { AddNewLevel(); }

        GetOStream(Debug, 0) << "Debug: Level: " << nextCoarseLevelID << " + R/S/C" << std::endl;
        Levels_[nextCoarseLevelID]->Request(rapFactory);
        Levels_[nextCoarseLevelID]->Request(smootherFactory); //TODO: skip if lastLevel
        Levels_[nextCoarseLevelID]->Request(coarsestSolverFactory);
      }

      Level & level = *Levels_[iLevel];
      TEUCHOS_TEST_FOR_EXCEPTION(level.GetLevelID() != iLevel, Exceptions::RuntimeError, "MueLu::Hierarchy::Setup(): wrong level ID");

      //
      if (iLevel != startLevel) {
        TEUCHOS_TEST_FOR_EXCEPTION(level.GetPreviousLevel() != Levels_[iLevel-1], Exceptions::RuntimeError, "MueLu::Hierarchy::Setup(): wrong level parent");

        rapFactory.Build(*level.GetPreviousLevel(), level);
        GetOStream(Debug, 0) << "Debug: Level: " << iLevel << " - R" << std::endl;
        level.Release(rapFactory);
      }
      //

      RCP<Operator> Ac;
      if (level.IsAvailable("A")) {
        Ac = level.Get<RCP<Operator> >("A");
        sumCoarseNnz += Ac->getGlobalNumEntries();
      } else {
        //TODO: happen when Ac factory = do nothing (ie: SetSmoothers)
      }

      if (iLevel == lastLevel || (Ac != Teuchos::null && Ac->getRowMap()->getGlobalNumElements() <= maxCoarseSize_)) {
        if (nextCoarseLevelID <= lastLevel) {
          GetOStream(Debug, 0) << "Debug: Level: " << nextCoarseLevelID << " - R/S/C" << std::endl;
          Levels_[nextCoarseLevelID]->Release(rapFactory);
          Levels_[nextCoarseLevelID]->Release(smootherFactory);
          Levels_[nextCoarseLevelID]->Release(coarsestSolverFactory);
        }

        coarsestSolverFactory.Build(level); //TODO: PRE?POST

        GetOStream(Debug, 0) << "Debug: Level: " << iLevel << " - S/C" << std::endl;
        level.Release(smootherFactory);
        level.Release(coarsestSolverFactory);

        break;

      } else {

        //GetOStream(Debug, 0) << "BUILD SMOO" << std::endl; GetOStream(Debug, 0).flush();
        smootherFactory.Build(level);
        GetOStream(Debug, 0) << "Debug: Level: " << iLevel << " - S/C" << std::endl;
        level.Release(smootherFactory);
        level.Release(coarsestSolverFactory);

      }

    } // for loop

      // Crop. TODO: add a warning
    Levels_.resize(iLevel + 1);

    // TODO: not exception safe: manager will still hold default factories if you exit this function with an exception
    manager.Clean();

    // Gather statistics
    Xpetra::global_size_t fineNnz = Levels_[startLevel]->Get< RCP<Operator> >("A")->getGlobalNumEntries();
    Xpetra::global_size_t totalNnz = fineNnz + sumCoarseNnz;

    Teuchos::ParameterList status;
    status.set("fine nnz", fineNnz);
    status.set("total nnz", totalNnz);
    status.set("start level", startLevel);
    status.set("end level", iLevel); //TODO: check if it's actually correct (exit by break vs. end of loop)
    status.set("operator complexity", ((double)totalNnz) / fineNnz);

    return status;

  } // Setup()
#endif // OLD

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::SetCoarsestSolver(SmootherFactoryBase const &smooFact, PreOrPost const &pop) {

    Level & level = *Levels_[LastLevelID()];
    RCP<FactoryManager> manager = rcp(new FactoryManager());
    manager->SetFactory("Smoother",     Teuchos::null); //? TODO remove
    manager->SetFactory("CoarseSolver", Teuchos::null);

    SetFactoryManager SFM(level, manager);

    level.Request("PreSmoother", &smooFact);
    level.Request("PostSmoother", &smooFact);

    smooFact.BuildSmoother(level, pop);

    if (level.IsAvailable("PreSmoother", &smooFact)) {
      RCP<SmootherBase> Pre  = level.Get<RCP<SmootherBase> >("PreSmoother", &smooFact);
      level.Set("PreSmoother", Pre);
      level.AddKeepFlag("PreSmoother", NoFactory::get(), MueLu::Final);
      level.RemoveKeepFlag("PreSmoother", NoFactory::get(), MueLu::UserData); // FIXME: This is a hack
    }

    if (level.IsAvailable("PostSmoother", &smooFact)) {
      RCP<SmootherBase> Post = level.Get<RCP<SmootherBase> >("PostSmoother", &smooFact);
      level.Set("PostSmoother", Post);
      level.AddKeepFlag("PostSmoother", NoFactory::get(), MueLu::Final);
      level.RemoveKeepFlag("PostSmoother", NoFactory::get(), MueLu::UserData); // FIXME: This is a hack
    }

    level.Release("PreSmoother", &smooFact);
    level.Release("PostSmoother", &smooFact);
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::SetSmoothers(SmootherFactory const & smooFact, LO const & startLevel, LO numDesiredLevels) {
    Monitor h(*this, "SetSmoothers");

    if (numDesiredLevels == -1)
      numDesiredLevels = GetNumLevels() - startLevel - 1;
    LO lastLevel = startLevel + numDesiredLevels - 1;

    //checks
    if (startLevel >= GetNumLevels())
      throw(Exceptions::RuntimeError("startLevel >= actual number of levels"));

    if (startLevel == GetNumLevels() - 1)
      throw(Exceptions::RuntimeError("startLevel == coarse level. Use SetCoarseSolver()"));

    if (lastLevel >= GetNumLevels() - 1) {
      lastLevel = GetNumLevels() - 2;
      GetOStream(Warnings0, 0) << "Warning: coarsest level solver will not be changed!" << std::endl;
    }

    FactoryManager manager;
    manager.SetFactory("Smoother", rcpFromRef(smooFact));
    manager.SetFactory("CoarseSolver", Teuchos::null);
    manager.SetFactory("P", Teuchos::null);
    manager.SetFactory("R", Teuchos::null);
    manager.SetFactory("A", Teuchos::null);

    lastLevel++; // hack: nothing will be done on the last level in Setup() because coarse solver of manager == Teuchos::null. TODO: print() of Setup() will be confusing
    numDesiredLevels = lastLevel - startLevel + 1;

    // GetOStream(Debug, 0) << "startLevel=" << startLevel << ", nummDesiredLevels=" << numDesiredLevels << std::endl;
    Setup(manager, startLevel, numDesiredLevels);

  } //SetSmoothers()

    // #define GimmeNorm(someVec, someLabel) { (someVec).norm2(norms); GetOStream(Statistics1, 0) << someLabel << " = " << norms << std::endl; }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Iterate(MultiVector const &B, LO nIts, MultiVector &X, //TODO: move parameter nIts and default value = 1
                                                                                  const bool &InitialGuessIsZero, const CycleType &Cycle, const LO &startLevel)
  {
    //Teuchos::Array<Magnitude> norms(1);
    bool zeroGuess=InitialGuessIsZero;

    for (LO i=0; i<nIts; i++) {

      RCP<Level> Fine = Levels_[startLevel];

      if (startLevel == 0 && IsPrint(Statistics1)) {
        GetOStream(Statistics1, 0) << "iter:    "
                                   << std::setiosflags(std::ios::left)
                                   << std::setprecision(3) << i
                                   << "           residual = "
                                   << std::setprecision(10) << Utils::ResidualNorm(*(Fine->Get< RCP<Operator> >("A")), X, B)
                                   << std::endl;
      }

      //X.norm2(norms);
      if (Fine->Get< RCP<Operator> >("A")->getDomainMap()->isCompatible(*(X.getMap())) == false) {
        std::ostringstream buf;
        buf << startLevel;
        std::string msg = "Level " + buf.str() + ": level A's domain map is not compatible with X";
        throw(Exceptions::Incompatible(msg));
      }

      if (Fine->Get< RCP<Operator> >("A")->getRangeMap()->isCompatible(*(B.getMap())) == false) {
        std::ostringstream buf;
        buf << startLevel;
        std::string msg = "Level " + buf.str() + ": level A's range map is not compatible with B";
        throw(Exceptions::Incompatible(msg));
      }

      //If on the coarse level, do either smoothing (if defined) or a direct solve.
      if (startLevel == ((LO)Levels_.size())-1) //FIXME is this right?
        {
          bool emptySolve = true;
          if (Fine->IsAvailable("PreSmoother")) { // important do use IsAvailable before Get here. Avoid building default smoother
            RCP<SmootherBase> preSmoo = Fine->Get< RCP<SmootherBase> >("PreSmoother");
            preSmoo->Apply(X, B, false);
            emptySolve=false;
          }
          if (Fine->IsAvailable("PostSmoother")) { // important do use IsAvailable before Get here. Avoid building default smoother
            RCP<SmootherBase> postSmoo = Fine->Get< RCP<SmootherBase> >("PostSmoother");
            postSmoo->Apply(X, B, false);
            emptySolve=false;
          }
          if (emptySolve==true)
            GetOStream(Warnings0, 0) << "Warning: No coarse grid solver" << std::endl;
        } else {
        //on an intermediate level
        RCP<Level> Coarse = Levels_[startLevel+1];

        //TODO: add IsAvailable test to avoid building default smoother
        if (Fine->IsAvailable("PreSmoother")) {
          RCP<SmootherBase> preSmoo = Fine->Get< RCP<SmootherBase> >("PreSmoother");
          preSmoo->Apply(X, B, zeroGuess);
        } else {
          GetOStream(Warnings0, 0) << "Warning: Level " <<  startLevel << ": No PreSmoother!" << std::endl;
        }

        RCP<MultiVector> residual = Utils::Residual(*(Fine->Get< RCP<Operator> >("A")), X, B);

        RCP<Operator> P = Coarse->Get< RCP<Operator> >("P");
        RCP<Operator> R;
        RCP<MultiVector> coarseRhs, coarseX;
        if (implicitTranspose_) {
          coarseRhs = MultiVectorFactory::Build(P->getDomainMap(), X.getNumVectors());
          coarseX   = MultiVectorFactory::Build(P->getDomainMap(), X.getNumVectors());
          P->apply(*residual, *coarseRhs, Teuchos::TRANS, 1.0, 0.0);
        } else {
          R = Coarse->Get< RCP<Operator> >("R");
          coarseRhs = MultiVectorFactory::Build(R->getRangeMap(), X.getNumVectors());
          coarseX   = MultiVectorFactory::Build(R->getRangeMap(), X.getNumVectors());
          R->apply(*residual, *coarseRhs, Teuchos::NO_TRANS, 1.0, 0.0);
        }
        coarseX->putScalar(0.);

        Iterate(*coarseRhs, 1, *coarseX, true, Cycle, startLevel+1);
        // ^^ zero initial guess
        if (Cycle>1)
          Iterate(*coarseRhs, 1, *coarseX, false, Cycle, startLevel+1);
        // ^^ nonzero initial guess

        // update X+=P * coarseX
        //P->apply(*coarseX, X, Teuchos::NO_TRANS, 1.0, 1.0);  //Xpetra throws an error if linAlgebra==0
        RCP<MultiVector> correction = MultiVectorFactory::Build(P->getRangeMap(), X.getNumVectors());
        P->apply(*coarseX, *correction, Teuchos::NO_TRANS, 1.0, 0.0);
        X.update(1.0, *correction, 1.0);

        //X.norm2(norms);
        //TODO: add IsAvailable test to avoid building default smoother
        if (Fine->IsAvailable("PostSmoother")) {
          RCP<SmootherBase> postSmoo = Fine->Get< RCP<SmootherBase> >("PostSmoother");
          postSmoo->Apply(X, B, false);
        } else {
          GetOStream(Warnings0, 0) << "Warning: Level " <<  startLevel << ": No PostSmoother!" << std::endl;
        }
      }
      zeroGuess=false;
    } //for (LO i=0; i<nIts; i++)

  } //Iterate()

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  std::string Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::description() const {
    std::ostringstream out;
    out << BaseClass::description();
    out << "{numLevels = " << GetNumLevels() << "}";
    return out.str();
  }
    
  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::print(Teuchos::FancyOStream &out, const VerbLevel verbLevel) const {
    MUELU_DESCRIBE;

    if (verbLevel & Parameters0) {
      out0 << "Number of Levels: " << GetNumLevels() << endl;
    }
      
    if (verbLevel & Parameters1) { 
      out0 << "Max Coarse Size: "    << maxCoarseSize_ << endl;
      out0 << "Implicit Transpose: " << implicitTranspose_ << endl;
    }
      
    if (verbLevel & Statistics1) {
      Teuchos::OSTab tab2(out);
      for(int i = 0; i < GetNumLevels(); i++) {
        Levels_[i]->print(out, verbLevel);
      }
    }
  }

  template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  Hierarchy<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Hierarchy(const Hierarchy &h) { }

} //namespace MueLu

// TODO: We need a Set/Get function to change the CycleType (for when Iterate() calls are embedded in a Belos Preconditionner for instance).

#endif // MUELU_HIERARCHY_DEF_HPP
