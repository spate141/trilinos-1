# Microsoft Developer Studio Project File - Name="mesquite" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=mesquite - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mesquite.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mesquite.mak" CFG="mesquite - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mesquite - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "mesquite - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
F90=df.exe
RSC=rc.exe

!IF  "$(CFG)" == "mesquite - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE F90 /compile_only /nologo /warn:nofileopt
# ADD F90 /compile_only /nologo /warn:nofileopt
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../TSTT-interface" /I "../include" /I "../src" /I "../src/Control" /I "../src/Control/Wrappers" /I "../src/Mesh" /I "../src/Misc" /I "../src/ObjectiveFunction" /I "../src/QualityAssessor" /I "../src/QualityImprover" /I "../src/QualityImprover/TopologyModifier" /I "../src/QualityImprover/VertexMover" /I "../src/QualityImprover/VertexMover/ConjugateGradient" /I "../src/QualityImprover/VertexMover/LaplacianSmoothers" /I "../src/QualityImprover/VertexMover/NonSmoothSteepestDescent" /I "../src/QualityImprover/VertexMover/Randomize" /I "../src/QualityImprover/VertexMover/SteepestDescent" /I "../src/QualityMetric" /I "../src/QualityMetric/Shape" /I "../src/QualityMetric/Smoothness" /I "../src/QualityMetric/Untangle" /I "../src/QualityMetric/Volume" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USE_STD_INCLUDES" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "mesquite - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE F90 /check:bounds /compile_only /debug:full /nologo /traceback /warn:argument_checking /warn:nofileopt
# ADD F90 /check:bounds /compile_only /debug:full /nologo /traceback /warn:argument_checking /warn:nofileopt
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../TSTT-interface" /I "../include" /I "../src" /I "../src/Control" /I "../src/Control/Wrappers" /I "../src/Mesh" /I "../src/Misc" /I "../src/ObjectiveFunction" /I "../src/QualityAssessor" /I "../src/QualityImprover" /I "../src/QualityImprover/TopologyModifier" /I "../src/QualityImprover/VertexMover" /I "../src/QualityImprover/VertexMover/ConjugateGradient" /I "../src/QualityImprover/VertexMover/LaplacianSmoothers" /I "../src/QualityImprover/VertexMover/NonSmoothSteepestDescent" /I "../src/QualityImprover/VertexMover/Randomize" /I "../src/QualityImprover/VertexMover/SteepestDescent" /I "../src/QualityMetric" /I "../src/QualityMetric/Shape" /I "../src/QualityMetric/Smoothness" /I "../src/QualityMetric/Untangle" /I "../src/QualityMetric/Volume" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USE_STD_INCLUDES" /YX /FD /GZ                                                             /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "mesquite - Win32 Release"
# Name "mesquite - Win32 Debug"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\AspectRatioGammaQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\CompositeOFAdd.hpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\CompositeOFMultiply.hpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\CompositeOFScalarAdd.hpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\CompositeOFScalarMultiply.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\CompositeQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\ConditionNumberQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\ConjugateGradient\ConjugateGradient.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\GeneralizedConditionNumberQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Control\InstructionQueue.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Control\Wrappers\InstructionWrapper.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\InverseMeanRatioQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Control\Wrappers\LaplacianIQ.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Smoothness\LaplacianQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\LaplacianSmoothers\LaplacianSmoother.hpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\LInfTemplate.hpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\LPTemplate.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\MeanRatioQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Mesh\MeshSet.hpp
# End Source File
# Begin Source File

SOURCE=..\include\Mesquite.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Misc\MesquiteError.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Misc\MesquiteUtilities.hpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\MinTemplate.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Mesh\MsqFreeVertexIndexIterator.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Mesh\MsqMeshEntity.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Misc\MsqMessage.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Misc\MsqTimer.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Mesh\MsqVertex.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\MultiplyQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\NonSmoothSteepestDescent\NonSmoothSteepestDescent.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\NullImprover.hpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\ObjectiveFunction.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Misc\ParameterSet.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Mesh\PatchData.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Mesh\PatchDataUser.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityAssessor\QualityAssessor.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\QualityImprover.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\QualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\Randomize\Randomize.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Control\Wrappers\ShapeImprover.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\ShapeQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\LaplacianSmoothers\SmartLaplacianSmoother.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Smoothness\SmoothnessQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\SteepestDescent\SteepestDescent.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Control\StoppingCriterion.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\TopologyModifier\TopologyModifier.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Untangle\UntangleBetaQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Untangle\UntangleQualityMetric.hpp
# End Source File
# Begin Source File

SOURCE=..\src\Misc\Vector3D.hpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\VertexMover.hpp
# End Source File
# End Group
# Begin Group "src"

# PROP Default_Filter ""
# Begin Group "Control"

# PROP Default_Filter ""
# Begin Group "Wrappers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\Control\Wrappers\ShapeImprover.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\Control\InstructionQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Control\StoppingCriterion.cpp
# End Source File
# End Group
# Begin Group "Mesh"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\Mesh\MeshSet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Mesh\MsqMeshEntity.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Mesh\MsqVertex.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Mesh\PatchData.cpp
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\Misc\MesquiteError.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Misc\MesquiteUtilities.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Misc\MsqMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Misc\MsqTimer.cpp
# ADD CPP /D "USE_CLOCK_TIMER"
# End Source File
# End Group
# Begin Group "QualityImprover"

# PROP Default_Filter ""
# Begin Group "TopologyModifier"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\QualityImprover\TopologyModifier\TopologyModifier.cpp
# End Source File
# End Group
# Begin Group "VertexMover"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\ConjugateGradient\ConjugateGradient.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\LaplacianSmoothers\LaplacianSmoother.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\NonSmoothSteepestDescent\NonSmoothSteepestDescent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\Randomize\Randomize.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\LaplacianSmoothers\SmartLaplacianSmoother.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\SteepestDescent\SteepestDescent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityImprover\VertexMover\VertexMover.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\QualityImprover\QualityImprover.cpp
# End Source File
# End Group
# Begin Group "QualityAssessor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\QualityAssessor\QualityAssessor.cpp
# End Source File
# End Group
# Begin Group "ObjectiveFunction"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\ObjectiveFunction\CompositeOFAdd.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\CompositeOFMultiply.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\CompositeOFScalarAdd.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\CompositeOFScalarMultiply.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\LInfTemplate.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\LPTemplate.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\MinTemplate.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ObjectiveFunction\ObjectiveFunction.cpp
# End Source File
# End Group
# Begin Group "QualityMetric"

# PROP Default_Filter ""
# Begin Group "Shape"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\AspectRatioGammaQualityMetric.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\ConditionNumberQualityMetric.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\GeneralizedConditionNumberQualityMetric.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\InverseMeanRatioQualityMetric.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\Shape\MeanRatioQualityMetric.cpp
# End Source File
# End Group
# Begin Group "Smoothness"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\QualityMetric\Smoothness\LaplacianQualityMetric.cpp
# End Source File
# End Group
# Begin Group "Untangle"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\QualityMetric\Untangle\UntangleBetaQualityMetric.cpp
# End Source File
# End Group
# Begin Group "Volume"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=..\src\QualityMetric\MultiplyQualityMetric.cpp
# End Source File
# Begin Source File

SOURCE=..\src\QualityMetric\QualityMetric.cpp
# End Source File
# End Group
# End Group
# End Target
# End Project
