#ifndef __TSQR_Trilinos_TsqrTrilinosMessenger_hpp
#define __TSQR_Trilinos_TsqrTrilinosMessenger_hpp

#include "Teuchos_CommHelpers.hpp"
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR { 
  namespace Trilinos {
    /// \class TrilinosMessenger
    ///
    /// \param A thin wrapper around Teuchos::Comm, for use by TSQR
    ///
    /// The internode parallel part of TSQR communicates via a
    /// MessengerBase< Datum > interface.  TrilinosMessenger< Datum >
    /// implements that interface by wrapping Teuchos::Comm.
    ///
    /// \warning Datum should be a class with value-type semantics
    template< class Datum >
    class TrilinosMessenger : public MessengerBase< Datum > {
    public:
      typedef Teuchos::RCP< const Teuchos::Comm<int> > CommPtr;

      TrilinosMessenger (const CommPtr& pComm) : pComm_ (pComm) {}

      /// Send sendData[0:sendCount-1] to processor destProc.
      virtual void 
      send (const Datum sendData[], 
	    const int sendCount, 
	    const int destProc, 
	    const int tag) 
      {
	/// \note (mfh 14 June 2010) Teuchos generates "tag" arguments to
	/// MPI calls internally, so we ignore the tag here.  I don't use
	/// tags for anything in TSQR, so it doesn't matter.
	Teuchos::send (*pComm_, sendCount, sendData, destProc);
      }

      /// Receive recvData[0:recvCount-1] from processor srcProc.
      virtual void 
      recv (Datum recvData[], 
	    const int recvCount, 
	    const int srcProc, 
	    const int tag) 
      {
	/// \note (mfh 14 June 2010) Teuchos generates "tag" arguments to
	/// MPI calls internally, so we ignore the tag here.  I don't use
	/// tags for anything in TSQR, so it doesn't matter.
	Teuchos::receive (*pComm_, srcProc, recvCount, recvData);
      }

      /// Exchange sencRecvCount elements of sendData with processor
      /// destProc, receiving the result into recvData.
      virtual void 
      swapData (const Datum sendData[], 
		Datum recvData[], 
		const int sendRecvCount, 
		const int destProc, 
		const int tag)
      {
	if (destProc == rank())
	  std::copy (sendData, sendData+sendRecvCount, recvData);
	else
	  {
	    using Teuchos::RCP;
	    using Teuchos::ArrayRCP;
	    using Teuchos::CommRequest;

	    // FIXME (mfh 14 June 2010) Would be nice if Teuchos had a
	    // sendRecv() routine... as it is, we have to do a send and
	    // then a receive.  We do an isend and an ireceive for
	    // potential overlap.
	    ArrayRCP< const Datum > sendBuf (sendData, 0, sendRecvCount, false);
	    ArrayRCP< Datum > recvBuf (recvData, 0, sendRecvCount, false);

	    RCP< CommRequest > sendReq = Teuchos::isend (*pComm_, sendBuf, destProc);
	    RCP< CommRequest > recvReq = Teuchos::irecv (*pComm_, recvBuf, destProc);
	    // Wait on both the send and the receive to complete.  The
	    // two can happen independently, because sendBuf and recvBuf
	    // are different.  (We assert no aliasing of buffers here,
	    // and we've also checked above that destProc != rank().)
	    Teuchos::waitAll (*pComm_, Teuchos::tuple (sendReq, recvReq));
	  }
      }

      /// Allreduce sum all processors' inDatum data, returning the
      /// result (on all processors).
      virtual Datum 
      globalSum (const Datum& inDatum) 
      {
	Datum outDatum;
	Teuchos::reduceAll (*pComm_, Teuchos::REDUCE_SUM, inDatum, 
			    Teuchos::outArg(outDatum));
	return outDatum;
      }

      /// Allreduce sum all processors' inData[0:count-1], storing the
      /// results (on all processors) in outData.
      virtual void
      globalVectorSum (const Datum inData[], 
		       Datum outData[], 
		       const int count) 
      {
	Teuchos::reduceAll (*pComm_, Teuchos::REDUCE_SUM, count, 
			    inData, outData);
      }

      /// 
      /// Return this process' rank.
      /// 
      virtual int rank () const { return Teuchos::rank (*pComm_); }

      /// 
      /// Return the total number of ranks in the Teuchos::Comm communicator.
      /// 
      virtual int size () const { return Teuchos::size (*pComm_); }

      /// 
      /// Execute a barrier over the communicator.
      /// 
      virtual void barrier () const { Teuchos::barrier (*pComm_); }

    private:
      /// 
      /// Shared pointer to the the underlying Teuchos::Comm object.
      ///
      CommPtr pComm_;
    };
  } // namespace Trilinos
} // namespace TSQR

#endif // __TSQR_Trilinos_TsqrTrilinosMessenger_hpp
