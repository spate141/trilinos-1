#ifndef MUELU_VARIABLECONTAINER_HPP
#define MUELU_VARIABLECONTAINER_HPP

#include <map>
#include <Teuchos_ParameterEntry.hpp>

#include "MueLu_ConfigDefs.hpp"
#include "MueLu_BaseClass.hpp"

#include "MueLu_Exceptions.hpp"
#include "MueLu_FactoryBase_fwd.hpp"
#include "MueLu_KeepType.hpp"

namespace MueLu {

  /*!
    @class VariableContainer
    @brief Class that stores all relevant data for a variable

    Maintains all data for a variable, that is, the data itself, a boolean flag for the "Keep" status,
    a boolean flag for the "Available" status, a reference counter for all requests and a list with
    all requesting factories.
  */
  class VariableContainer : public BaseClass {
  public:
    //! @name Constructors/Destructors.
    //@{

    //! Default constructor.
    VariableContainer() :
      available_(false), keep_(false), count_(0)
    {};

    virtual ~VariableContainer(){};

    //@}

    //! @name Data access
    //@{

    //! Store data in container class and set the "Available" status true.
    void SetData(const Teuchos::ParameterEntry & entry) {
      data_ = entry;
      available_ = true;
    } //SetData

    //! return const reference to data stored in container
    //! note: we do not check if data is available
    const Teuchos::ParameterEntry & GetData() const {
      return data_;
    }

    //! return reference to data stored in container
    //! note: we do not check if data is available
    Teuchos::ParameterEntry & GetData() {
      return data_;
    }

    //! returns true if data is available, i.e. SetData has been called before
    bool IsAvailable() const {
      return available_;
    }

    //@}

    //! @name Request/Release
    //@{

    //! request data
    //! increment request counter and add reqFactory to the list
    //! of requesting factories
    void Request(const FactoryBase* reqFactory) {
      count_++;   // increment request counter
      if(requests_.count(reqFactory)==0) {
        requests_[reqFactory] = 1;
      } else
      {
        int cnt = requests_[reqFactory];
        requests_[reqFactory] = ++cnt;
      }
    }

    //! release data
    //! decrement request counter and try to remove reqFactory from list of
    //! requesting factories
    void Release(const FactoryBase* reqFactory) {
      if(requests_.count(reqFactory) > 0) {
        int cnt = requests_[reqFactory];
        requests_[reqFactory] = --cnt;
        if(cnt == 0 && GetKeepFlag() == 0) {
          requests_.erase(reqFactory);
        }
      }
      count_--; // decrement request counter
    }

    //! returns how often the data has been requested by the factory reqFactory.
    int NumRequests(const FactoryBase* reqFactory) const {
      if(requests_.count(reqFactory)>0) {
        return requests_.find(reqFactory)->second;
      }
      return 0;
    }

    //! returns how often the data has been requested by all factories
    int NumAllRequests() const {
      return count_;
    }

    //! returns true, if data is requested by reqFactory
    bool IsRequested(const FactoryBase* reqFactory) const {
      if (NumRequests(reqFactory) > 0) return true;
      return false;
    }

    //! returns true, if data is requested by at least one factory
    bool IsRequested() const {
      if (count_ > 0) return true;
      return false;
    }

    //@}

    //! @name Keep status
    //@{

    //! returns true if at least one keep flag is set
    bool IsKept(KeepType keep) const { return keep_ & keep; }

    //! add a keep flag to the flag combination
    void AddKeepFlag(KeepType keep = UserData) { keep_ = keep_ | keep; } // bitwise addition because flags can be set several times.

    //! remove a keep flag to the flag combination
    void RemoveKeepFlag(KeepType keep = UserData) { keep_ = keep_ & (keep_ ^ keep); } // xor between keep and keep_ and mask using & to avoid adding bits to keep_ if they were not set at the first place.

    //! returns the keep flag combination
    KeepType GetKeepFlag() const { return keep_; }

    //@}

  private:
    Teuchos::ParameterEntry           data_;        ///< the data itself
    bool                              available_;   ///< is data available?
    KeepType                          keep_;        ///< keep flag
    int                               count_;       ///< number of requests by all factories
    std::map<const FactoryBase*,int>  requests_;    ///< requesting factories
  };

}

#endif /* MUELU_VARIABLECONTAINER_HPP */

//TODO: move implementation to .cpp file + fwd decl of this class
