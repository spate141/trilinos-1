# @HEADER
# ************************************************************************
# 
#                     Pamgen Package
#                 Copyright (2004) Sandia Corporation
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

## #######################################################################
## Options to automake (rarely used - don't worry about it)
## #######################################################################
AUTOMAKE_OPTIONS = foreign

## #######################################################################
## Aclocal command (rarely used - don't worry about it)
## #######################################################################
ACLOCAL_AMFLAGS = -I config

#
#       I believe that by switching to AUX_DIR(../../config) one 
#       could get rid of these.
#
## #######################################################################
## Additional files to be included in distribution for 'make dist'
## #######################################################################
#np# For a typical package, there is no reason to distribute these files
#np# because users should not have to bootstrap.  We distribute them with
#np# new package so that the files can be used in creating the 
#np# configure script for other packages. 
EXTRA_DIST = \
config/strip_dup_incl_paths.pl          config/strip_dup_libs.pl \
config/replace-install-prefix.pl        config/generate-makeoptions.pl \
config/token-replace.pl
#EXTRA_DIST = \
#config/depcomp \
#config/ac_cxx_have_std.m4                 config/install-sh \
#config/missing 				  config/mkinstalldirs \
#config/tac_arg_check_mpi.m4 		  config/tac_arg_config_mpi.m4 \
#config/ac_cxx_reinterpret_cast.m4         config/generate-makeoptions.pl \
#config/replace-install-prefix.pl          config/tac_arg_enable_can_use_package.m4 \
#config/tac_arg_enable_compiler_options.m4 config/tac_arg_enable_export-makefiles.m4 \
#config/tac_arg_enable_feature.m4  	  config/tac_arg_enable_option.m4 \
#config/ac_prompt_user.m4                  config/tac_arg_with_ar.m4 \
#config/ac_prompt_user_no_define.m4        config/tac_arg_with_compiler.m4 \
#config/acx_check_dos_filesys.m4           config/tac_arg_with_flags.m4 \
#config/tac_arg_with_incdirs.m4            config/acx_pthread.m4 \
#config/config.guess                       config/tac_arg_with_libdirs.m4 \
#config/config.sub                         config/tac_arg_with_package.m4 \
#config/strip_dup_incl_paths.pl            config/strip_dup_libs.pl \
#config/tac_arg_with_perl.m4 		  config/string-replace.pl \
#config/token-replace.pl \
#bootstrap 
#
#       I believe that by switching to AUX_DIR(../../config) one 
#       could get rid of these.
#
## #######################################################################
## Tools in the auxillary directory 
## #######################################################################
AUX_DIST = config/install-sh config/missing config/mkinstalldirs 
#
#  Again, I hope that AUX_DIR(../../config) eliminates these
#  config/install-sh config/missing config/mkinstalldirs 

## #######################################################################
## Files to be deleted by 'make maintainer-clean'
## #######################################################################
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 autom4te.cache/* \
	configure config.status config.log \
	src/common/config-h.in src/common/stamp-h.in \
	$(AUX_DIST) 

#The following line helps the test harness recover from build errors.
                                                                                
all-local:
	@echo "Trilinos package pamgen built successfully"

## #######################################################################
## Export Makefile Installation
## #######################################################################
if USING_EXPORT_MAKEFILES

install-exec-hook:
	mkdir -p $(DESTDIR)$(includedir)
	cp $(top_builddir)/Makefile.export.pamgen $(DESTDIR)$(includedir)/.
	$(PERL_EXE) $(top_srcdir)/config/replace-install-prefix.pl \
		--exec-prefix=$(exec_prefix) \
		--my-export-makefile=Makefile.export.pamgen \
		--my-abs-top-srcdir=@abs_top_srcdir@ \
		--my-abs-incl-dirs=@abs_top_builddir@/src:@abs_top_srcdir@/src \
		--my-abs-lib-dirs=@abs_top_builddir@/src 
	$(PERL_EXE) $(top_srcdir)/config/generate-makeoptions.pl $(top_builddir)/src/Makefile \
		PAMGEN > $(DESTDIR)$(includedir)/Makefile.export.pamgen.macros

uninstall-hook:
	rm -f $(includedir)/Makefile.export.pamgen
	rm -f $(includedir)/Makefile.export.pamgen.macros

else

install-exec-hook:

uninstall-hook:

endif

## #######################################################################
## Subdirectories to be make'd recursively
## #######################################################################
#  #np# - Most packages should have examples and a full suite of tests. 
#We now build tests and examples through separate make targets, rather than
#during "make".  We still need to conditionally include the test and example
#in SUBDIRS, even though BUILD_TESTS and BUILD_EXAMPLES will never be
#defined, so that the tests and examples are included in the distribution
#tarball. 

if SUB_TEST
TEST_SUBDIR=test
endif

if SUB_EXAMPLE
EXAMPLE_SUBDIR=example
endif

#  #np# - The following make targets must be defined for all packages.
#  #np# - If the package does not have tests or examples, replace the
#  #np# - corresponding rules with something like:
#  #np# - @echo "pamgen does not have any tests yet"
if BUILD_TESTS
tests:
	@echo ""
	@echo "Now building pamgen tests."
	@echo ""
	cd $(top_builddir)/test && $(MAKE)
	@echo ""
	@echo "Finished building pamgen tests."
	@echo ""
else
tests:
	@echo "pamgen tests were disabled at configure time"
endif

if BUILD_EXAMPLES
examples:
	@echo ""
	@echo "Now building pamgen examples."
	@echo ""
	cd $(top_builddir)/example && $(MAKE)
	@echo ""
	@echo "Finished building pamgen examples."
	@echo ""

install-examples:
	cd $(top_builddir)/example && $(MAKE) install
else
examples:
	@echo "pamgen examples were disabled at configure time"

install-examples:
	@echo "pamgen examples were disabled at configure time"
endif

clean-tests:
	cd $(top_builddir)/test && $(MAKE) clean

clean-examples:
	cd $(top_builddir)/example && $(MAKE) clean

everything:
	$(MAKE) && $(MAKE) examples && $(MAKE) tests

clean-everything:
	$(MAKE) clean-examples && $(MAKE) clean-tests && $(MAKE) clean

install-everything:
	$(MAKE) install && $(MAKE) install-examples

SUBDIRS = src $(TEST_SUBDIR) $(EXAMPLE_SUBDIR)

## #######################################################################
## The below targets allow you to use the new
## testharness to run the test suite as make targets
## #######################################################################

TRILINOS_HOME_DIR=@abs_top_srcdir@/../..
TRILINOS_BUILD_DIR=@abs_top_builddir@/../..
TRILINOS_TEST_CATEGORY=INSTALL

runtests-serial :
	$(PERL_EXE) $(TRILINOS_HOME_DIR)/commonTools/test/utilities/runtests \
  --trilinos-dir=$(TRILINOS_HOME_DIR) \
  --comm=serial \
  --build-dir=$(TRILINOS_BUILD_DIR) \
  --category=$(TRILINOS_TEST_CATEGORY) \
  --output-dir=@abs_top_builddir@/test/runtests-results \
  --verbosity=1 \
  --packages=pamgen

runtests-mpi :
	$(PERL_EXE) $(TRILINOS_HOME_DIR)/commonTools/test/utilities/runtests \
  --trilinos-dir=$(TRILINOS_HOME_DIR) \
  --comm=mpi \
  --mpi-go=$(TRILINOS_MPI_GO) \
  --build-dir=$(TRILINOS_BUILD_DIR) \
  --category=$(TRILINOS_TEST_CATEGORY) \
  --output-dir=@abs_top_builddir@/test/runtests-results \
  --verbosity=1 \
  --packages=pamgen

if HAVE_MPI
PAMGEN_CHECK_COMM=mpi
else
PAMGEN_CHECK_COMM=serial
endif

# run-examples:
# 	@echo
# 	@echo "==================================="
# 	@echo "Running the Pamgen examples..."
# 	@echo "==================================="
# 	@echo
# 	@echo "For more details, please refer to Trilinos/doc/check_installation.txt"
# 	@echo
# 	@$(PERL_EXE) $(TRILINOS_HOME_DIR)/commonTools/test/utilities/runtests \
# 	--trilinos-dir=$(TRILINOS_HOME_DIR) \
# 	--comm=$(PAMGEN_CHECK_COMM) \
# 	--mpi-go="$(TRILINOS_MPI_GO)" \
# 	--build-dir=$(TRILINOS_BUILD_DIR) \
# 	--category=PamgenExamples \
# 	--output-dir=@abs_top_builddir@/test/runtests-results \
# 	--verbosity=1 \
# 	--packages=pamgen

# run-tests:
# 	@echo
# 	@echo "================================"
# 	@echo "Running the Pamgen tests..."
# 	@echo "================================"
# 	@echo
# 	@echo "For more details, please refer to Trilinos/doc/check_installation.txt"
# 	@echo
# 	@$(PERL_EXE) $(TRILINOS_HOME_DIR)/commonTools/test/utilities/runtests \
# 	--trilinos-dir=$(TRILINOS_HOME_DIR) \
# 	--comm=$(PAMGEN_CHECK_COMM) \
# 	--mpi-go="$(TRILINOS_MPI_GO)" \
# 	--build-dir=$(TRILINOS_BUILD_DIR) \
# 	--category=FRAMEWORK \
# 	--output-dir=@abs_top_builddir@/test/runtests-results \
# 	--verbosity=1 \
# 	--packages=pamgen
