#! /usr/bin/env python

# @HEADER
# ************************************************************************
#
#              PyTrilinos.Epetra: Python Interface to Epetra
#                   Copyright (2005) Sandia Corporation
#
# Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
# license for use of this work by or on behalf of the U.S. Government.
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
# Questions? Contact Michael A. Heroux (maherou@sandia.gov)
#
# ************************************************************************
# @HEADER

# Imports.  Users importing an installed version of PyTrilinos should use the
# "from PyTrilinos import ..." syntax.  Here, the setpath module adds the build
# directory, including "PyTrilinos", to the front of the search path.  We thus
# use "import ..." for Trilinos modules.  This prevents us from accidentally
# picking up a system-installed version and ensures that we are testing the
# build module.
from   numpy    import *
from   optparse import *
import sys
import unittest

parser = OptionParser()
parser.add_option("-t", "--testharness", action="store_true",
                  dest="testharness", default=False,
                  help="test local build modules; prevent loading system-installed modules")
parser.add_option("-v", "--verbosity", type="int", dest="verbosity", default=2,
                  help="set the verbosity level [default 2]")
options,args = parser.parse_args()
if options.testharness:
    import setpath
    import Epetra
else:
    try:
        import setpath
        import Epetra
    except ImportError:
        from PyTrilinos import Epetra
        print >>sys.stderr, "Using system-installed Epetra"

##########################################################################

class EpetraIntVectorTestCase(unittest.TestCase):
    "TestCase class for IntVector objects"

    def setUp(self):
        self.length      = 9
        self.comm        = Epetra.PyComm()
        self.map         = Epetra.Map(self.length*self.comm.NumProc(),0,self.comm)
        self.numPyArray1 = arange(self.length)
        self.numPyArray2 = array([0,-1,2,-3,4,-5,6,-7,6])
        self.dtype       = zeros(1,'i').dtype

    def tearDown(self):
        self.comm.Barrier()

    def testConstructor01(self):
        "Test Epetra.IntVector (BlockMap,bool) constructor"
        eiv = Epetra.IntVector(self.map,True)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())

    def testConstructor02(self):
        "Test Epetra.IntVector (BlockMap) constructor"
        eiv = Epetra.IntVector(self.map)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())

    def testConstructor03(self):
        "Test Epetra.IntVector (BlockMap,bad-list) constructor"
        list = [0, 1.0, "e", "pi"]
        self.assertRaises(TypeError,Epetra.IntVector,self.map,list)

    def testConstructor04(self):
        "Test Epetra.IntVector (BlockMap,1D-small-list) constructor"
        list = [0, 1, 2, 3]
        eiv = Epetra.IntVector(self.map,list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())
        for i in range(len(list)):
            self.assertEquals(eiv[i], list[i])

    def testConstructor05(self):
        "Test Epetra.IntVector (BlockMap,1D-correct-list) constructor"
        list = [0, 1, 2, 3, 2, 1, 1, 0, 0]
        eiv = Epetra.IntVector(self.map,list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())
        for i in range(len(list)):
            self.assertEquals(eiv[i], list[i])

    def testConstructor06(self):
        "Test Epetra.IntVector (BlockMap,1D-big-list) constructor"
        list = [0, 1, 2, 3, 2, 1, 1, 0, 0, -1, -2]
        eiv = Epetra.IntVector(self.map,list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())
        for i in range(eiv.shape[0]):
            self.assertEquals(eiv[i], list[i])

    def testConstructor07(self):
        "Test Epetra.IntVector (BlockMap,2D-small-list) constructor"
        list = [[0, 1, 2],
                [2, 1, 1]]
        eiv = Epetra.IntVector(self.map,list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())
        for i in range(len(list)):
            for j in range(len(list[i])):
                self.assertEquals(eiv[i*3+j], list[i][j])

    def testConstructor08(self):
        "Test Epetra.IntVector (BlockMap,2D-big-list) constructor"
        list = [[0, 1, 2, 3,  2,  1],
                [1, 1, 0, 0, -1, -2]]
        eiv = Epetra.IntVector(self.map,list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())
        for i in range(len(list)):
            for j in range(len(list[i])):
                ii = i * 6 + j
                if (ii < eiv.MyLength()):
                    self.assertEquals(eiv[ii], list[i][j])

    def testConstructor09(self):
        "Test Epetra.IntVector (BlockMap,3D-small-list) constructor"
        list = [[[0, 1], [2, 3]],
                [[2, 1], [1, 0]]]
        eiv = Epetra.IntVector(self.map,list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())
        for i in range(len(list)):
            for j in range(len(list[i])):
                for k in range(len(list[i][j])):
                    self.assertEquals(eiv[4*i+2*j+k], list[i][j][k])

    def testConstructor10(self):
        "Test Epetra.IntVector (BlockMap,3D-correct-list) constructor"
        list = [[[0, 1, 2], [3, 2, 1], [1, 0, 0]]]
        eiv = Epetra.IntVector(self.map,list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())
        for i in range(len(list)):
            for j in range(len(list[i])):
                for k in range(len(list[i][j])):
                    self.assertEquals(eiv[i,j,k], list[i][j][k])

    def testConstructor11(self):
        "Test Epetra.IntVector (BlockMap,3D-big-list) constructor"
        list = [[[ 0,  1, 2], [3, 2, 1], [1, 0, 0]],
                [[-1,  0, 1], [2, 3, 2], [1, 1, 0]],
                [[ 0, -1, 0], [1, 2, 3], [2, 1, 1]]]
        eiv = Epetra.IntVector(self.map,list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), self.length)
        self.assertEquals(eiv.GlobalLength(), self.length*comm.NumProc())
        for i in range(len(list)):
            for j in range(len(list[i])):
                for k in range(len(list[i][j])):
                    ii = i*9 + j*3 + k
                    if (ii < eiv.MyLength()):
                        self.assertEquals(eiv[ii], list[i][j][k])

    def testConstructor12(self):
        "Test Epetra.IntVector (1D-list) constructor"
        list = [0, 1, 2, 3, 2, 1, 1, 0, 0]
        eiv = Epetra.IntVector(list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), len(list))
        self.assertEquals(eiv.GlobalLength(), len(list))
        for i in range(len(list)):
            self.assertEquals(eiv[i], list[i])

    def testConstructor13(self):
        "Test Epetra.IntVector (2D-list) constructor"
        list = [[0, 1, 2, 3, 2, 1, 1, 0, 0],
                [0, 0, 1, 1, 2, 3, 2, 1, 0]]
        eiv = Epetra.IntVector(list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), 2*len(list[0]))
        self.assertEquals(eiv.GlobalLength(), 2*len(list[0]))
        for i in range(eiv.shape[0]):
            for j in range(len(list[i])):
                self.assertEquals(eiv[i,j], list[i][j])

    def testConstructor14(self):
        "Test Epetra.IntVector (3D-list) constructor"
        list = [[[0, 1, 2], [3, 2, 1], [1, 0, 0]],
                [[0, 0, 1], [1, 2, 3], [2, 1, 0]],
                [[3, 2, 1], [0, 0, 0], [1, 1, 2]]]
        eiv = Epetra.IntVector(list)
        self.assertEquals(eiv.dtype,self.dtype)
        self.assertEquals(eiv.MyLength(), 3*3*len(list[0][0]))
        self.assertEquals(eiv.GlobalLength(), 3*3*len(list[0][0]))
        for i in range(eiv.shape[0]):
            for j in range(len(list[i])):
                self.failUnless((eiv[i,j] == list[i][j]).all())

    def testConstructor15(self):
        "Test Epetra.IntVector (bad-list) constructor"
        list = [0, 1.0, "e", "pi"]
        self.assertRaises(TypeError,Epetra.IntVector,list)

    def testConstructor16(self):
        "Test Epetra.IntVector copy constructor"
        eiv1 = Epetra.IntVector(self.map)
        self.assertEquals(eiv1.dtype,self.dtype)
        eiv2 = Epetra.IntVector(eiv1)
        self.assertEquals(eiv2.dtype,self.dtype)
        self.assertEquals(eiv2.MyLength(),     eiv1.MyLength()    )
        self.assertEquals(eiv2.GlobalLength(), eiv1.GlobalLength())
        for i in range(len(eiv1)):
            self.assertEquals(eiv1[i], eiv2[i])

    def testPutValue(self):
        "Test Epetra.IntVector PutValue method"
        eiv = Epetra.IntVector(self.map)
        self.assertEquals(eiv.dtype,self.dtype)
        for i in range(self.map.NumMyPoints()):
            self.assertEquals(eiv[i], 0)
        value = 789
        result = eiv.PutValue(value)
        self.assertEquals(result, 0)
        for i in range(self.map.NumMyPoints()):
            self.assertEquals(eiv[i], value)

    def testExtractCopy(self):
        "Test Epetra.IntVector ExtractCopy method"
        a     = [self.numPyArray1] * 4
        eiv   = Epetra.IntVector(self.map,a)
        array = eiv.ExtractCopy()
        self.assertEquals(type(array), ndarray)
        self.failUnless((eiv[:] == array[:]).all())
        self.assertEquals(eiv.array is array, False)

    def testExtractView(self):
        "Test Epetra.IntVector ExtractView method"
        a     = [self.numPyArray1] * 4
        eiv   = Epetra.IntVector(self.map,a)
        array = eiv.ExtractView()
        self.assertEquals(type(array), ndarray)
        for i in range(self.map.NumMyPoints()):
            self.assertEquals(eiv[i], array[i])
        # This will fail as long as we still use UserArray
        #self.assertEquals(eiv.array is array, True)

    def testPrint(self):
        "Test Epetra.IntVector Print method"
        output = ""
        if self.comm.MyPID() == 0:
            output += "%10s%14s%20s  \n" % ("MyPID","GID","Value")
        for lid in range(self.length):
            gid = self.map.GID(lid)
            output += "%10d%14d%24d\n" % (self.comm.MyPID(),gid,0)
        eiv = Epetra.IntVector(self.map)
        filename = "testIntVector%d.dat" % self.comm.MyPID()
        f = open(filename,"w")
        eiv.Print(f)
        f.close()
        self.assertEqual(open(filename,"r").read(), output)

    def testMinValue(self):
        "Test Epetra.IntVector MinValue method"
        a        = self.numPyArray1
        eiv      = Epetra.IntVector(self.map,a)
        result   = min(self.numPyArray1)
        minValue = eiv.MinValue()
        self.assertEquals(minValue, result)

    def testMaxValue(self):
        "Test Epetra.IntVector MaxValue method"
        a        = self.numPyArray2
        eiv      = Epetra.IntVector(self.map,a)
        result   = max(self.numPyArray2)
        maxValue = eiv.MaxValue()
        self.assertEquals(maxValue, result)

    def testSetArray(self):
        "Test Epetra.IntVector __setattr__ method for 'array'"
        eiv = Epetra.IntVector(self.map)
        self.assertRaises(AttributeError, eiv.__setattr__, "array", "junk")

    def testValues(self):
        "Test Epetra.IntVector Values method"
        a     = [self.numPyArray1] * 4
        eiv   = Epetra.IntVector(self.map,a)
        array = eiv.Values()
        self.assertEquals(type(array), ndarray)
        self.failUnless((eiv[:] == array[:]).all())
        # This will fail as long as we use UserArray
        #self.assertEquals(eiv.array is array, True)

##########################################################################

if __name__ == "__main__":

    # Create the test suite object
    suite = unittest.TestSuite()

    # Add the test cases to the test suite
    suite.addTest(unittest.makeSuite(EpetraIntVectorTestCase))

    # Create a communicator
    comm    = Epetra.PyComm()
    iAmRoot = comm.MyPID() == 0

    # Run the test suite
    if iAmRoot: print >>sys.stderr, \
       "\n************************\nTesting Epetra.IntVector\n************************\n"
    verbosity = options.verbosity * int(iAmRoot)
    result = unittest.TextTestRunner(verbosity=verbosity).run(suite)

    # Exit with a code that indicates the total number of errors and failures
    errsPlusFails = comm.SumAll(len(result.errors) + len(result.failures))
    if errsPlusFails == 0 and iAmRoot: print "End Result: TEST PASSED"
    sys.exit(errsPlusFails)
