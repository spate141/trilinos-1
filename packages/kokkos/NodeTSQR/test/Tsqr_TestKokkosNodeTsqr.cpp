// @HEADER
// ***********************************************************************
//
//                 Anasazi: Block Eigensolvers Package
//                 Copyright (2010) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
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
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
// @HEADER

#define TRIVIAL_TEST 1

#include "Tsqr_Config.hpp"

#include "Teuchos_ConfigDefs.hpp" // HAVE_MPI
#include "Teuchos_Tuple.hpp"
#ifdef HAVE_MPI
#  include "Teuchos_GlobalMPISession.hpp"
#  include "Teuchos_oblackholestream.hpp"
#endif // HAVE_MPI
#include "Teuchos_CommandLineProcessor.hpp"
#include "Teuchos_DefaultComm.hpp"
#include "Teuchos_Time.hpp"

#ifdef TRIVIAL_TEST
#  include <Tsqr_KokkosNodeTsqr.hpp>
#  include <Tsqr_nodeTestProblem.hpp>
#  include <Tsqr_verifyTimerConcept.hpp>
#  include <Tsqr_Random_NormalGenerator.hpp>
#  include <Tsqr_LocalVerify.hpp>
#  include <Teuchos_Time.hpp>
#else
#  include <Tsqr_KokkosNodeTsqrTest.hpp>
#endif // TRIVIAL_TEST

#ifdef HAVE_TSQR_COMPLEX
#  include <complex>
#endif // HAVE_TSQR_COMPLEX

namespace {
  //
  // The documentation string for this test executable to print out at
  // the command line on request.
  //
  const char docString[] = "This program tests TSQR::KokkosNodeTsqr, "
    "which implements an intranode parallel version of TSQR for CPU-based "
    "Kokkos Node types.  Accuracy and performance tests are included.";

  //
  // TestParameters encapsulates values of command-line parameters, as
  // well as state that may change from one benchmark / verify
  // invocation to the next.
  //
  struct TestParameters {
    TestParameters () :
      verify (false),
      benchmark (false),
      numPartitions (1),
      numRows (1000),
      numCols (10),  
      numTrials (10),
      testReal (true),
#ifdef HAVE_TSQR_COMPLEX
      testComplex (false),
#endif // HAVE_TSQR_COMPLEX
      cacheSizeHint (0),
      contiguousCacheBlocks (false),
      printFieldNames (true),
      humanReadable (false),
      debug (false),
      seed (4)
    {
      seed[0] = 0;
      seed[1] = 0;
      seed[2] = 0;
      seed[3] = 1;
    }

    TestParameters (const std::vector<int> theSeed) :
      verify (false),
      benchmark (false),
      numPartitions (1),
      numRows (1000),
      numCols (10),  
      numTrials (10),
      testReal (true),
#ifdef HAVE_TSQR_COMPLEX
      testComplex (false),
#endif // HAVE_TSQR_COMPLEX
      cacheSizeHint (0),
      contiguousCacheBlocks (false),
      printFieldNames (true),
      humanReadable (false),
      debug (false),
      seed (theSeed)
    {}
    
    bool verify, benchmark;
    int numPartitions, numRows, numCols, numTrials;
    bool testReal;
#ifdef HAVE_TSQR_COMPLEX
    bool testComplex;
#endif // HAVE_TSQR_COMPLEX
    size_t cacheSizeHint;
    bool contiguousCacheBlocks, printFieldNames, humanReadable, debug;
    // Length-4 seed for the pseudorandom number generator.
    std::vector<int> seed;
  };


  //
  // Run the test(s) for a particular Scalar type T.
  // Used by Cons, which in turn is used by runTests().
  //
  template<class T>
  class Dispatcher {
  public:
    typedef T dispatch_type;

    static void execute (TestParameters& params) {
      if (params.benchmark)
	benchmark (params);
      if (params.verify)
	verify (params);
    }

  private:
    static void benchmark (TestParameters& params) {
      using Teuchos::TypeNameTraits;

#ifdef TRIVIAL_TEST
      std::cerr << "Benchmark stub for type " << TypeNameTraits<T>::name() << std::endl;
#else
      using TSQR::Test::benchmarkKokkosNodeTsqr;
      benchmarkKokkosNodeTsqr<int, T> (TypeNameTraits<T>::name(),
				       params.numTrials, 
				       params.numRows, 
				       params.numCols, 
				       params.numPartitions,
				       params.cacheSizeHint,
				       params.contiguousCacheBlocks,
				       params.printFieldNames,
				       params.humanReadable);
#endif // TRIVIAL_TEST
      params.printFieldNames = false;
    }

    static void verify (TestParameters& params) {
      using Teuchos::TypeNameTraits;

      TSQR::Random::NormalGenerator<int, T> gen (params.seed);
#ifdef TRIVIAL_TEST
      std::cerr << "Verify stub for type " << TypeNameTraits<T>::name() << std::endl;
#else
      using TSQR::Test::benchmarkKokkosNodeTsqr;
      verifyKokkosNodeTsqr<int, T> (TypeNameTraits<T>::name(),
				    gen,
				    params.numRows, 
				    params.numCols, 
				    params.numPartitions, 
				    params.cacheSizeHint,
				    params.contiguousCacheBlocks,
				    params.printFieldNames,
				    params.humanReadable,
				    params.debug);
#endif // TRIVIAL_TEST
      params.printFieldNames = false;
      // Save the seed for next time, since we can't use the same
      // NormalGenerator for a different Scalar type T.
      gen.getSeed (params.seed);
    }
  };

  //
  // Class for executing a template function over a compile-time
  // fixed-length list of types.  See runTests() for an example.
  //
  template<class CarType, class CdrType>
  class Cons {
  public:
    static void execute (TestParameters& params) {
      Dispatcher<CarType>::execute (params);
      CdrType::execute (params);
    }
  };

  //
  // Base case for Cons template recursion.
  //
  class NullCons {
  public:
    static void execute (TestParameters&) {}
  };

  // 
  // Run the tests for all types of interest.
  // This routine will modify TestParameters.
  //
  void
  runTests (TestParameters& params)
  {
    typedef Cons<float, Cons<double, NullCons> > real_tests;
#ifdef HAVE_TSQR_COMPLEX
    typedef Cons<std::complex<float>, Cons<std::complex<double>, NullCons> > complex_tests;
#endif // HAVE_TSQR_COMPLEX

    // This screams for syntactic sugar, but welcome to C++, the land of verbose obscurity.
    //typedef Cons<float, Cons<double, Cons<std::complex<float>, Cons<std::complex<double>, NullCons> > > > all_tests;

    const bool doVerify = params.verify;
    const bool doBenchmark = params.benchmark;
    const bool doPrintFieldNames = params.printFieldNames;

    // The Boolean trickery ensures that we do all verify tests at
    // once, and all benchmark tests at once, instead of interleaving
    // them.
    if (doBenchmark)
      {
	params.verify = false;
	if (params.testReal)
	  real_tests::execute (params);
#ifdef HAVE_TSQR_COMPLEX
	if (params.testComplex)
	  complex_tests::execute (params);
#endif // HAVE_TSQR_COMPLEX
	params.verify = doVerify;
      }
    if (doVerify)
      {
	params.benchmark = false;
	if (params.testReal)
	  real_tests::execute (params);
#ifdef HAVE_TSQR_COMPLEX
	if (params.testComplex)
	  complex_tests::execute (params);
#endif // HAVE_TSQR_COMPLEX
	params.benchmark = doBenchmark;
      }
    params.printFieldNames = doPrintFieldNames;
  }

  // Parse command-line options for this test.
  //
  // argc [in] As usual in C(++)
  //
  // argv [in] As usual in C(++)
  //
  // allowedToPrint [in] Whether this (MPI) process is allowed
  //   to print to stdout/stderr.  Different per (MPI) process.
  //
  // printedHelp [out] Whether this (MPI) process printed the
  //   "help" display (summary of command-line options).
  //
  // Return an encapsulation of the command-line options.
  TestParameters
  parseOptions (int argc, 
		char* argv[], 
		const bool allowedToPrint, 
		bool& printedHelp)
  {
    using std::cerr;
    using std::endl;

    printedHelp = false;

    // Command-line parameters, set to their default values.
    TestParameters params;
    /// We really want the cache size hint as a size_t, but
    /// Teuchos::CommandLineProcessor doesn't offer that option.  So
    /// we read it in as an int, which means negative inputs are
    /// possible.  We check for those below in the input validation
    /// phase.
    //
    // Fetch default value of cacheSizeHint.
    int cacheSizeHintAsInt = static_cast<int> (params.cacheSizeHint);
    try {
      using Teuchos::CommandLineProcessor;

      CommandLineProcessor cmdLineProc (/* throwExceptions=*/ true, 
					/* recognizeAllOptions=*/ true);
      cmdLineProc.setDocString (docString);
      cmdLineProc.setOption ("verify",
			     "noverify",
			     &params.verify,
			     "Test accuracy");
      cmdLineProc.setOption ("benchmark",
			     "nobenchmark",
			     &params.benchmark,
			     "Test performance");
      cmdLineProc.setOption ("numRows", 
			     &params.numRows, 
			     "Number of rows in the test matrix");
      cmdLineProc.setOption ("numCols", 
			     &params.numCols, 
			     "Number of columns in the test matrix");
      cmdLineProc.setOption ("numTrials", 
			     &params.numTrials, 
			     "Number of trials (only used when \"--benchmark\"");
      cmdLineProc.setOption ("testReal", 
			     "noTestReal",
			     &params.testReal,
			     "Test real arithmetic");
#ifdef HAVE_TSQR_COMPLEX
      cmdLineProc.setOption ("testComplex", 
			     "noTestComplex",
			     &params.testComplex,
			     "Test complex arithmetic");
#endif // HAVE_TSQR_COMPLEX
      cmdLineProc.setOption ("numPartitions", 
			     &params.numPartitions,
			     "Number of partitions to use (max available parallelism)");
      cmdLineProc.setOption ("cacheSizeHint", 
			     &cacheSizeHintAsInt, 
			     "Cache size hint in bytes (0 means pick a reasonable default)");
      cmdLineProc.setOption ("contiguousCacheBlocks",
			     "noncontiguousCacheBlocks",
			     &params.contiguousCacheBlocks,
			     "Whether cache blocks should be stored contiguously");
      cmdLineProc.setOption ("printFieldNames",
			     "noPrintFieldNames",
			     &params.printFieldNames,
			     "Print field names (for machine-readable output only)");
      cmdLineProc.setOption ("humanReadable",
			     "machineReadable",
			     &params.humanReadable,
			     "If set, make output easy to read by humans "
			     "(but hard to parse)");
      cmdLineProc.setOption ("debug", 
			     "noDebug", 
			     &params.debug, 
			     "Print debugging information");
      cmdLineProc.parse (argc, argv);
    } 
    catch (Teuchos::CommandLineProcessor::UnrecognizedOption& e) { 
      if (allowedToPrint)
	cerr << "Unrecognized command-line option: " << e.what() << endl;
      throw e;
    }
    catch (Teuchos::CommandLineProcessor::HelpPrinted& e) { 
      printedHelp = true; 
      return params; // Don't verify parameters in this case
    } 

    // Validate command-line options.  We provide default values
    // for unset options, so we don't have to validate those.
    if (params.numRows <= 0)
      throw std::invalid_argument ("Number of rows must be positive");
    else if (params.numCols <= 0)
      throw std::invalid_argument ("Number of columns must be positive");
    else if (params.numRows < params.numCols)
      throw std::invalid_argument ("Number of rows must be >= number of columns");
    else if (params.benchmark && params.numTrials < 1)
      throw std::invalid_argument ("\"--benchmark\" option requires numTrials >= 1");
    else if (params.numPartitions < 1)
      throw std::invalid_argument ("\"--numPartitions\" option must be >= 1");
    else
      {
	if (cacheSizeHintAsInt < 0)
	  throw std::invalid_argument ("Cache size hint must be nonnegative");
	else 
	  params.cacheSizeHint = static_cast<size_t> (cacheSizeHintAsInt);
      }
    return params;
  }
} // namespace (anonymous)


//
// The "main" test driver.
//
int 
main (int argc, char *argv[]) 
{
  using Teuchos::RCP;

#ifdef HAVE_MPI
  typedef RCP<const Teuchos::Comm<int> > comm_ptr;

  Teuchos::oblackholestream blackhole;
  Teuchos::GlobalMPISession mpiSession (&argc, &argv, &blackhole);
  comm_ptr comm = Teuchos::DefaultComm<int>::getComm();
  const int myRank = comm->getRank();
  // Only Rank 0 gets to write to stdout.  The other MPI process ranks
  // send their output to something that looks like /dev/null (and
  // likely is, on Unix-y operating systems).
  std::ostream& out = (myRank == 0) ? std::cout : blackhole;
  // Only Rank 0 performs the tests.
  const bool performingTests = (myRank == 0);
  const bool allowedToPrint = (myRank == 0);

#else // Don't HAVE_MPI: single-node test

  const bool performingTests = true;
  const bool allowedToPrint = true;
  std::ostream& out = std::cout;
#endif // HAVE_MPI

  // Fetch command-line parameters.
  bool printedHelp = false;
  TestParameters params = 
    parseOptions (argc, argv, allowedToPrint, printedHelp);
  if (printedHelp)
    return 0;

  if (performingTests)
    {
      using std::endl;

      // We allow the same run to do both benchmark and verify.
      runTests (params);

      // The Trilinos test framework expects a message like this.
      // Obviously we haven't tested anything, but eventually we
      // will include accuracy integration tests.
      out << "\nEnd Result: TEST PASSED" << endl;
    }

  return 0;
}


