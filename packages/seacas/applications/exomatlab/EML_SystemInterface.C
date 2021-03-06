/*
 * Copyright(C) 2011 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
 * certain rights in this software
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *           
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.
 *                         
 * * Neither the name of Sandia Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *                                                 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "EML_SystemInterface.h"

#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>

#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "EML_Version.h"
#include "SL_tokenize.h"

#include <Ioss_FileInfo.h>

#if defined(__PUMAGON__)
#define NPOS (size_t)-1
#else
#define NPOS std::string::npos
#endif

namespace {
  void parse_variable_names(const char *tokens, StringIdVector *variable_list);
}

SystemInterface::SystemInterface()
  : minimumTime_(0.0), maximumTime_(-1.0), inputFile_(), outputFile_(),
    listVars_(false), fieldSuffix_(0)
{
  enroll_options();
}

SystemInterface::~SystemInterface() {}

void SystemInterface::enroll_options()
{
  options_.usage("[options] input_database output_matlab_script_file\n"
		 "\t\tIf output name not specified, then output file will be the\n"
		 "\t\tbasename of the input file with suffix '.m'");

  options_.enroll("help", GetLongOption::NoValue,
		  "Print this summary and exit", 0);

  options_.enroll("version", GetLongOption::NoValue,
		  "Print version and exit", NULL);

  options_.enroll("field_suffix", GetLongOption::MandatoryValue,
		  "Character used to separate a field component suffix from the field name.\n"
		  "\t\tEnter '_' to treat field_x, field_y, field_z as a 3-component vector 'field'.\n"
		  "\t\tDefault = none (field_x, field_y, field_z are different fields)", "none");
  
  options_.enroll("minimum_time", GetLongOption::MandatoryValue,
		  "Minimum timestep for which to transfer data to matlab file.", 0);
  
  options_.enroll("maximum_time", GetLongOption::MandatoryValue,
		  "Maximum timestep for which to transfer data to matlab file.", 0);
  
  options_.enroll("list", GetLongOption::MandatoryValue,
		  "List global, nodal, element, nodeset, or sideset variables.\n\t\tEnter 'all' to list all types.\n"
		  "\t\tCode exits after listing variable names.", NULL);
  
  options_.enroll("gvar", GetLongOption::MandatoryValue,
		  "Comma-separated list of global variables to be output or ALL or NONE.",
		  "ALL");

  options_.enroll("evar", GetLongOption::MandatoryValue,
		  "(NI) Comma-separated list of element variables to be output or ALL or NONE.\n"
		  "\t\tVariables can be limited to certain blocks by appending a\n"
		  "\t\tcolon followed by the block id.  E.g. -evar sigxx:10:20",
		  0);

  options_.enroll("nvar", GetLongOption::MandatoryValue,
		  "(NI) Comma-separated list of nodal variables to be output or ALL or NONE.\n"
		  "\t\tVariables can be limited to certain nodes by appending a\n"
		  "\t\tcolon followed by the node id.  E.g. -nvar disp:10:20",
		  0);

  options_.enroll("nsetvar", GetLongOption::MandatoryValue,
		  "(NI) Comma-separated list of nodeset variables to be output or ALL or NONE.",
		  0);

  options_.enroll("ssetvar", GetLongOption::MandatoryValue,
		  "(NI) Comma-separated list of sideset variables to be output or ALL or NONE.",
		  0);

  options_.enroll("copyright", GetLongOption::NoValue,
		  "Show copyright and license data.",
		  NULL);
}

bool SystemInterface::parse_options(int argc, char **argv)
{
  int option_index = options_.parse(argc, argv);
  if ( option_index < 1 )
    return false;

  // Get options from environment variable also...
  char *options = getenv("exomatlab");
  if (options != NULL) {
    std::cerr << "\nThe following options were specified via the EXOMATLAB_OPTIONS environment variable:\n"
	      << "\t" << options << "\n\n";
    options_.parse(options, options_.basename(*argv));
  }

  if (options_.retrieve("help")) {
    options_.usage();
    std::cerr << "\n\tCan also set options via EXOMATLAB_OPTIONS environment variable.\n";
    std::cerr << "\n\t->->-> Send email to gdsjaar@sandia.gov for exomatlab support.<-<-<-\n";

    exit(EXIT_SUCCESS);
  }

  if (options_.retrieve("version")) {
    // Version is printed up front, just exit...
    exit(0);
  }
  
  {
    const char *temp = options_.retrieve("field_suffix");
    if (strcmp("none", temp) == 0) {
      // This is ASCII 1 which means it won't be found so
      // vector/tensor won't be recognized by default.
      fieldSuffix_ = 1; 
    } else {
      fieldSuffix_ = temp[0];
    }
  }

  {
    const char *temp = options_.retrieve("list");
    if (temp != NULL) {
      listVars_ = true;
      parse_variable_names(temp, &varsToList_);
    }
  }


  {
    const char *temp = options_.retrieve("gvar");
    parse_variable_names(temp, &globalVarNames_);
  }

  {
    const char *temp = options_.retrieve("nvar");
    parse_variable_names(temp, &nodeVarNames_);
  }

  {
    const char *temp = options_.retrieve("evar");
    parse_variable_names(temp, &elemVarNames_);
  }

  {
    const char *temp = options_.retrieve("nsetvar");
    parse_variable_names(temp, &nsetVarNames_);
  }

  {
    const char *temp = options_.retrieve("ssetvar");
    parse_variable_names(temp, &ssetVarNames_);
  }

  {
    const char *temp = options_.retrieve("minimum_time");
    if (temp != NULL)
      minimumTime_  = strtod(temp, NULL);
  }
  
  {
    const char *temp = options_.retrieve("maximum_time");
    if (temp != NULL)
      maximumTime_  = strtod(temp, NULL);
  }
  
  if (options_.retrieve("copyright")) {
    std::cerr << "\n"
	      << "Copyright(C) 2011 Sandia Corporation.  Under the terms of Contract\n"
	      << "DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains\n"
	      << "certain rights in this software\n"
	      << "\n"
	      << "Redistribution and use in source and binary forms, with or without\n"
	      << "modification, are permitted provided that the following conditions are\n"
	      << "met:\n"
	      << "\n"
	      << "* Redistributions of source code must retain the above copyright\n"
	      << "   notice, this list of conditions and the following disclaimer.\n"
	      << "          \n"
	      << "* Redistributions in binary form must reproduce the above\n"
	      << "  copyright notice, this list of conditions and the following\n"
	      << "  disclaimer in the documentation and/or other materials provided\n"
	      << "  with the distribution.\n"
	      << "                        \n"
	      << "* Neither the name of Sandia Corporation nor the names of its\n"
	      << "  contributors may be used to endorse or promote products derived\n"
	      << "  from this software without specific prior written permission.\n"
	      << "                                                \n"
	      << "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
	      << "\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
	      << "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
	      << "A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
	      << "OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
	      << "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n"
	      << "LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
	      << "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
	      << "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
	      << "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
	      << "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
	      << "\n";
    exit(EXIT_SUCCESS);
  }  
  
  // Parse remaining options as input file.
  if (option_index < argc) {
    inputFile_ = argv[option_index++];
    if (option_index < argc) {
      outputFile_ = argv[option_index++];
    } else {
      outputFile_ = Ioss::FileInfo(inputFile_).basename() + ".m";
    }
  } else {
    options_.usage();
    std::cerr << "\nERROR: no files specified\n\n";
    return false;
  }
  return true;
}

void SystemInterface::show_version()
{
  std::cout << qainfo[0] << "\n"
	    << "\t(A code for outputting exodusII global variable data for use in matlab.)\n"
	    << "\t(Version: " << qainfo[2] << ") Modified: " << qainfo[1] << '\n';
}

namespace {
  std::string LowerCase(const std::string &name)
  {
    std::string s = name;
    std::transform (s.begin(), s.end(),    // source
		    s.begin(),             // destination
		    ::tolower);            // operation
    return s;
  }

  typedef std::vector<std::string> StringVector;
  bool string_id_sort(const std::pair<std::string,int> &t1,
		      const std::pair<std::string,int> &t2)
  {
    return t1.first < t2.first || (!(t2.first < t1.first) &&
				   t1.second < t2.second);
  }

  void parse_variable_names(const char *tokens, StringIdVector *variable_list)
  {
    // Break into tokens separated by ","
    if (tokens != NULL) {
      std::string token_string(tokens);
      StringVector var_list;
      SLIB::tokenize(token_string, ",", var_list);
    
      // At this point, var_list is either a single string, or a
      // string separated from 1 or more ids with ":" delimiter.  For
      // example, sigxx:1:10:100 would indicate that the variable
      // "sigxx" should be written only for elements with id 1, 10,
      // and 100.  "sigxx" would indicate that the variable should be
      // written for all elements.
      std::vector<std::string>::iterator I = var_list.begin();
      while (I != var_list.end()) {
	StringVector name_id;
	SLIB::tokenize(*I, ":", name_id);
	std::string var_name = LowerCase(name_id[0]);
	if (name_id.size() == 1) {
	  (*variable_list).push_back(std::make_pair(var_name,0));
	} else {
	  for (size_t i=1; i < name_id.size(); i++) {
	    // Convert string to integer...
	    int id = strtoul(name_id[i].c_str(), NULL, 0);
	    (*variable_list).push_back(std::make_pair(var_name,id));
	  }
	}
	++I;
      }
      // Sort the list...
      std::sort(variable_list->begin(), variable_list->end(), string_id_sort);
    }
  }
}
