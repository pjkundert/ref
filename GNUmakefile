# 
# REF		-- Reference Counting Pointer Framework
# 

VERSION		= 4.1.0
ARCH		= $(shell uname -m | sed -e s/i.86/i386/ -e s/9000.*/hppa/)
#DEBUG		= yes 		# anything turns on -g
#OPT		= 3		# defaults to 3
TARGET		= /usr/local/include
TIME		= /usr/bin/time -p

# We want to deduce these, unless they are explicitly set...
ifeq ($(origin CC),default)
    CC		=
endif
ifeq ($(origin CXX),default)
    CXX		=
endif

CXXFLAGS	+= -I. -I./smarttest

# Default flags for building test executables.  Not required by
# default (only ref.H is required by client programs)!  Figure out
# deguggable vs. optimized, limit optimization level to practical limit
ifeq ($(ARCH),i386)
    ifneq ($(DEBUG),)
	CXXFLAGS 	+= -g
    endif

    ifneq ($(OPT),)
	CXXFLAGS	+= -O$(OPT)
    else
	CXXFLAGS	+= -O3
    endif

    ifeq ($(CXX),icc)
        CXXFLAGS += -Wall
    else
        ifeq ($(CXX),)
            CXX	 = g++
        endif
        CXXFLAGS += -pipe 				\
		    -W -Wall -Wwrite-strings		 \
		    -Wcast-align -Wpointer-arith	  \
		    -Wconversion -Wcast-qual -Wfloat-equal \
		    -Wnon-virtual-dtor -Woverloaded-virtual \
		    -Wsign-promo -Wshadow
    endif
endif
ifeq ($(ARCH),sun4v)
    ifeq ($(CXX),)
        CXX	 = CC
    endif
    ifneq ($(DEBUG),)
	CXXFLAGS	+= -g
    endif
    ifeq ($(CXX),CC)
        CXXFLAGS 	+= +w
        ifneq ($(OPT),)
	    CXXFLAGS	+= -xO$(OPT)
        else
	    CXXFLAGS	+= -xO4
        endif
    else
	# Assume something like g++
        ifneq ($(OPT),)
	    CXXFLAGS	+= -O$(OPT)
        else
	    CXXFLAGS	+= -O3
        endif
    endif
endif
ifeq ($(ARCH),x86_64)
    ifneq ($(DEBUG),)
	CXXFLAGS += -g
    endif

    ifneq ($(OPT),)
	CXXFLAGS	+= -O$(OPT)
    else
	CXXFLAGS	+= -O3
    endif

    ifeq ($(CXX),icc)
        CXXFLAGS += -Wall
    else
        ifeq ($(CXX),)
            CXX	 = g++
        endif
        CXXFLAGS += -pipe 				\
		    -W -Wall -Wwrite-strings		 \
		    -Wcast-align -Wpointer-arith	  \
		    -Wconversion -Wcast-qual -Wfloat-equal \
		    -Wnon-virtual-dtor -Woverloaded-virtual \
		    -Wsign-promo -Wshadow
    endif
endif
ifeq ($(ARCH),hppa)
    ifneq ($(DEBUG),)
	CXXFLAGS	+= -g
    else
	ifneq ($(OPT),)
	    # Opimisation level specified (incompatible with -g)...
	    ifneq ($(LWOPT),0)
	        # Specific (non-zero) optimisation level set; use it
		CXXFLAGS+= +O$(OPT)
            endif
	else
            CXXFLAGS	+= +O2
	endif
    endif

    ifeq ($(CXX),)
        CXX	= aCC
    endif
    CXXFLAGS   += -AA +p				\
		  +DAportable				 \
		  -D_HPUX_SOURCE			  \
		  -D_POSIX_C_SOURCE=199506L		   \
		  -D_RWSTD_MULTI_THREAD
    # 
    # on HP-UX aCC, the -mt flag sets the right -D... and -l... flags for thread-safety.
    # Even if -mt is used to compile all objects, if -mt isn't used to link the target,
    # most all 'pthread_...' functions will compile to stubs, which is most efficient
    # in single-threaded executables.
    # 
    CXXFLAGS	       += -mt
endif

CXXFLAGS	+= -I../cut/

# 
# Basic Targets
# 
#     The default target 'all' builds (optional) unit test object, and
# source distribution package.  Neither of these are necessary to use
# REF.  The ref-test.o object may optionally be linked into
# applications, to include REF's CUT unit tests.
# 
.PHONY: all configure test testboost install dist distclean clean

all:			ref-test.o

configure:

test:			configure ref-test ref-test-fast
	$(TIME) ./ref-test
	$(TIME) ./ref-test-fast

testboost:		smarttest/smarttest smarttest/fillsort
	$(TIME) ./smarttest/smarttest
	$(TIME) ./smarttest/fillsort

testparallel: testparallel1 testparallel2 testparallel4 testparallel8 testparallel16 testparallel32 testparallel64

testparallel1:		configure ref-test
	$(TIME) bash -c "  ./ref-test >/dev/null \
		      & wait"

testparallel2:		configure ref-test
	$(TIME) bash -c "  ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & wait"

testparallel4:		configure ref-test
	$(TIME) bash -c "  ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & wait"

testparallel8:		configure ref-test
	$(TIME) bash -c "  ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & wait"

testparallel16:		configure ref-test
	$(TIME) bash -c "  ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & wait"

testparallel32:		configure ref-test
	$(TIME) bash -c "  ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & wait"

testparallel64:		configure ref-test
	$(TIME) bash -c "  ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & ./ref-test >/dev/null \
		      & wait"

install:
	cp ref ref.H $(TARGET)

dist:			distclean ref-$(VERSION).tgz

distclean:		clean

clean:
	-rm -f  *.tgz 				\
		*.html 				 \
		*~ 	*/*~ 			  \
		*.o 	*/*.o 			   \
		core* 	*/core*			    \
		ref-test ref-test-fast		     \
		smarttest/smarttest smarttest/fillsort

# 
# Unit Tests
# 
headers		= 	ref			\
			ref.H			 \
			../cut/cut.H

ref-test.o:		ref-test.C $(headers)
	$(CXX) $(CXXFLAGS) -c               -DTEST                     ref-test.C -o $@

ref-test: 		ref-test.C $(headers)
	$(CXX) $(CXXFLAGS) -DTESTSTANDALONE -DTEST                     ref-test.C -o $@

ref-test-iter_swap.o:	ref-test.C $(headers)
	$(CXX) $(CXXFLAGS) -c               -DTEST -DREF_PTR_ITER_SWAP ref-test.C -o $@

ref-test-iter_swap: 	ref-test.C $(headers)
	$(CXX) $(CXXFLAGS) -DTESTSTANDALONE -DTEST -DREF_PTR_ITER_SWAP ref-test.C -o $@

# 
# WARNING: Do not normally use REF_PTR_DEREF_FAST (or ref-test-fast.o)
# in production code!  You should instead use the template
# ref::ptr_fast<T> directly, where necessary.  We use this method here
# to test that ref::ptr_tiny<T> and ref::ptr_fast<T> are functionally
# equivalent, by running precisely the same unit tests in both forms.
# 
# If you can GUARANTEE that your code will never interoperate with
# other code also using REF, you may define REF_PTR_DEREF_FAST in your
# compilations (and link against ref-test-fast.o instead of
# ref-test.o, to include all REF unit tests); all instances of
# ref::ptr<T> will then implement ref::ptr_fast<T> instead of
# ref::ptr_tiny<T>; they will double in size (from 1 to 2 pointers),
# but will maintain 100% of the dereferencing speed of simple
# pointers.
# 
ref-test-fast.o:	$(headers) ref-test.C
	$(CXX) $(CXXFLAGS) -c               -DTEST                     -DREF_PTR_DEREF_FAST ref-test.C -o $@

ref-test-fast: 		$(headers) ref-test.C
	$(CXX) $(CXXFLAGS) -DTESTSTANDALONE -DTEST                      -DREF_PTR_DEREF_FAST ref-test.C -o $@

ref-test-iter_swap-fast.o: $(headers) ref-test.C
	$(CXX) $(CXXFLAGS) -c               -DTEST -DREF_PTR_ITER_SWAP -DREF_PTR_DEREF_FAST ref-test.C -o $@

ref-test-iter_swap-fast: $(headers) ref-test.C
	$(CXX) $(CXXFLAGS) -DTESTSTANDALONE -DTEST -DREF_PTR_ITER_SWAP -DREF_PTR_DEREF_FAST ref-test.C -o $@

# 
# Force generation of HTML unit test output, by indicated to CUT that
# it is executing in a CGI environment (REQUEST_METHOD=...).
# 
test-html:		ref-test.html ref-test-fast.html

ref-test.html:		ref-test
	REQUEST_METHOD=true ./ref-test > ref-test.html

ref-test-fast.html:	ref-test-fast
	REQUEST_METHOD=true ./ref-test-fast > ref-test-fast.html

# 
# Distribution
# 
ref-$(VERSION).tgz: 	configure		\
			GNUmakefile		 \
			ref			  \
			ref.H			   \
			COPYING			    \
			README			     \
	 		INSTALL			      \
			ref-test.C		       \
			smarttest
	rm -f   ref-$(VERSION)
	ln -s . ref-$(VERSION)
	tar czf $@ $(addprefix ref-$(VERSION)/,$^)
	rm -f   ref-$(VERSION)

# 
# Build Boost Smart Pointer tests
# 
#     A suite of Boost Smart Pointer tests is available; these have
# been extended to encompass ref::ptr as well, so the performance
# could be compared...  ref::ptr compares reasonably well, given its
# focus on maintaining conversion semantics of pointers, which
# requires the shared reference counting object to have virtual
# methods.
# 
smarttestheaders = 	smarttest/cyclic_ptr.hpp	\
			smarttest/fast_shared_ptr.hpp	\
			smarttest/linked_ptr.hpp	\
			smarttest/shared_in_ptr.hpp	\
			ref.H

smarttest/smarttest.o:	smarttest/smarttest.cpp		$(smarttestheaders)
smarttest/cyclic_ptr.o:	smarttest/cyclic_ptr.cpp	$(smarttestheaders)
smarttest/fillsort.o:	smarttest/fillsort.cpp		$(smarttestheaders)

smarttest/smarttest:	smarttest/smarttest.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@ $(LDLIBS)

smarttest/fillsort:	smarttest/fillsort.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@ $(LDLIBS)
