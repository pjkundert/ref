
// 
// ref-test.C		-- REF unit tests
// 
// Copyright (C) 2004 Enbridge Inc.
// 
// This file is part of the Enbridge REF Library
// 
// REF is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2, or (at your option) any later
// version.
// 
// REF is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
// 
// You should have received a copy of the GNU General Public License
// along with REF; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
// 

#include <ref>
#include <cut>

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <exception>
#include <set>
#include <deque>
#include <map>

#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>

#if defined( __GNUC__ )
#  define UNUSED		__attribute__(( unused ))
#else
#  define UNUSED
#endif

static char 			ident[] UNUSED = "@(#) $Id: ref-test.C,v 1.6 2006-10-05 17:28:37 kundertp Exp $ " __DATE__ "; " __TIME__;

// 
// ref::ptr<T> < ref::ptr<T>
// 
//     We must declare any desired comparison ref::... operators< we want used within the
// namespace ref.  Otherwise, it will not be seen from the '::' namespace, by the 'std::'
// algorithms.  This is due to the fact that C++ refuses to deduce function argument types, if the
// variant portion of the type is embedded within a class or namespace!  So, we'll put it in the
// same namespace as the type being used in the deduction...
// 
namespace ref {
    template <typename T> 
    inline bool operator<( const ref::ptr<T> &lhs, const ref::ptr<T> &rhs ) { return lhs.get() < rhs.get(); }
}

// 
// std::ostream& << std::map<T,U>
// 
template<typename T, typename U>
inline
std::ostream	       	       &operator<<(
				    std::ostream       &lhs,
				    const std::map<T,U>&rhs )
{
    for ( typename std::map<T,U>::const_iterator mi = rhs.begin()
	 ; mi != rhs.end()
	 ; ++mi ) {
	lhs << ( mi == rhs.begin()
		 ? ""
		 : ", " )		<< mi->first 
	    << ": "			<< mi->second;
    }
    return lhs;
}

#if defined( TESTSTANDALONE )

// Standalone test suite, if TESTSTANDALONE is defined.  
namespace cut {
    test			root( "Root REF unit test suite" );
}

int				main( 
				    int			argc,
				    char const 	      **argv )
{
    // if REQUEST_METHOD environment variable set, assume running as CGI
    bool			cgi	= getenv( "REQUEST_METHOD" );
    ref::ptr<cut::runner>       tests;
    if ( cgi )
	tests				= new cut::htmlrunner( std::cout );
    else
	tests				= new cut::runner( std::cout );

    // run returns true iff all test(s) successful
    if ( argc > 1 ) { 				// Named tests specified?
	for ( int i = 1; i < argc; ++i ) {	// Run each one
	    std::cout << "running test: " << argv[i] << std::endl;
	    if ( ! tests->run( argv[i] )) {
		return 1;
	    }
	}
    } else
	return ! tests->run();			// Run all tests
}

#endif // TESTSTANDALONE


#if defined( TEST )

//
// Unit Tests for ref::ptr classes
// 

namespace cut {

    class rcint
	: public ref::counter<rcint> {
	int			_object;

    public:
				rcint()
	{
	    ;
	}	
				rcint(
				    int		val )
				    : _object( val )
	{
	    ;
	}	
			       ~rcint()
	{
	    ;
	}
	virtual rcint	       *__ref_getptr()
	{
	    return this;
	}
				operator int()
	    const
	{
	    return _object;
	}
	int const	       &operator=(
					  int const       &rhs )
	{
	    _object		= rhs;
	    return _object;
	}
    };

    // We'll put the ref::counter<> AFTER the base class, just for laughs.
    class rcvecint
	: public std::vector<int>
	, public ref::counter<rcvecint> {
    public:
	int			_tmp;
				rcvecint()
	{
	    ;
	}
	virtual		       ~rcvecint()
	{
	    ;
	}
	virtual rcvecint       *__ref_getptr()
	{
	    return this;
	}
    };

    // 
    // For ref::ptr<Derived> to ref::ptr<Base> conversion tests.  Put in lots of padding, to
    // ensure that correct pointer conversions on access and destruction occur.
    // 
    class Base {
	bool			junkB[123];
    public:
				Base()
	{
	    ++basecnt;
	}
			       ~Base()
	{
	    --basecnt;
	}

	static int		basecnt;
        int			base;
    };
    int				Base::basecnt	= 0;		
    class Derived
        : public Base {
	short			junkD[456];
    public:
				Derived()
	{
	    ++derivedcnt;
	}
			       ~Derived()
	{
	    --derivedcnt;
	}
	
	static int		derivedcnt;
        int			derived;
    };
    int				Derived::derivedcnt = 0;

    class Derived1
        : public Derived {
	char			junkD1[456];
    public:
				Derived1()
	{
	    ++derived1cnt;
	}
			       ~Derived1()
	{
	    --derived1cnt;
	}
	
        static int		derived1cnt;
        int			derived1;
    };
    int				Derived1::derived1cnt = 0;

    class Other1 {
	char			junkO1[456];
    public:
				Other1()
	{
	    ++other1cnt;
	}
			       ~Other1()
	{
	    --other1cnt;
	}

        static int		other1cnt;
        int			other1;	
    };
    int				Other1::other1cnt = 0;

    class Other2 {
	char			junkO2[123];
    public:
				Other2()
	{
	    ++other2cnt;
	}
			       ~Other2()
	{
	    --other2cnt;
	}

        static int		other2cnt;
        int			other2;	
    };
    int				Other2::other2cnt = 0;

    // All classes from here on have virtual destructors; they can "forget" their class derivation
    // (so long as you use at least a Derived2*)...
    class Derived2
        : public Other1
        , public Derived1 {
	int			junkD2[1000];
    public:
				Derived2()
	{
	    ++derived2cnt;
	}
	virtual		       ~Derived2()
	{
	    --derived2cnt;
	}

        static int		derived2cnt;
        int			derived2;
    };
    int				Derived2::derived2cnt = 0;

    // Just like Derived2, but with a built-in ref::counter.  ref::ptr<Derived3> should use it's
    // built-in ref::counter.
    class Derived3
        : public Other2
	, public ref::counter<Derived3>
        , public Derived2 {
	virtual Derived3       *__ref_getptr()
	{
	    return this;
	}
	int			junkD3[1000];
    public:
				Derived3()
	{
	    ++derived3cnt;
	}
	virtual		       ~Derived3()
	{
	    --derived3cnt;
	}
        static int		derived3cnt;
        int			derived3;
    };
    int				Derived3::derived3cnt = 0;

    // Use Derived3, which has a built-in ref::counter.
    class Derived4
        : public Derived3 {
	int			junkD4[1000];
    public:
				Derived4()
	{
	    ++derived4cnt;
	}
	virtual		       ~Derived4()
	{
	    --derived4cnt;
	}
        static int		derived4cnt;
        int			derived4;
    };
    int				Derived4::derived4cnt = 0;


#if 0
    // 
    // checkrss
    // 
    //     Doesn't work on all architectures.  /proc stuff works OK on
    // linux, but not HP-UX.  getrusage for rusage::rs_idrss works on
    // neither platform.  Use actual heap address growth instead...
    // 
    int			checkrss()
    {
	std::string		name( "/proc/" );
	std::ostringstream	pid;
	pid 	<< (int)getpid();
	name 	+= pid.str();
	name 	+= "/status";
	std::ifstream		ifs( name.c_str() );
	while ( ifs ) {
	    std::string			s;
	    getline( ifs, s );
	    if ( s.find( "VmRSS:" ) != std::string::npos ) {
		// Got line!  
		std::istringstream	iss( s );
		iss >> s;	// ignore "VmRss:" part
		int			n;
		iss >> n;	// Get numeric part
		return n;
	    }
	}

	// Should not be reached, unless there IS no
	// /proc/... implemented on this architecture.  Then, just
	// return the ru_idrss from getrusage()
	rusage		usage;
	getrusage( RUSAGE_SELF, &usage );
	return (int)usage.ru_idrss;
    }
#endif

    template<class RP>
    inline
    bool
    passrefptr(
	cut::test	       &test,
	cut::runner	       &assert,
	unsigned int		cnt,		// The reference count before the call
	RP		       &r,		// the caller's ref::ptr
	const RP		p )		// our copy of the ref::ptr
    {
	assert.isequal( test, "both object's ref counts are identical", __FILE__, __LINE__, true, 
			p.__ref_getcnt(), r.__ref_getcnt() );
	if ( cnt ) {
	    assert.isequal( test, "there is an object; ref count increases", __FILE__, __LINE__, true,
			    p.__ref_getcnt() > cnt );
	    assert.isequal( test, "the objects should be equal in one order (ref. the same object)", __FILE__, __LINE__,
			    true, p == r );
	    assert.isequal( test, "the objects should be equal in the other order (ref. the same object)", __FILE__, __LINE__,
			    true, r == p );
	    assert.isequal( test, "the underlying object pointers should match after dereferencing const reference", __FILE__, __LINE__,
			    true, p == &*r );
	    assert.isequal( test, "the underlying object pointers should match after dereferencing const local", __FILE__, __LINE__,
			    true, r == &*p );
	} else {
	    assert.isequal( test, "there is no object; ref count stays zero", __FILE__, __LINE__, true,
			    p.__ref_getcnt(), cnt );
	    assert.isequal( test, "the objects should be false by reference (ref. no object)", __FILE__, __LINE__,
			    false, r );
	    assert.isequal( test, "the objects should be false by value (ref. no object)", __FILE__, __LINE__,
			    false, p );
	}

	return ( p.__ref_getcnt() == r.__ref_getcnt()
		 && ( cnt 
		      ? ( p.__ref_getcnt() > cnt ) 
		      : ( p.__ref_getcnt() == cnt )));
    }

    CUT( root, ref_tests,   "REF" ) {
	(void) assert;
    }

    CUT( ref_tests, ref_tests_general, "ref::ptr" ) {
	{
	    //	    std::cout << "Empty ref::ptr to int object " << std::endl;
	    ref::ptr<int>	a;
	}
	{
	    //	    std::cout << "Full  ref::ptr to int object by construction" << std::endl;
	    ref::ptr<int>	a( new int );
	}
	{
	    //	    std::cout << "Full  ref::ptr to int object by assignment" << std::endl;
	    ref::ptr<int>	a;
	    a		= new int;
	}
	{
	    //	    std::cout << "Empty ref::ptr to rcint object " << std::endl;
	    ref::ptr<rcint>	a;
	}
	{
	    //	    std::cout << "Full  ref::ptr to rcint object by construction " << std::endl;
	    ref::ptr<rcint>	a( new rcint );
	}
	{
	    //	    std::cout << "Full  ref::ptr to rcint object by assignment" << std::endl;
	    ref::ptr<rcint>	a;
	    a		= new rcint;
	}
	{
	    //	    std::cout << "Empty ref::ptr to rcvecint object " << std::endl;
	    ref::ptr<rcvecint>		a;
	}
	{
	    //	    std::cout << "Full  ref::ptr to rcvecint object by construction" << std::endl;
	    ref::ptr<rcvecint>	a( new rcvecint );
	}
	{
	    //	    std::cout << "Full  ref::ptr to rcvecint object by assignment" << std::endl;
	    ref::ptr<rcvecint>	a;
	    a		= new rcvecint;
	}


	ref::ptr<int> 		q;
	assert.ISEQUAL( q.__ref_getcnt(), 0U );

	// Test that doing this 10000 times doesn't leak memory.  Froth memory a bit, by
	// allocating a random sized chunk each time thru.
	void	       	       *maxaddr		= 0;
	void	 	       *lastmax		= 0;
	char		       *randmem		= 0;
	int			checki		= 1000;
	for ( int i = 0; i < 10000; ++i ) {

	    // Check max heap usage after first few of cycles; heap
	    // usage shouldn't increase after this point.  It takes a
	    // few cycles, because some ref::ptr<T>'s are retained
	    // (randomly) from loop to loop...
	    if ( i == checki ) {
		checki <<= 1;
		if ( maxaddr > lastmax )
		    assert.out() << "Loop " << i << ", max heap addr: " << maxaddr << " > last max: " << lastmax
				 << "(grown by " << long( maxaddr ) - long( lastmax ) << " bytes since last loop)" 
				 << std::endl;
		assert.ISFALSE( maxaddr > lastmax );
	    }
	    lastmax			= maxaddr;

	    // Froth.  Use 2 primes to create a rudimentary pseudo-random betwen 1 and ~1K.
	    if ( randmem )
		delete [] randmem;
	    randmem			= new char[(i*1619)%1021+1];

	    // When passing the current reference count, take the
	    // current reference count before passing the parameter
	    // list to the function, because the order of operations
	    // is undefined; it happens to evaluate the "copy
	    // constructor" before running the __ref_getcnt()...
	    unsigned int 		cnt;
	    cnt		= q.__ref_getcnt();
	    assert.ISTRUE( passrefptr( *this, assert, cnt, q, q ));
	    q		= new int;			// Each time thru loop, release old and replace by new dynamic int.
	    maxaddr	= std::max( maxaddr, (void *)q.get() );
	    cnt		= q.__ref_getcnt();
	    assert.ISEQUAL( cnt, 1U );
	    assert.ISTRUE( passrefptr( *this, assert, cnt, q, q ));
	    // make sure we're using a dynamic ref::counter (object doesn't have one)
	    assert.ISNOTEQUAL( (void *)q.__ref_getcounter(), (void *)q.get() );
	    *q	= 999;
	    cnt		= q.__ref_getcnt();
	    assert.ISEQUAL( cnt, 1U );
	    assert.ISTRUE( passrefptr( *this, assert, cnt, q, q ));
	    assert.ISEQUAL( q.__ref_getcnt(), 1U );
	    assert.ISEQUAL( *q, 999 );

	    {
		ref::ptr<Base>	x	= new Base();
		ref::ptr<Base>	y;
		assert.ISEQUAL( x.__ref_getcnt(), 1U );
		assert.ISEQUAL( y.__ref_getcnt(), 0U );
		{
		    y			= x;
		    assert.ISEQUAL( x.__ref_getcnt(), 2U );
		    assert.ISEQUAL( y.__ref_getcnt(), 2U );
		}
		assert.ISEQUAL( x.__ref_getcnt(), 2U );
		assert.ISEQUAL( y.__ref_getcnt(), 2U );
		ref::ptr<Derived> z	= new Derived();
		{
		    y 			= z;
		    assert.ISEQUAL( x.__ref_getcnt(), 1U );
		    assert.ISEQUAL( y.__ref_getcnt(), 2U );
		    assert.ISEQUAL( z.__ref_getcnt(), 2U );
		}
		assert.ISEQUAL( x.__ref_getcnt(), 1U );
		assert.ISEQUAL( y.__ref_getcnt(), 2U );
		assert.ISEQUAL( z.__ref_getcnt(), 2U );
		x			= 0;
		assert.ISEQUAL( x.__ref_getcnt(), 0U );
		assert.ISEQUAL( y.__ref_getcnt(), 2U );
		assert.ISEQUAL( z.__ref_getcnt(), 2U );
		z			= 0;
		assert.ISEQUAL( x.__ref_getcnt(), 0U );
		assert.ISEQUAL( y.__ref_getcnt(), 1U );
		assert.ISEQUAL( z.__ref_getcnt(), 0U );
		y			= 0;
		assert.ISEQUAL( x.__ref_getcnt(), 0U );
		assert.ISEQUAL( y.__ref_getcnt(), 0U );
		assert.ISEQUAL( z.__ref_getcnt(), 0U );

		// Now, set up a loop through different ref::ptr<T> types,
		// to the same underyling object.
	    }

	    // Test that using a ref::ptr<T> within an expression
	    // replacing that ref::ptr<T> with a new object doesn't
	    // perform the assignment until after the expression
	    // completes. 
	    {
		ref::ptr<int>	m	= new int( 99 );
		ref::ptr<rcint>	n	= new rcint( 99 );
		assert.ISEQUAL( *m, 99 );
		assert.ISEQUAL( (int)*n, 99 );

		// Now, replace the objects with new objects, but get
		// the reference counter for the old objects to ensure
		// they haven't been destructed before the expression
		// completes.
		m	= new int( m.__ref_getcnt() );
		n	= new rcint( n.__ref_getcnt() );
		assert.ISEQUAL( *m, 1 );
		assert.ISEQUAL( (int)*n, 1 );

		// Use the Copy Constructor to initialize some
		// object's derived from ref::counter<>;
		// ref::counter<>'s Copy Constructure should NOT copy
		// the original object's reference count into the new
		// object!  If it does, the new object will never
		// count down to zero, and memory will leak.  The
		// value should be the same (because the rcint was
		// copied) but the ref::counter should be zero (until
		// assigned to the new ref::ptr<rcint> then it will be
		// 1).
		assert.ISEQUAL( n.__ref_getcnt(), 1U );
		ref::ptr<rcint>	o	= new rcint( *n );
		assert.ISEQUAL( n.__ref_getcnt(), 1U );
		assert.ISEQUAL( o.__ref_getcnt(), 1U );

		assert.ISEQUAL( (int)*n, 1 );
		assert.ISEQUAL( (int)*o, 1 );
		*o	= 22;
		assert.ISEQUAL( (int)*n, 1 );
		assert.ISEQUAL( (int)*o, 22 );
	    }

	    // Test that going out of scope removes a reference to an
	    // arbitrary object.
	    {
		ref::ptr<int> 	r;
		assert.ISFALSE( r );
		assert.ISTRUE( ! r );
		r	= new int;
		assert.ISTRUE( ! ! r );
		maxaddr	= std::max( maxaddr, (void *)r.get() );
		assert.ISNOTEQUAL( (void *)r.__ref_getcounter(), (void *)r.get() );
		*r	= 888;
		assert.ISEQUAL( *q, 999 );
		assert.ISEQUAL( *r, 888 );
		assert.ISEQUAL( q.__ref_getcnt(), 1U );
		assert.ISEQUAL( r.__ref_getcnt(), 1U );

		assert.ISNOTEQUAL( (void *)q.__ref_getcounter(), (void *)r.__ref_getcounter() );
		q	= r;
		assert.ISEQUAL( (void *)q.__ref_getcounter(), (void *)r.__ref_getcounter() );
		assert.ISEQUAL( *q, 888 );
		assert.ISEQUAL( *r, 888 );
		assert.ISEQUAL( q.__ref_getcnt(), 2U );
		assert.ISEQUAL( r.__ref_getcnt(), 2U );
	    }
	    assert.ISEQUAL( *q, 888 );
	    assert.ISEQUAL( q.__ref_getcnt(), 1U );
	

	    // Test going out of scope with a simple ref::counter derived
	    // object.
	    ref::ptr<rcint>		a( new rcint );
	    maxaddr	= std::max( maxaddr, (void *)a.get() );

	    // make sure we're using the object's own ref::counter (it's a derived class)
	    assert.ISEQUAL( (void *)a.__ref_getcounter(), (void *)a.get());
	    *a 	= 999;
	    int c 	= *a;
	    assert.ISEQUAL( a.__ref_getcnt(), 1U );
	    assert.ISEQUAL( (int)*a, 999 );
	    assert.ISEQUAL( c, 999 );
	    {
		ref::ptr<rcint>	b;
		assert.ISTRUE( ! b );
		b	= new rcint;
		assert.ISTRUE( ! ! b );
		maxaddr	= std::max( maxaddr, (void *)b.get() );
		*b	= 888;
		assert.ISEQUAL( (int)*a, 999 );
		assert.ISEQUAL( (int)*b, 888 );
		assert.ISEQUAL( a.__ref_getcnt(), 1U );
		assert.ISEQUAL( b.__ref_getcnt(), 1U );

		assert.ISNOTEQUAL( (void *)a.__ref_getcounter(), (void *)b.__ref_getcounter() );
		a	= b;
		assert.ISEQUAL( (void *)a.__ref_getcounter(), (void *)b.__ref_getcounter() );
		assert.ISEQUAL( (int)*a, 888 );
		assert.ISEQUAL( (int)*b, 888 );
		assert.ISEQUAL( a.__ref_getcnt(), 2U );
		assert.ISEQUAL( b.__ref_getcnt(), 2U );
	    }
	    assert.ISEQUAL( (int)*a, 888 );

	    // Test going out of scope with a complex ref::counter derived object.  On some
	    // architectures, the order of the ref::counter<...> is put before, and on others
	    // after, the vector it shares the multiple derivation with.
	    ref::ptr<rcvecint>	x( new rcvecint );
	    maxaddr	= std::max( maxaddr, (void *)x.get() );
	    assert.ISEQUAL( (void *)x.__ref_getcounter(), (void *)(ref::counter<rcvecint> *)x.get());
	    x->_tmp 	= 999;
	    x->resize( 10 );
	    std::fill( dynamic_cast<std::vector<int> &>(*x).begin(),
		       dynamic_cast<std::vector<int> &>(*x).end(), 9 );

	    assert.ISEQUAL( x.__ref_getcnt(), 1U );
	    assert.ISEQUAL( x->at( 0 ), 9 );
	    assert.ISEQUAL( x->at( 1 ), 9 );
	    assert.ISEQUAL( x->at( 9 ), 9 );
	    {
		ref::ptr<rcvecint>	y;
		y	= new rcvecint;
		maxaddr	= std::max( maxaddr, (void *)y.get() );
		y->_tmp	= 888;
		y->resize( 10 );
		std::fill( static_cast<std::vector<int> &>(*y).begin(), 
			   static_cast<std::vector<int> &>(*y).end(), 8 );

		assert.ISEQUAL( x->_tmp, 999 );
		assert.ISEQUAL( x->at( 1 ), 9 );
		assert.ISEQUAL( y->_tmp, 888 );
		assert.ISEQUAL( static_cast<std::vector<int> &>(*y)[1], 8 );
		assert.ISEQUAL( x.__ref_getcnt(), 1U );
		assert.ISEQUAL( y.__ref_getcnt(), 1U );

		x	= y;
		assert.ISEQUAL( x->_tmp, 888 );
		assert.ISEQUAL( y->_tmp, 888 );
		assert.ISEQUAL( x.__ref_getcnt(), 2U );
		assert.ISEQUAL( y.__ref_getcnt(), 2U );
	    }
	    assert.ISEQUAL( x->_tmp, 888 );
	    assert.ISEQUAL( static_cast<std::vector<int> &>(*x)[1], 8 );


	    {
		ref::ptr<std::string>		s	= new std::string;
		ref::ptr<ref::dyn<std::string> >  rs	= new ref::dyn<std::string>;
		maxaddr	= std::max( maxaddr, (void *)s.get() );
		maxaddr	= std::max( maxaddr, (void *)rs.get() );
		assert.ISNOTEQUAL( (void *) s.__ref_getcounter(), (void *) s.get() );
		assert.ISEQUAL(    (void *)rs.__ref_getcounter(), (void *)rs.get() );

		*s     	+= "Hello";
		*rs    	+= "Hello";

		{
		    ref::ptr<std::string>	t	= s;
		    ref::ptr<ref::dyn<std::string> > rt	= rs;
		    *t	+= ", ";
		    *rs += ", ";
		}

		*s	+= "World!";
		*rs	+= "World!";

		assert.ISEQUAL( s->c_str(), "Hello, World!" );
		assert.ISEQUAL( rs->c_str(), "Hello, World!" );
	    }

	    {
		ref::ptr< ref::dyn<std::string> >	q1;
		{ 
		    ref::ptr< ref::dyn<std::string> > s;
		    s		= new ref::dyn<std::string>;
		    maxaddr	= std::max( maxaddr, (void *)s.get() );
		    *s	       += "Hello";
		    q1		= s;
		    *s	       += ", ";
		    assert.ISEQUAL( q1.__ref_getcnt(), 2U );
		}
		*q1    	       += "World!";		// *q == "Hello, World!";
		assert.ISEQUAL( q1->c_str(), "Hello, World!" );
		assert.ISEQUAL( q1.__ref_getcnt(), 1U );

		// degenerate use of ref::dyn<T>.  So long as u is never
		// assigned to a 'ref::ptr<ref::dyn<std::string> >', it
		// should never self-destruct and 'delete' itself.  It
		// will be destructed normally, as it goes out of scope.
		ref::dyn<std::string>       	u;

		u.assign( "Hello, World!" );				// one way to assign
		assert.ISEQUAL( u.c_str(), q1->c_str() );
		static_cast<std::string&>( u )	= "Hello, World!";	// another way.
		assert.ISEQUAL( u.c_str(), q1->c_str() );

		// Destruct the dynamically allocated
		// ref::dyn<std::string> assigned to q1.
		q1		= 0;
		assert.ISEQUAL( q1.__ref_getcnt(), 0U );
		
		// u should not destruct (but NOT delete!)
	    }


	    // Remember the last counter value each time around.
	    ref::ptr<int>			oldi;
	    ref::ptr<int>			thisi	= new int;
	    maxaddr	= std::max( maxaddr, (void *)thisi.get() );
	    *thisi		= i;
	    {
		static ref::ptr<int>		lasti;
		if ( lasti )
		    assert.ISEQUAL( *lasti + 1, i );
		// Randomly allow lasti to live beyond scope
		if ( random() & 1 )
		    oldi	= lasti;
		lasti		= thisi;
	    }
	    // If we allowed it to live, make sure its still the last loops number.
	    if ( oldi )
		assert.ISEQUAL( *oldi, i - 1 );


	    // 
	    // Now, ensure that passing-by-value does the expected
	    // thing.  If a ref::ptr<X> is used in a ( b ? 0 :
	    // ref::ptr<X> ) expression, does the whole thing evaluate
	    // to a raw X* pointer, OR to a ref::ptr<X>?  My guess is
	    // to an X* pointer.  In this case, we've just extracted a
	    // pointer to the underlying object, and passed it along
	    // to a user who won't properly be incrementing the
	    // original ref::counter<X> object!
	    // 
	    {
		ref::ptr<int>		a1	= new int;
		maxaddr		= std::max( maxaddr, (void *)a1.get() );
		bool			bit	= random() & 1;
		ref::ptr<int>		b	= ( bit ? ref::ptr<int>( 0 ) : a1 );
		if ( b ) {
		    assert.ISEQUAL( (void *)a1.get(), (void *)b.get() );
		    assert.ISEQUAL( (void *)a1.__ref_getcounter(), (void *)b.__ref_getcounter() );
		    assert.ISEQUAL( b.__ref_getcnt(), 2U );
		    assert.ISTRUE( passrefptr( *this, assert, 2U, b,
						( bit 
						  ? ref::ptr<int>( 0 ) 
						  : b )));	// converts to: `(bit ? ref::ptr<int>(0) : ref::ptr<int>((&b)))' 
		} else {
		    assert.ISEQUAL( b.__ref_getcnt(), 0U );
		}
		b		= a1;
		assert.ISEQUAL( b.__ref_getcnt(), 2U );
		assert.ISEQUAL( (void *)a1.get(), (void *)b.get() );
		assert.ISEQUAL( (void *)a1.__ref_getcounter(), (void *)b.__ref_getcounter() );
	    }

	    // Ensure the auto_array<T> works, roughly equivalently to a raw T* array
	    {
		typedef	ref::ptr<int>	T;

		T			rpi	= new int;
		*rpi		= 99;
		assert.ISEQUAL( rpi.__ref_getcnt(), 1U );
		{
		    T		       *aa	= new T[10];
		    ref::auto_array<T>	acln( aa );
		    ref::auto_array<T>	bb( new T[20] );
		    ref::auto_array<T>	cc( new T[30] );
		    aa[ 9] 	= new int;
		    *aa[ 9] 	=  9;
		    assert.ISEQUAL( aa[ 9].__ref_getcnt(), 1U );
		    aa[0]	= rpi;
		    bb[0] 	= rpi;
		    cc[0] 	= rpi;
		    cc[29]	= new int;
		    *cc[29]	= 29;
		    assert.ISEQUAL( *cc[0], 99 );
		    assert.ISEQUAL( rpi.__ref_getcnt(), 4U );
		}
		assert.ISEQUAL( rpi.__ref_getcnt(), 1U );

		{
		    int		       *iap	= new int[10];
		    ref::auto_array<int> iapcln( iap );

		    int		       *old	= iapcln;
		    assert.ISTRUE( old 		== iap );

		    int		      **ipp	= &iapcln;
		    assert.ISTRUE( *ipp 	== iap );

		    old		= *ipp;				// 'new' allocator might actually reuse the same memory area if we delete the old one first!
		    *ipp	= new int[20];
		    delete [] old;

		    assert.ISFALSE( *ipp 	== iap );
		    assert.ISFALSE( (int*)iapcln== iap );

		    assert.ISTRUE( (int*)iapcln	== *ipp );
		    assert.ISTRUE( (int*)iapcln == *ipp );
		    assert.ISTRUE( &iapcln	== ipp );

		    old		= iapcln;
		    iapcln	= new int[30];
		    assert.ISFALSE( (int*)iapcln== old );

		    iapcln	= 0;

		    int	      	      *&ref	= iapcln;	// should take a reference of the _underlying_ int* pointer, NOT of a copy of that pointer!
		    assert.ISTRUE( &ref		== &iapcln );	// the address of our ref should be the same as the address returned by the (T**)auto_array<int>::operator&

		}
		{
		    // support valid conversions of auto_array<T> to compatible pointer types, and
		    // handle 0 pointers correctly.
		    typedef unsigned char	uint8;
//		    ref::auto_array<uint8>	buf	= new uint8[10];
		    ref::auto_array<uint8>	buf(	  new uint8[10] );
		    strcpy( reinterpret_cast<char *>( &*buf ), "Hello" );
		    assert.ISEQUAL( reinterpret_cast<const char *>( &*buf ), "Hello" );

		    buf		= 0;				// releases previous array
		    assert.ISTRUE( (uint8 *)buf == (uint8 *)0 );// and test if it's pointer is now 0
		}
	    }


	    // 
	    // Test automatic conversion between ref::ptr<Derived> and ref::ptr<Base>.  We want
	    // ref::ptr<T> to have the same conversion semantics as T*.  For this group of tests,
	    // we'll ensure that a new <T> is always assigned to a ref::pt<T> -- that
	    // way, the automatically created ref::counter (a ref::count_other<T>, actually)
	    // always knows the actual type of its object, and hence triggers that appropriate
	    // non-virtual destructor.
	    // 
	    assert.ISEQUAL( Base::basecnt,		0 );
	    assert.ISEQUAL( Derived::derivedcnt, 	0 );
	    assert.ISEQUAL( Derived1::derived1cnt,	0 );
	    assert.ISEQUAL( Other1::other1cnt, 		0 );
	    assert.ISEQUAL( Other2::other2cnt, 		0 );
	    assert.ISEQUAL( Derived2::derived2cnt,	0 );
	    assert.ISEQUAL( Derived3::derived3cnt,	0 );
	    assert.ISEQUAL( Derived4::derived4cnt,	0 );
	    {
		Derived	       *d0	= new Derived;
		maxaddr	= std::max( maxaddr, (void *)d0 );
		ref::ptr<Derived> d	= d0;
		assert.ISEQUAL( d.__ref_getcnt(),  1U );
		ref::ptr<Base>	b1( d );
		assert.ISEQUAL( d.__ref_getcnt(),  2U );
		assert.ISEQUAL( b1.__ref_getcnt(), 2U );
		ref::ptr<Base>	b2	= d;
		assert.ISEQUAL( d.__ref_getcnt(),  3U );
		assert.ISEQUAL( b1.__ref_getcnt(), 3U );
		ref::ptr<Base>	b3;
		assert.ISEQUAL( d.__ref_getcnt(),  3U );
		assert.ISEQUAL( b1.__ref_getcnt(), 3U );
		{
		    b3			= d;
		    assert.ISEQUAL( d.__ref_getcnt(),  4U );
		    assert.ISEQUAL( b1.__ref_getcnt(), 4U );
	    	}
		assert.ISEQUAL( d.__ref_getcnt(),  4U );
		assert.ISEQUAL( b1.__ref_getcnt(), 4U );

		assert.ISEQUAL( Base::basecnt, 		1 );
		assert.ISEQUAL( Derived::derivedcnt,	1 );
		assert.ISEQUAL( Derived1::derived1cnt,	0 );
		assert.ISEQUAL( Other1::other1cnt,	0 );
		assert.ISEQUAL( Other2::other2cnt, 	0 );
		assert.ISEQUAL( Derived2::derived2cnt,	0 );
		assert.ISEQUAL( Derived3::derived3cnt,	0 );
		assert.ISEQUAL( Derived4::derived4cnt,	0 );

		// 
		// As expected, the following code illustrates that conversions which do not have
		// a default converstion specified (eg. from a Base class to a Derived class, or
		// from a completely unrelated class), then fail.
		// 
		// ref::ptr.H:435: error: invalid conversion from `cut::Base*' to `cut::Derived*'
		// ref::ptr.H:435: error: cannot convert `cut::Base*' to `int*' in initialization
		// 
#if defined( REF_PTR_FAIL_1 )
		d			= b1;
#endif
#if defined( REF_PTR_FAIL_2 )
		ref::ptr<int>	e;
		e			= b1;
#endif

		// 
		// As expected, the following code shows that "automatic" conversions from a
		// ref::ptr<T> to T* fail.
		// 
#if defined( REF_PTR_FAIL_3 )
		ref::ptr<int>	irp	= new int;
		int	       *ip;
		ip			= irp;
#endif

		// 
		// Now, what about Base classes that don't start at
		// the beginning of the Derived object (multiple
		// inheritance)?  They must be converted using the
		// proper pointer conversion operation...
		// 
		ref::ptr<Derived2>			d2	= new Derived2;
		maxaddr	= std::max( maxaddr, (void *)d2.get() );
		d2->derived2		= 10002;
		d2->derived1		= 10001;
		d2->derived		= 10000;
		d2->other1		= 20001;
		d2->base		=  9999;
		ref::ptr<Derived1> d1	= d2;
		assert.ISEQUAL( d2->derived1, 10001 );
		assert.ISEQUAL( d1->derived1, 10001 );

		assert.ISEQUAL( Base::basecnt,		2 );
		assert.ISEQUAL( Derived::derivedcnt,	2 );
		assert.ISEQUAL( Derived1::derived1cnt,	1 );
		assert.ISEQUAL( Other1::other1cnt,	1 );
		assert.ISEQUAL( Other2::other2cnt, 	0 );
		assert.ISEQUAL( Derived2::derived2cnt,	1 );
		assert.ISEQUAL( Derived3::derived3cnt,	0 );
		assert.ISEQUAL( Derived4::derived4cnt,	0 );


		// Now, ensure that an object shared by multiple
		// "adapted" ref::ptr<> objects auto-destructs
		// appropriately.  ref::ptr<Base> b1, b2 and b3 share
		// the object managed by ref::ptr<Derived> d.  Let's leave just one of them
		// to clean up the Derived object.
		d			= 0;
		b1			= 0;
		b2			= 0;

		// Now, ensure that destruction is triggered correctly
		// via the "adapted" Base pointer.  The 'd2' adapted
		// ref::ptr<> will be the last one left, and should
		// trigger destruction of the object.
		d1			= 0;
		assert.ISEQUAL( d2.__ref_getcnt(), 1U );

	        // Let's just leave a long chain of ref::ptr<> objects
		// using ref_count_adapter<>s, which must destruct in
		// a chain to get rid of the underlying Derived2
		// object.  Wrap them, to limit the scope of any temporaries.
		{
		    d1			= d2;
		    d			= d1;
		}
		assert.ISEQUAL( d2.__ref_getcnt(), 3U );

		ref::ptr<Base>	b	= d;
		assert.ISEQUAL( d2.__ref_getcnt(), 4U );
		assert.ISEQUAL( b.__ref_getcnt(),  4U );
	    }

	    assert.ISEQUAL( Base::basecnt,		0 );
	    assert.ISEQUAL( Derived::derivedcnt, 	0 );
	    assert.ISEQUAL( Derived1::derived1cnt,	0 );
	    assert.ISEQUAL( Other1::other1cnt, 		0 );
	    assert.ISEQUAL( Other2::other2cnt, 		0 );
	    assert.ISEQUAL( Derived2::derived2cnt,	0 );
	    assert.ISEQUAL( Derived3::derived3cnt,	0 );
	    assert.ISEQUAL( Derived4::derived4cnt,	0 );

	    // 
	    // Test automatic conversion between ref::ptr<Derived...> and ref::ptr<Base>.  We want
	    // ref::ptr<T> to have the same conversion semantics as T*.  For this group of tests,
	    // however, we'll lose the true type of the dynamically allocated object we assigned
	    // to a ref::ptr<T> -- that way, the created ref::counter (a ref::count_other<T>,
	    // actually) will NOT know the actual type of its object, and if it triggered a
	    // non-virtual destructor, it would not propery destruct the object.  Hence, we must
	    // use objects with virtual destructors.  Since ref::counter<T> defines a virtual
	    // destructor, we should be OK.
	    // 
	    // Use a type <Derived2> that has NO built-in ref::ptr.  Even though the actual object
	    // of type Derived4 HAS a ref::ptr, it's true type is lost, and the <Derived2> type
	    // does NOT have one...
	    {
		Derived4       *d4	= new Derived4;
		maxaddr	= std::max( maxaddr, (void *)d4 );
		ref::ptr<Derived2> d	= d4;	// Lose true type!

		ref::ptr<Base>	b1( d );
		ref::ptr<Base>	b2	= d;
		ref::ptr<Base>	b3;
		b3			= d;
		assert.ISEQUAL( d.__ref_getcnt(), 4U );
		assert.ISEQUAL( b1.__ref_getcnt(), 4U );

		assert.ISEQUAL( Base::basecnt, 		1 );
		assert.ISEQUAL( Derived::derivedcnt,	1 );
		assert.ISEQUAL( Derived1::derived1cnt,	1 );
		assert.ISEQUAL( Other1::other1cnt,	1 );
		assert.ISEQUAL( Other2::other2cnt, 	1 );
		assert.ISEQUAL( Derived2::derived2cnt,	1 );
		assert.ISEQUAL( Derived3::derived3cnt,	1 );
		assert.ISEQUAL( Derived4::derived4cnt,	1 );
	    }

	    assert.ISEQUAL( Base::basecnt,		0 );
	    assert.ISEQUAL( Derived::derivedcnt, 	0 );
	    assert.ISEQUAL( Derived1::derived1cnt,	0 );
	    assert.ISEQUAL( Other1::other1cnt, 		0 );
	    assert.ISEQUAL( Other2::other2cnt, 		0 );
	    assert.ISEQUAL( Derived2::derived2cnt,	0 );
	    assert.ISEQUAL( Derived3::derived3cnt,	0 );
	    assert.ISEQUAL( Derived4::derived4cnt,	0 );

	    // Now, use a type <Derived3> that has a built-in ref::ptr.  It should be used.
	    // Ensure we use the native ref::ptr<Derived3>::operator=( Derived3 * ) assignment
	    // operator!
	    {
		Derived4       *d4	= new Derived4;
		maxaddr	= std::max( maxaddr, (void *)d4 );
		ref::ptr<Derived3> d	= d4;		// Lose true type!

		assert.ISTRUE( d.__ref_getcounter() == (ref::counter<Derived3> *)d4 );

		ref::ptr<Base>	b1( d );
		ref::ptr<Base>	b2	= d;
		ref::ptr<Base>	b3;
		b3			= d;
		assert.ISEQUAL( d.__ref_getcnt(), 4U );
		assert.ISEQUAL( b1.__ref_getcnt(), 4U );

		assert.ISEQUAL( Base::basecnt, 		1 );
		assert.ISEQUAL( Derived::derivedcnt,	1 );
		assert.ISEQUAL( Derived1::derived1cnt,	1 );
		assert.ISEQUAL( Other1::other1cnt,	1 );
		assert.ISEQUAL( Other2::other2cnt, 	1 );
		assert.ISEQUAL( Derived2::derived2cnt,	1 );
		assert.ISEQUAL( Derived3::derived3cnt,	1 );
		assert.ISEQUAL( Derived4::derived4cnt,	1 );
	    }

	    assert.ISEQUAL( Base::basecnt,		0 );
	    assert.ISEQUAL( Derived::derivedcnt, 	0 );
	    assert.ISEQUAL( Derived1::derived1cnt,	0 );
	    assert.ISEQUAL( Other1::other1cnt, 		0 );
	    assert.ISEQUAL( Other2::other2cnt, 		0 );
	    assert.ISEQUAL( Derived2::derived2cnt,	0 );
	    assert.ISEQUAL( Derived3::derived3cnt,	0 );
	    assert.ISEQUAL( Derived4::derived4cnt,	0 );

	    {
		// ref::ptr<const T> should be able to point to and
		// share ref::ptr<T> object's, and their reference
		// counters.  The opposite should not work.  This is
		// done using the ref_count_adapter, because <T> can
		// be implicitly converted to a <const T>.
		
		ref::ptr<int>		a1	= new int;
		*a1 	= 1;
		ref::ptr<const int>	ca0	= new int( 5 );
		ref::ptr<const int>	ca1( a1 );
		ref::ptr<const int>	ca2	= a1;
		// The ref::counter is not the same (a
		// ref_count_adapter<Base,Derived> is used)...
		assert.ISFALSE( ca1.__ref_getcounter() == dynamic_cast<ref::counter<const int>*>( a1.__ref_getcounter()));
		assert.ISFALSE( ca2.__ref_getcounter() == dynamic_cast<ref::counter<const int>*>( a1.__ref_getcounter()));
		// but the underlying object is shared!
		assert.ISTRUE(  ca1.get() == static_cast<const int *>( a1.get()));
		assert.ISTRUE(  ca2.get() == static_cast<const int *>( a1.get()));
		assert.ISEQUAL( a1.__ref_getcnt(), 3U );

		assert.ISFALSE( dynamic_cast<ref::counter<const int> *>( a1.__ref_getcounter() ) == ca1.__ref_getcounter());
		assert.ISFALSE( dynamic_cast<ref::counter<const int> *>( a1.__ref_getcounter() ) == ca2.__ref_getcounter());
		assert.ISTRUE(  static_cast<const int *>( a1.get() ) == ca1.get());
		assert.ISTRUE(  static_cast<const int *>( a1.get() ) == ca2.get());

		// The underlying ref::counter count is mutable, so is
		// changable even in const objects containing their
		// own counters.
		ref::ptr<rcint>		ra	= new rcint;
		ref::ptr<const rcint>	cra0	= new rcint( 5 );
		ref::ptr<const rcint>	cra1( ra );
		ref::ptr<const rcint>	cra2	= ra;
		assert.ISFALSE( cra1.__ref_getcounter() == dynamic_cast<ref::counter<const rcint> *>( ra.__ref_getcounter() ));
		assert.ISFALSE( cra2.__ref_getcounter() == dynamic_cast<ref::counter<const rcint> *>( ra.__ref_getcounter() ));
		assert.ISTRUE(  cra1.get() == static_cast<const rcint *>( ra.get()));
		assert.ISTRUE(  cra2.get() == static_cast<const rcint *>( ra.get()));
		assert.ISEQUAL( a1.__ref_getcnt(), 3U );

		// 
		// Uncomment these to test the rejection of promotion
		// of <const T> to <T>.
		// 
		// ref::ptr<rcint>		wrong0 	= new const rcint( 5 );
		// ref::ptr<rcint>		wrong1( cra0 );	
		// ref::ptr<rcint>		wrong2	= cra0;
	    }

	    {
		// Ensure that empty ref::ptr<T>'s are handled
		// efficiently; no matter how they are copied, the
		// recipient ref::ptr<X> should allocate no
		// ref_count_other<X>, ref_count_adapter<X>, etc.
		ref::ptr<Derived>		d0;
		ref::ptr<Derived>		d1	= d0;
		ref::ptr<Derived>		d2( d0 );
		ref::ptr<Derived>		d3;
		ref::ptr<Derived>		d4	= (Derived *)0;
		d3		= d0;
		assert.ISTRUE( ! d0.__ref_getcounter() );
		assert.ISTRUE( ! d1.__ref_getcounter() );
		assert.ISTRUE( ! d2.__ref_getcounter() );
		assert.ISTRUE( ! d3.__ref_getcounter() );

		ref::ptr<Base>			b1	= d0;
		ref::ptr<Base>			b2( d0 );
		ref::ptr<Base>			b3;
		ref::ptr<Base>			b4	= (Derived *)0;
		b3		= d0;
		assert.ISTRUE( ! b1.__ref_getcounter() );
		assert.ISTRUE( ! b2.__ref_getcounter() );
		assert.ISTRUE( ! b3.__ref_getcounter() );
		assert.ISTRUE( ! b4.__ref_getcounter() );

		// Now, assignments requiring adapters require them to
		// be created on the fly.  This will only occur when
		// assigning a non-empty ref::ptr<Derived> to a
		// ref::ptr<Base>.  Since there is no templated
		// assignment operator to do this directly, the
		// compiler uses the constructor to create a temporary
		// ref::ptr<Base> from a ref::ptr<Derived> (which uses
		// ref_count_adapter<Base,Dirived> to do its reference
		// counting).  Then, it uses the ref::ptr<Base> =
		// ref::ptr<Base> assignment operator to do the
		// assigment.  While slightly less efficient than
		// having a direct assignment operator (an extra
		// temporary and an increment/decrement pair), this
		// operation is not common enough to warrant the added
		// complexity to ref::ptr<T>...
		// 
		// Even after adding the operator=, temporaries are still used on
		// some platforms.  Use braces to constrain their lifetime.
		d0		= new Derived;
		assert.ISEQUAL( d0.__ref_getcnt(), 1U );
		{
		    b1		= d0;
		}
		assert.ISEQUAL( d0.__ref_getcnt(), 2U );
		assert.ISTRUE( b1.__ref_getcounter() );
		assert.ISTRUE( b1.get() == static_cast<Base *>( d0.get() ));
	    }
	    {
		// Test the self-assignment is handled correctly; it
		// should be a no-op.
		ref::ptr<rcint>		e	= new rcint;
		assert.ISEQUAL( e.__ref_getcnt(), 1U );
		e 	= e;
		assert.ISEQUAL( e.__ref_getcnt(), 1U );
	    }
	}
	// Do final check on heap growth; shouldn't have grown since the check above..
	if ( maxaddr > lastmax )
	    assert.out() << "After Loop, max heap addr: " << maxaddr << " > last max: " << lastmax << std::endl;
	assert.ISFALSE( maxaddr > lastmax );
    }

} // namespace cut

namespace cut {

    CUT( ref_tests, ref_containers,  "ref::ptr container" ) {
	std::set<ref::ptr<int> >		rpintset;
	ref::ptr<int>		a	= new int;	*a = 1;
	ref::ptr<int>		b	= new int;	*b = 2;
	ref::ptr<int>		c	= new int;	*c = 3;
	ref::ptr<int>		d	= a;
	assert.ISTRUE( rpintset.insert( a ).second );
	assert.ISEQUAL( rpintset.size(), (size_t)1 );
	assert.ISTRUE( rpintset.insert( b ).second );
	assert.ISTRUE( rpintset.insert( c ).second );
	assert.ISEQUAL( rpintset.size(), (size_t)3 );
	assert.ISFALSE( rpintset.insert( c ).second );
	assert.ISEQUAL( rpintset.size(), (size_t)3 );
	// Same underlying value, same object; should NOT insert.
	assert.ISFALSE( rpintset.insert( d ).second );
	assert.ISEQUAL( rpintset.size(), (size_t)3 );

	// The object is referenced thrice; by d, by a, and by the rpintset container
	assert.ISEQUAL( a.__ref_getcnt(), (unsigned int)3 );

	// Same underlying value, but different object!  Should insert.
	d	= new int; *d = 3;
	assert.ISTRUE( rpintset.insert( d ).second );
	assert.ISEQUAL( rpintset.size(), (size_t)4 );
	// Different underlying value, but same object; should NOT insert.
	*d = 4;
	assert.ISFALSE( rpintset.insert( d ).second );
	assert.ISEQUAL( rpintset.size(), (size_t)4 );

	assert.ISEQUAL( a.__ref_getcnt(), (unsigned int)2 );
	assert.ISEQUAL( d.__ref_getcnt(), (unsigned int)2 );

	// Now, point d back to a; there is now a thing in the
	// rpintset that is ONLY referenced by the set!  See if it
	// copies OK.
	d = a;
	assert.ISEQUAL( a.__ref_getcnt(), (unsigned int)3 );
	d = 0;
	assert.ISEQUAL( a.__ref_getcnt(), (unsigned int)2 );
	std::deque<ref::ptr<int> >	rpintdeq;
	std::copy( rpintset.begin(), rpintset.end(), back_inserter( rpintdeq ));

	assert.ISEQUAL( a.__ref_getcnt(), (unsigned int)3 );
	// Find the one that contains an int equal to 4; that will
	// only be referenced by the ...set and the ...deq; there is
	// no external ref::ptr<int> referencing it.
	for ( std::set<ref::ptr<int> >::iterator si = rpintset.begin()
	      ; si != rpintset.end()
	      ; ++si ) {
	    if ( **si == 4 )
		assert.ISEQUAL( (*si).__ref_getcnt(), (unsigned int)2 );
	    else
		assert.ISEQUAL( (*si).__ref_getcnt(), (unsigned int)3 );
	    if ( **si == 2 )
		assert.ISTRUE( *si == b );
	    else {
		assert.ISFALSE( *si == b );
		assert.ISTRUE( *si != b );
	    }
	}
	std::map<ref::ptr<int>, int>
				rpintmap;
	assert.ISEQUAL( a.__ref_getcnt(), (unsigned int)3 );
	rpintmap[a]	= 123;
	assert.ISEQUAL( a.__ref_getcnt(), (unsigned int)4 );
	
    }

    CUT( ref_tests, ref_deref, "ref::ptr deref" ) {
	{
	    ref::ptr<int>	a;
	    try {
		int	       *b UNUSED = &*a;
#if defined( REF_PTR_DEREF_TEST )
		assert.FAILED( "Didn't throw!", "Should have caught an exception on invalid ref::ptr<int> deref." );
#else
		assert.ISTRUE( b == 0 );
#endif
	    } catch ( std::exception &e ) {
		// eg: "ref::ptr 0xBFFFE330 is attempting to dereference, but no object is assigned!"
		assert.ISEQUAL( ( strlen( e.what() ) > 37 
				  ? e.what() + 37
				  : "" )
				, " is attempting to dereference, but no ref-counted object has been assigned!" );
	    }
	}

	{
	    const ref::ptr<int>	a;
	    try {
		int            *b UNUSED = &*a;
#if defined( REF_PTR_DEREF_TEST )
		assert.FAILED( "Didn't throw!", "Should have caught an exception on invalid ref::ptr<const int> deref." );
#else
		assert.ISTRUE( b == 0 );
#endif
	    } catch ( std::exception &e ) {
		// eg: "const ref::ptr 0xBFFFE330 is attempting to dereference, but no object is assigned!"
		assert.ISEQUAL( ( strlen( e.what() ) > 37
				  ? e.what() + 37
				  : "" )
				, " is attempting to dereference, but no ref-counted object has been assigned!" );
	    }
	}
	
	{
	    ref::ptr<rcint>	a;
	    try {
		rcint	       *b UNUSED = &*a;
#if defined( REF_PTR_DEREF_TEST )
		assert.FAILED( "Didn't throw!", "Should have caught an exception on invalid ref::ptr<ref::counter<int> > deref." );
#else
		assert.ISTRUE( b == 0 );
#endif
	    } catch ( std::exception &e ) {
		// eg: "ref::ptr 0xBFFFE330 is attempting to dereference, but no object is assigned!"
		assert.ISEQUAL( ( strlen( e.what() ) > 37
				  ? e.what() + 37
				  : "" )
				, " is attempting to dereference, but no ref-counted object has been assigned!" );
	    }
	}
    }

    
    class object { public: int __; };
    
    // Make an "object" having its own ref::counter; ref::ptr will use it..
    class rco
      : public object
      , public ref::counter<rco> {
        virtual rco * __ref_getptr() { return this; }
        /* ... */
    };

    // Take reference-counting parameters
    void fun( ref::ptr<int> d ) {
	// Assign 99 to the LAST call's object (if any)!
        static ref::ptr<int>	e;
	if ( e )
            *e 	= 99;

        // Forget last call's object (will self-destruct
        // if no-one else holds it), and remember this new one
        // for next time!
        e	= d;				
    }

    CUT( ref_tests, ref_demo, "ref::ptr demo code" ) {

	ref::ptr<rco>		r = new rco;

	// Add reference-counting and self-destruction to some other object
	ref::ptr<ref::dyn<object> >
	    			o = new ref::dyn<object>;	

	ref::ptr<int>		a( new int );		// new int 'A' on heap, 'a' holds ref::counter of 1 to 'A'
	ref::ptr<int>		b;			// 'b's ref::counter is 0, and it's pointer ( *b, b->, ...) is 0.
	b		= new int;			// new int 'B' on heap; 'b's ref::counter is 1
	ref::ptr<int>		c;
	c 		= b;				// 'b' and 'c' share ref::counter of 2 to 'B'
	a		= c;				// 'A' freed; 'a', 'b' and 'c' share ref::counter of 3 to 'B'
    
	fun( a );					// fun's parameter 'd' now shares ref::counter of 4 to 'B'
	assert.ISEQUAL( a.__ref_getcnt(), (unsigned int)( 4 ));

	ref::ptr<rco>		e; 
	{
	    ref::ptr<rco>	f = new rco;		// 'f' uses <rco> object 'R's own ref::counter base class
	    e	= f;					// 'R' ref::counter is now 2
	    assert.ISEQUAL( e.__ref_getcnt(), (unsigned int)( 2 ));
	}						// 'R's ref::counter is now 1 (f passed out of scope)
	assert.ISEQUAL( e.__ref_getcnt(), (unsigned int)( 1 ));
	e		= 0;				// 'R' is freed.
    }

    class intobj {
    public:
	mutable volatile int	_value;			// don't optimise away accesses!

    public:
				intobj()		// Default constructor necessary for ref::dyn<intobj>
				    : _value( 0 )
	{
	    ;
	}
				intobj( 
				    int			value )
				    : _value( value )
	{
	    ;
	}
	virtual		       ~intobj()
	{
	    ; 
	}

    public:
	int			operator=(
				    int		value )
	{
	    return _value		= value;
	}
	int			getvalue()
            const
	{	
	    return _value;
	}
	virtual int		getvaluevirtual()
	    const
	{
	    return _value;
	}

	virtual int		assign(			// necessary for ref::dyn<intobj>; can't access operator=
				    int		value )
	{
	    return *this		= value;
	}
    };

    class rcintobj
	: public ref::counter<rcintobj>
	, public intobj {
	virtual rcintobj       *__ref_getptr() { return this; }
    public:
				rcintobj(
				    int 		value 	= 0 )
				    : intobj( value )
	{ 
	    ; 
	}
	virtual 	       ~rcintobj()
 	{
	    ; 
	}
    };


    inline
    int				duration(
				    const timeval      &lhs,
				    const timeval      &rhs )
    {
	return ( int( lhs.tv_sec 
		      - rhs.tv_sec )	* 1000000
		 + int( lhs.tv_usec )
		 - int( rhs.tv_usec ));
    }

    inline
    timeval			timevalnow()
    {
	timeval			tmp;
	gettimeofday( &tmp, 0 );
	return tmp;
    }

    CUT( ref_tests, ref_rate, "ref::ptr rate" ) {

	intobj		       *direct	= new intobj( 1 );
	timeval			begin;
	int 			sum;


	// Size of the ref::ptr<T> object, relative to a pointer, and ref::counter, relative to
	// an integer.
	{
	    ref::ptr<intobj>	tmp;
	    assert.out() << "sizeof( ref::ptr<T> ):                            "
			 << std::setw( 5 ) << sizeof tmp	<< " bytes, vs. "
			 << std::setw( 5 ) << sizeof direct	<< " bytes == "
			 << ( sizeof tmp ) / ( sizeof direct )	<< " x sizeof( T * )" 		<< std::endl;

	    intobj		vo;
	    rcintobj		rcvo;
	    assert.out() << "sizeof( ref::counter<T>-derived virtual object ): "
			 << std::setw( 5 ) << sizeof rcvo	<< " bytes, vs. "
			 << std::setw( 5 ) << sizeof vo		<< " bytes == "
			 << ( sizeof rcvo ) - ( sizeof vo )	<< " extra bytes" 		<< std::endl;

	    object		so;
	    rco			rcso;
	    assert.out() << "sizeof( ref::counter<T>-derived simple object ):  "
			 << std::setw( 5 ) << sizeof rcso	<< " bytes, vs. "
			 << std::setw( 5 ) << sizeof so		<< " bytes == "
			 << ( sizeof rcso ) - ( sizeof so )	<< " extra bytes" 		<< std::endl;

	    ref::count_other<object> rcoo( 0 );
	    assert.out() << "sizeof( ref::count_other<T> ) (shared counter):   "
			 << std::setw( 5 ) << sizeof rcoo	<< " extra bytes" 		<< std::endl;
	}

	// Access object via pointer
	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += direct->_value )
	    ;
	int dirsimpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "pointer access,   not indirect,    simple:      "
		     << std::setw( 5 ) << sum / dirsimpassed  	<< "/usec, over " 
		     << std::setw( 5 ) << dirsimpassed 		<< " usecs. (baseline simple)" << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += direct->getvalue() )
	    ;
	int dirnovpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "pointer access,   not indirect,    non-virtual: "
		     << std::setw( 5 ) << sum / dirnovpassed  	<< "/usec, over " 
		     << std::setw( 5 ) << dirnovpassed 		<< " usecs. (baseline non-virtual)" << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 	       	       += direct->getvaluevirtual() )
	    ;
	int dirvirpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "pointer access,   not indirect,    virtual:     "
		     << std::setw( 5 ) << sum / dirvirpassed	<< "/usec, over " 
		     << std::setw( 5 ) << dirvirpassed 		<< " usecs. (baseline virtual)" << std::endl;

	// Access object via indirect pointer
	intobj	    * volatile * volatile indirect = &direct;	// don't allow indirection to be optimised away
	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += (*indirect)->_value )
	    ;
	int indsimpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "pointer access,       indirect,    simple:      "
		     << std::setw( 5 ) << sum / indsimpassed	<< "/usec, over " 
		     << std::setw( 5 ) << indsimpassed 		<< " usecs. ("
		     << dirsimpassed * 100 / indsimpassed	<< "%)"		// ( 10,000 * 100 ) / 12,000	== 83% as fast
		     << std::endl;	

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += (*indirect)->getvalue() )
	    ;
	int indnovpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "pointer access,       indirect,    non-virtual: "
		     << std::setw( 5 ) << sum / indnovpassed	<< "/usec, over " 
		     << std::setw( 5 ) << indnovpassed 		<< " usecs. ("
		     << dirnovpassed  * 100 / indnovpassed	<< "%)"
		     << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 	       	       += (*indirect)->getvaluevirtual() )
	    ;
	int indvirpassed			= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "pointer access,       indirect,    virtual:     "
		     << std::setw( 5 ) << sum / indvirpassed	<< "/usec, over " 
		     << std::setw( 5 ) << indvirpassed 		<< " usecs. (" 
		     << dirvirpassed * 100 / indvirpassed	<< "%)"
		     << std::endl;

	// Access object via ref::ptr, no built-in ref::counter (dynamically allocated)
	ref::ptr<intobj>	refptr	= direct;	// Will auto-destruct new intobj, above
	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += refptr->_value )
	    ;
	int rptsimpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "ref::ptr access,  no ref::counter, simple:      "
		     << std::setw( 5 ) << sum / rptsimpassed	<< "/usec, over " 
		     << std::setw( 5 ) << rptsimpassed 		<< " usecs. ("
		     << dirsimpassed * 100 / rptsimpassed	<< "%)"
		     << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += refptr->getvalue() )
	    ;
	int rptnovpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "ref::ptr access,  no ref::counter, non-virtual: "
		     << std::setw( 5 ) << sum / rptnovpassed	<< "/usec, over " 
		     << std::setw( 5 ) << rptnovpassed 		<< " usecs. ("
		     << dirnovpassed * 100 / rptnovpassed	<< "%)"
		     << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += refptr->getvaluevirtual() )
	    ;
	int rptvirpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "ref::ptr access,  no ref::counter, virtual:     "
		     << std::setw( 5 ) << sum / rptvirpassed	<< "/usec, over " 
		     << std::setw( 5 ) << rptvirpassed	 	<< " usecs. ("
		     << dirvirpassed * 100 / rptvirpassed	<< "%)"
		     << std::endl;

	// Access object via ref::ptr, w/ref::counter built into object
	ref::ptr<rcintobj>	refctr	= new rcintobj( 1 );
	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 	       	       += refctr->_value )
	    ;
	int ctrsimpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "ref::ptr access,  w/ ref::counter, simple:      "
		     << std::setw( 5 ) << sum / ctrsimpassed	<< "/usec, over " 
		     << std::setw( 5 ) << ctrsimpassed		<< " usecs. ("
		     << dirsimpassed * 100 / ctrsimpassed	<< "%)"
		     << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 	       	       += refctr->getvalue() )
	    ;
	int ctrnovpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "ref::ptr access,  w/ ref::counter, non-virtual: "
		     << std::setw( 5 ) << sum / ctrnovpassed	<< "/usec, over " 
		     << std::setw( 5 ) << ctrnovpassed		<< " usecs. ("
		     << dirnovpassed * 100 / ctrnovpassed	<< "%)"
		     << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += refctr->getvaluevirtual() )
	    ;
	int ctrvirpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "ref::ptr access,  w/ ref::counter, virtual:     " 
		     << std::setw( 5 ) << sum / ctrvirpassed  	<< "/usec, over " 
		     << std::setw( 5 ) << ctrvirpassed 		<< " usecs. ("
		     << dirvirpassed * 100 / ctrvirpassed 	<< "%)"
		     << std::endl;

	// Access object via ref::ptr/ref::dyn
	ref::ptr<ref::dyn<intobj> > 
	    			refdyn	= new ref::dyn<intobj>();
	refdyn->assign( 1 );
	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += refdyn->_value )
	    ;
	int dynsimpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "ref::ptr access,  w/ ref::dyn,     simple:      "
		     << std::setw( 5 ) << sum / dynsimpassed	<< "/usec, over " 
		     << std::setw( 5 ) << dynsimpassed 		<< " usecs. ("
		     << dirsimpassed * 100 / dynsimpassed	<< "%)"
		     << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += refdyn->getvalue() )
	    ;
	int dynnovpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "ref::ptr access,  w/ ref::dyn,     non-virtual: "
		     << std::setw( 5 ) << sum / dynnovpassed	<< "/usec, over " 
		     << std::setw( 5 ) << dynnovpassed	 	<< " usecs. ("
		     << dirnovpassed * 100 / dynnovpassed	<< "%)"
		     << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += refdyn->getvaluevirtual() )
	    ;
	int dynvirpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "ref::ptr access,  w/ ref::dyn,     virtual:     " 
		     << std::setw( 5 ) << sum / dynvirpassed  	<< "/usec, over " 
		     << std::setw( 5 ) << dynvirpassed 		<< " usecs. (" 
		     << dirvirpassed * 100 / dynvirpassed	<< "%)"
		     << std::endl;

	// Temporary reference to underlying object, via ref::ptr/ref::dyn
	intobj		       &rintobj	= *refdyn;
	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += rintobj.getvalue() )
	    ;
	int refsimpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "reference access, w/ ref::dyn,     simple:      "
		     << std::setw( 5 ) << sum / refsimpassed	<< "/usec, over " 
		     << std::setw( 5 ) << refsimpassed 		<< " usecs. ("
		     << dirsimpassed * 100 / refsimpassed	<< "%)"
		     << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += rintobj.getvalue() )
	    ;
	int refnovpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "reference access, w/ ref::dyn,     non-virtual: "
		     << std::setw( 5 ) << sum / refnovpassed	<< "/usec, over " 
		     << std::setw( 5 ) << refnovpassed	 	<< " usecs. ("
		     << dirnovpassed * 100 / refnovpassed	<< "%)"
		     << std::endl;

	for ( begin			= timevalnow()
	      , sum			= 0
	      ; sum < 1000000
	      ; sum 		       += rintobj.getvaluevirtual() )
	    ;
	int refvirpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "reference access, w/ ref::dyn,     virtual:     " 
		     << std::setw( 5 ) << sum / refvirpassed	<< "/usec, over " 
		     << std::setw( 5 ) << refvirpassed	 	<< " usecs. ("
		     << dirvirpassed * 100 / refvirpassed	<< "%)"
		     << std::endl;

	// Exercise std::iter_swap.  Affected greatly by implementing std::iter_swap in terms of
	// std::swap, if possible.
	{
	    ref::ptr<intobj>	obj	= new intobj;
	    std::vector<ref::ptr<intobj> >
		one( 1000, obj );
	    std::vector<ref::ptr<intobj> >
		two( 1000, obj );
	    assert.ISEQUAL( obj.__ref_getcnt(), (unsigned int)( 2001 ));
	    for ( begin			= timevalnow()
		 , sum			= 0
		 ; sum < 1000
		 ; sum 		       += 1 )
		std::swap_ranges( one.begin(), one.end(), two.begin() );
	}
	int rptswppassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "1M iter_swaps, w/ no ref::counter:              " 
		     << std::setw( 5 ) << sum * 1000 / rptswppassed	<< "/usec, over " 
		     << std::setw( 5 ) << rptswppassed		 	<< " usecs. (baseline)"
		     << std::endl;

	{
	    ref::ptr<rcintobj>	obj	= new rcintobj;
	    std::vector<ref::ptr<rcintobj> >
		one( 1000, obj );
	    std::vector<ref::ptr<rcintobj> >
		two( 1000, obj );
	    assert.ISEQUAL( obj.__ref_getcnt(), (unsigned int)( 2001 ));
	    for ( begin			= timevalnow()
		 , sum			= 0
		 ; sum < 1000
		 ; sum 		       += 1 )
		std::swap_ranges( one.begin(), one.end(), two.begin() );
	}
	int ctrswppassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "1M iter_swaps, w/    ref::counter:              " 
		     << std::setw( 5 ) << sum * 1000 / ctrswppassed	<< "/usec, over " 
		     << std::setw( 5 ) << ctrswppassed	 		<< " usecs. ("
		     << rptswppassed * 100 / ctrswppassed		<< "%)"
		     << std::endl;
	
	{
	    ref::ptr<ref::dyn<intobj> >	obj	= new ref::dyn<intobj>;
	    std::vector<ref::ptr<ref::dyn<intobj> > >
		one( 1000, obj );
	    std::vector<ref::ptr<ref::dyn<intobj> > >
		two( 1000, obj );
	    assert.ISEQUAL( obj.__ref_getcnt(), (unsigned int)( 2001 ));
	    for ( begin			= timevalnow()
		 , sum			= 0
		 ; sum < 1000
		 ; sum 		       += 1 )
		std::swap_ranges( one.begin(), one.end(), two.begin() );
	}
	int dynswppassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "1M iter_swaps, w/    ref::dyn:                  " 
		     << std::setw( 5 ) << sum * 1000 / dynswppassed	<< "/usec, over " 
		     << std::setw( 5 ) << dynswppassed	 		<< " usecs. ("
		     << rptswppassed * 100 / dynswppassed		<< "%)"
		     << std::endl;


	// Exercise mixture of std::swap (via std::rotate) and std::iter_swap (via
	// std::next_permutation).  Tried several algorithms; std::stable_sort,
	// std::random_shuffle.  Doesn't seem to be affected much by implementing std::iter_swap
	// in terms of std::swap; reason unknown.
	size_t		uniqsiz	= 0;
	std::map<int,int>
			distribution;

	int permute		= 0;				// approximate # of permutations of elements
	begin			= timevalnow();
	{
	    std::vector<ref::ptr<intobj> >
				perm( 1001 );
	    for ( size_t i = 0; i < perm.size(); permute += 2 * i, ++i ) {
		std::rotate( perm.begin(), perm.begin() + ( rand() % ( i + 1 )), perm.begin() + i );
		if ( ! ( i & 1 ))
		    perm[i]		= new intobj;		// even; assign a new intobj
		else
		    perm[i]		= perm[0];		// odd;  share a previously created intobj
	     // std::stable_sort( perm.begin(), perm.begin() + i + 1 );
		std::next_permutation( perm.begin(), perm.begin() + i + 1 );
	    }
	    std::set<ref::ptr<intobj> >
			uniq( perm.begin(), perm.end() );
	    uniqsiz			= uniq.size();
	    distribution.clear();
	    for ( std::set<ref::ptr<intobj> >::iterator si = uniq.begin()
		 ; si != uniq.end()
		 ; ++si ) {
		++distribution[si->__ref_getcnt()];
	    }
	}
	int	rptconpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "1M container operations, w/ no ref::counter:    " 
		     << std::setw( 5 ) << permute / rptconpassed<< "/usec, over "
		     << std::setw( 5 ) << rptconpassed	 	<< " usecs. (baseline).  "
//		     << std::setw( 5 ) << uniqsiz	 	<< " ref::ptrs, distribution: "
//		     << distribution				<< "(~"
//		     << std::setw( 5 ) << permute	 	<< " operations) "
		     << std::endl;

	begin			= timevalnow();
	{
	    std::vector<ref::ptr<rcintobj> >
				perm( 1001 );
	    for ( size_t i = 0; i < perm.size(); ++i ) {
		std::rotate( perm.begin(), perm.begin() + ( rand() % ( i + 1 )), perm.begin() + i );
		if ( ! ( i & 1 ))
		    perm[i]		= new rcintobj;		// even; assign a new intobj
		else
		    perm[i]		= perm[0];		// odd;  share a previously created intobj
		std::next_permutation( perm.begin(), perm.begin() + i + 1 );
	    }
	    std::set<ref::ptr<rcintobj> >
			uniq( perm.begin(), perm.end() );
	    uniqsiz			= uniq.size();
	    distribution.clear();
	    for ( std::set<ref::ptr<rcintobj> >::iterator si = uniq.begin()
		 ; si != uniq.end()
		 ; ++si ) {
		++distribution[si->__ref_getcnt()];
	    }
	}
	int	ctrconpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "1M container operations, w/    ref::counter:    " 
		     << std::setw( 5 ) << permute / ctrconpassed<< "/usec, over "
		     << std::setw( 5 ) << ctrconpassed	 	<< " usecs. ("
		     << rptconpassed * 100 / ctrconpassed	<< "%).  "
//		     << std::setw( 5 ) << uniqsiz	 	<< " ref::ptrs, distribution: "
//		     << distribution
		     << std::endl;

	begin			= timevalnow();
	{
	    std::vector<ref::ptr<ref::dyn<intobj> > >
				perm( 1001 );
	    for ( size_t i = 0; i < perm.size(); ++i ) {
		std::rotate( perm.begin(), perm.begin() + ( rand() % ( i + 1 )), perm.begin() + i );
		if ( ! ( i & 1 ))
		    perm[i]		= new ref::dyn<intobj>;	// even; assign a new intobj
		else
		    perm[i]		= perm[0];		// odd;  share a previously created intobj
		std::next_permutation( perm.begin(), perm.begin() + i + 1 );
	    }
	    std::set<ref::ptr<ref::dyn<intobj> > >
			uniq( perm.begin(), perm.end() );
	    uniqsiz			= uniq.size();
	    distribution.clear();
	    for ( std::set<ref::ptr<ref::dyn<intobj> > >::iterator si = uniq.begin()
		 ; si != uniq.end()
		 ; ++si ) {
		++distribution[si->__ref_getcnt()];
	    }
	}
	int	dynconpassed		= std::max( 1, duration( timevalnow(), begin ));
	assert.out() << "1M container operations, w/    ref::dyn:        " 
		     << std::setw( 5 ) << permute / dynconpassed<< "/usec, over "
		     << std::setw( 5 ) << dynconpassed	 	<< " usecs. ("
		     << rptconpassed * 100 / dynconpassed	<< "%).  "
//		     << std::setw( 5 ) << uniqsiz	 	<< " ref::ptrs, distribution: "
//		     << distribution
		     << std::endl;

    }

    CUT( ref_tests, ref_assign, "ref assign" ) {
	ref::ptr_tiny<int>	rt	= new int;
	ref::ptr_fast<int>	rf	= new int;

	ref::ptr_tiny<int>	rt1( rt );
	ref::ptr_fast<int>	rf1( rt );
	assert.ISEQUAL( rt1.get(), rf1.get() );
	assert.ISFALSE( rt1 != rf1);
	assert.ISTRUE( rt1 == rf1 );
	assert.ISFALSE( rf1 != rt1 );
	assert.ISTRUE( rf1 == rt1 );

	ref::ptr_tiny<int>	rt2( rf );
	ref::ptr_fast<int>	rf2( rf );
	assert.ISEQUAL( rt2.get(), rf2.get() );
	assert.ISFALSE( rt2 != rf2 );
	assert.ISTRUE( rt2 == rf2 );
	assert.ISFALSE( rf2 != rt2 );
	assert.ISTRUE( rf2 == rt2 );

	ref::ptr_tiny<int>	rt3	= new int; *rt3 = 3;
	ref::ptr_fast<int>	rf3	= new int; *rf3 = 33;
	int		       *it3	= rt3.get();
	int		       *if3	= rf3.get();

	ref::ptr_tiny<int>	rt4	= new int; *rt4 = 4;
	ref::ptr_fast<int>	rf4	= new int; *rf4 = 44;
	int		       *it4	= rt4.get();
	int		       *if4	= rf4.get();

	rt3.swap( rf4 );
	assert.ISEQUAL( *rt3, 44 );
	assert.ISEQUAL( *rf4, 3 );
	assert.ISEQUAL( rt3.get(), if4 );
	assert.ISEQUAL( rf4.get(), it3 );

	rf3.swap( rt4 );
	assert.ISEQUAL( *rf3, 4 );
	assert.ISEQUAL( *rt4, 33 );
	assert.ISEQUAL( rf3.get(), it4 );
	assert.ISEQUAL( rt4.get(), if3 );

	rf3				= rt4;
	assert.ISEQUAL( rf3.get(), rt4.get() );
	assert.ISEQUAL( *rf3, *rt4 );

    }

    CUT( ref_tests, ref_array, "ref::array" ) {
	
	const char	        s[]	= "Hello";
	ref::array<char,10>	a( s, s + sizeof s );
	assert.ISEQUAL( a.size(), unsigned( 10 ));
	assert.ISEQUAL( s[0], a[0] );
	ref::array<char,10>	b( a );
	assert.ISEQUAL( b[0], a[0] );

	assert.ISFALSE( a < b );
	assert.ISFALSE( b < a );

	const char	        t[]	= "Bye!";
	ref::array<char,10>	c( t, t + sizeof t );
	assert.ISFALSE( a < c );
	assert.ISTRUE(  c < a );
	
	typedef std::map<ref::array<char,10>, int>
	    			maparray_t;
	maparray_t		ma;

	const char	       *gettysburg
	    =   "Lincoln's Address at Gettysburg, 1863   "
	    "Fourscore and seven years ago our fathers brought forth on this"
	    " continent a new nation, conceived in liberty and dedicated"
	    " to the proposition that all men are created equal.   "
	    "Now we are engaged in a great civil war, testing whether that nation"
	    " or any nation so conceived and so dedicated can long"
	    " endure. We are met on a great battle field of that war. We have come"
	    " to dedicate a portion of that field, as a final"
	    " resting-place for those who here gave their lives that that nation"
	    " might live. It is altogether fitting and proper that we"
	    " should do this.   ";
	const char	       *gb;
	const char	       *ge;
	for ( gb 			= gettysburg
	      , ge 			= strchr( gb, ' ' )
	      ; ge
	      ; gb			= ge + 1
	      , ge 			= strchr( gb, ' ' )) {
	    if ( *gb == ' ' ) continue;
	    ma[ref::array<char,10>( gb, ge )]++;
	}

	for ( maparray_t::iterator
		  		mai	= ma.begin()
	     ; mai != ma.end()
	     ; ++mai ) {
	    std::ostringstream	oss;
			
	    oss << "0x";
	    for ( ref::array<char,10>::const_iterator
		      		ii	= mai->first.begin()
		 ; ii != mai->first.end()
		 ; ++ii ) {
		if ( std::isprint( *ii )) {
		    oss << ' ' << *ii;
		} else {
		    oss << std::setbase( 16 ) << std::setfill( '0' ) << std::left << std::setw( 2 )
			<< int( *ii )
			<< std::setbase(  0 ) << std::setfill( ' ' ) << std::internal;
		}
	    }
//	    std::cout << oss.str() << " x " << mai->second << std::endl;
	}
	assert.ISEQUAL( ma.size(), size_t( 79 ));		// number of distinct (up to 10 chars) words
    }
}

#endif // TEST
