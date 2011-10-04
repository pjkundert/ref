
// 
// ref.H		-- Reference Counting Pointer Framework
// 
// Copyright (C) 2004 Enbridge Inc.
//
// This file is part of the Enbridge REF Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#ifndef _INCLUDE_REF_H
#define _INCLUDE_REF_H 1

#include <algorithm>		// std::swap, etc.

#  if   defined( REF_PTR_DEREF_TEST )
#    include <stdexcept>
#    include <sstream>
#    include <string>
#  endif

namespace ref {

    // 
    // ref::ptr<T>		-- Reference counting pointer to dynamic T's.
    // ref::counter<T>		-- Reference counter interface for an object castable to (T *).
    // ref::dyn<T>		-- Class template ref::counter wrapper for dynamic objects
    // 
    //     If ref::ptr<T> is pointing to an object of type T derived from
    // ref::counter, then no dynamic ref::counter<T> object is created; the
    // object's own ref::counter methods are employed.
    // 
    //     For arbitrary dynamically allocated objects, a dynamic
    // ref::counter object is created, which is destroyed automatically
    // along with the object pointed to, when its reference count drops to
    // zero.
    // 
    //     All of ref::counter's methods are preceeded by __ref_, to avoid
    // colliding with the namespace of any object, to allow simple
    // inclusion in most object's base class list.
    // 
    // USAGE
    // 
    //     If you have control over definition of a class, adding a built-in 
    // ref::counter is easy: just add ref::counter<your_class> as a(nother) base class:
    // 
    //     class your_class 
    //         : public ref::counter<your_class> {
    //         virtual your_class *__ref_getptr()  { return this; }
    //         /* ... the rest of your_class's declaration ... */
    //     }
    // 
    //     If you do NOT have control over some class "object", but you want
    // to make a version with built-in reference counters:
    // 
    //     class object { /* ... */ ; };
    // 
    //     // Make an "object" having its own ref::counter; ref::ptr will use it..
    //     class rcobject
    //       : public object
    //       , public ref::counter<rcobject> {
    //         virtual rcobject *__ref_getptr()  { return this; } // dereference the object
    //         /* ... */
    //     };
    //     ref::ptr<rcobject>		r = new rcobject;
    // 
    //     // Add reference-counting and self-destruction to some other object
    //     ref::ptr<ref::dyn<object> >	o = new ref::dyn<object>;	
    // 
    //     // Take reference-counting parameters
    //     fun( ref::ptr<int> d ) {
    // 
    //	       // Assign 99 to the LAST call's object (if any)!
    //         static ref::ptr<int>	e;
    //	       if ( e )
    //             *e 	= 99;
    // 
    //         // Forget last call's object (will self-destruct
    //         // if no-one else holds it), and remember this new one
    //         // for next time!
    //         e	= d;				
    //     }
    // 
    //     ref::ptr<int>		a( new int );	// new int 'A' on heap, 'a' holds ref::counter of 1 to 'A'
    //     ref::ptr<int>		b;		// 'b's ref::counter is 0, and it's pointer ( *b, b->, ...) is 0.
    //     b		= new int;			// new int 'B' on heap; 'b's ref::counter is 1
    //     ref::ptr<int>		c;
    //     c 		= b;				// 'b' and 'c' share ref::counter of 2 to 'B'
    //     a		= c;				// 'A' freed; 'a', 'b' and 'c' share ref::counter of 3 to 'B'
    //     
    //     fun( a );					// fun's parameter 'd' now shares ref::counter of 4 to 'B'
    // 
    //     ref::ptr<rcobject>		e; 
    //     {
    //         ref::ptr<rcobject>	f = new rcobject;// 'f' uses <rcobject> object 'R's own ref::counter base class
    //         e	= f;				// 'R' ref::counter is now 2
    //     }						// 'R' ref::counter is now 1 (f passed out of scope)
    //     e		= 0;				// 'R' is freed.
    // 
    // 
    //     See ref-test.C, "ref::ptr demo code" for unit test confirming the above code.
    // 
    // ACKNOWLEDGEMENTS
    // 
    //     Randy Charles Morin	www.kbcafe.com
    //     Sandu Turcan		idlsoft@hotmail.com
    // 

    // 
    // ref::counter<T>
    // 
    //     Implements reference counting for a derived class 'T'.
    // 
    //     Triggers destructor on itself when reference count reaches zero.  This interface class
    // does nothing except keep a counter, and trigger destructor; the derived class should
    // implement a virtual destructor as appropriate to clean up any resources used by the derived
    // ref::counter class.
    // 
    //     This interface is safe to use for statically or dynamically allocated objects;
    // statically allocated T objects having a ref::counter<T> base class MUST never be assigned
    // to a ref::ptr<T>, and hence will not have their __ref_inc/__ref_dec methods called, and
    // won't be auto-destroyed!  It doesn't implement the (required) __ref_getptr, so derived
    // classes must implement it, thus providing the correct pointer conversion.
    // 
    template<class T>
    class counter {
    private:
	unsigned int		__ref_count;

    public:
	// 
	//  ref::counter<T>	-- Constructors, Default and Copy.
	// ~ref::counter<T>	-- Destructor (virtual).
	// 
	///     Interface to a reference counter for objects of type T.  The Copy constructor DOES
	/// NOT copy the original object's reference count into the new object's __ref_count!
	/// This would be used, for example, when an object includes ref::counter as one of its
	/// base classes.  Obviously, the new object is a copy of the old one, except that it has
	/// its own reference counter, which starts at zero!
	/// 
	///     If a class is derived from this, then it will be destructed when reference counter
	/// reaches zero; override this to destroy the associated object, if necessary.  Any
	/// object which includes ref::counter should provide a virtual destructor; otherwise, if
	/// it is of (say) type Derived, it won't be destroyed properly, if a ref::ptr<Base> holds
	/// the last reference to it.
	// 
				counter<T>()
	    throw()
				    : __ref_count( 0 )
	{
	    ;
	}
				counter<T>(
				    const counter<T>   & )	// ignored...
	    throw()
				    : __ref_count( 0 )
	{
	    ;
	}
	virtual		       ~counter<T>()
	{
	    ;
	}

	// 
	// __ref_getcnt		-- Return the number of references.
	// 
	///     In the simple case, we hold the number of references.  More complex reference
	/// counters may override, to provide different reference counting semantics.
	// 
	virtual unsigned int	__ref_getcnt()
	    const
	    throw()
	{
	    return __ref_count;
	}

	// 
	// __ref_inc		-- increment the usage count on the object
	// __ref_dec		-- decrement the usage count, and trigger auto-destruction
	// 
	///    Trigger destructor on object as soon as counter counts DOWN TO 0.  NOTE that these
	/// semantics are critically important -- since we never trigger when we detect that we ARE
	/// at zero, it is safe to use an object derived from ref::counter as a static or automatic
	/// variable (which never invokes either __ref_inc() or __ref_dec().  The object won't
	/// auto-destruct.
	/// 
	///     However, make certain you never use your non-dynamically-allocated object in the
	/// context of (say) a 'ref::ptr', because it WILL invoke '__ref_inc()' and 'ref_dec()',
	/// and will attempt to auto-destruct your object!
	/// 
	///     A caller can determine if the reference count has fallen to zero (and hence the
	/// object has been destroyed), by looking for a return value of 0 from __ref_dec().
	// 
	virtual unsigned int	__ref_inc()
	    throw()
	{
	    return ++__ref_count;
	}
	virtual unsigned int	__ref_dec()
	{
	    if ( --__ref_count )
		return __ref_count;
	    delete this;
	    return 0;
	}

	// 
	// __ref_getptr
	// 
	///     Return the underlying reference-counted object.  For objects derived from
	/// ref::counter (in other words, objects that contain their own reference counter),
	/// __ref_getptr normally simply returns the object itself (the 'this' pointer).  Each
	/// class T deriving itself from ref::counter<T> must provide an overridden __ref_getptr()
	/// returning itself.
	/// 
	///     Override to implement some other interface to the associated type T (eg. to return
	/// some other object).  For example, for objects that do not contain their own counter
	/// (see ref::count_other, below), this would return the pointer to the underlying shared
	/// object.
	/// 
	///     Although this method is (usually) trivial, we can't make it 'const', because it
	/// needs to return a pointer to a mutable object.
	/// 
	virtual T	       *__ref_getptr()
	    = 0;
    };


    // 
    // ref::count_other<T>
    // 
    ///     A dynamically allocated reference counter for an arbitrary 'new'-allocated T object.
    /// The object is 'delete's when its reference count falls to zero.  Do not create one of
    /// these unless you've actually got an object (ie. not a 0 pointer!)
    /// 
    ///     If the "type" of any dynamically allocated object is going to be lost (eg. a (Derived
    /// *) is going to be assigned to a ref::ptr<Base> ), then make certain that virtual
    /// destructors Derived::~Derived and Base::~Base are defined (just as for any classes where
    /// dynamically allocated instances are passed around "anonymously", using base-class
    /// pointers.
    /// 
    /// WARNING
    /// 
    ///     ONLY allocate ref::count_other<T> dynamically, AND use only for objects dynamically
    /// allocated, using 'new'!  Static allocation of EITHER ref::count_other<T> OR the underlying
    /// T object, will corrupt heap upon destruction!
    /// 
    ///     This object does NOT check for multiple references to the same dynamically allocated
    /// object, and it probably should.  It is quite easy to forget to change every instance where
    /// a dynamically allocated T* pointer is used directly instead of using the ref::ptr<T> /
    /// ref::count_other<T> version.  However, checking for re-assignment of an object implies
    /// using a std::map<void*,...> to remember all assigned pointers, which could hold the
    /// ref::count_other<T> directly, directly instead of dynamically allocating it for each
    /// object!  The tradeof is a std::map<void*,...> traversal on assignment and destruction,
    /// instead of (or in addition to) a ref::count_other<T> new/delete.  The safety improvements
    /// might be worth it, at least in a "debugging" mode.
    /// 
    ///     In practice, however, it seems that it is actually quite simple to avoid ever accidentally
    /// using dynamically allocated T* pointers, by assigning them directly to a ref::ptr<T> upon
    /// creation.
    // 
    template<class T>
    class count_other
	: public counter<T> {
    private:
	T		       *__ref_other;

    public:
				count_other<T>(
				    T  		       *other )
	    throw()
				    : __ref_other( other )
	{	
	    ;
	}

	// 
	// ~ref::count_other
	// 
	///     Reference count has fallen to zero; base class ref::counter is self-destructing.
	/// Since we know that our object is dynamic, blow away the reference-counted object, too.
	/// No need to test for existance; ref::count_other<T>'s are only constructed with a
	/// non-zero <T*>!
	virtual		       ~count_other<T>()
	{
#if defined( REF_PTR_DEREF_TEST )
	    if ( ! __ref_other ) {
		std::ostringstream	error;
		error << "ref::count_other " << this 
		      << " is attempting to auto-destruct and clean up a non-existant object!";
		throw std::logic_error( error.str() );
	    }
#endif

	    delete __ref_other;

#if defined( REF_PTR_DEREF_TEST )
	    __ref_other	= 0;
#endif
	}
	
	// 
	// __ref_getptr
	// 
	/// Return the underlying reference-counted object.  In this case, it's the "__ref_other"
	/// object.
	virtual T	       *__ref_getptr()
	    throw()
	{
	    return __ref_other;
	}
    };

    // 
    // ref::count_adapter<Base,Derived>
    // 
    //     A ref::counter<> adapter, to allow a ref::ptr<Base> to share the
    // object counted by a ref::ptr<Derived>.  Keeps track of how many
    // references to ITSELF exist, and self-destructs when its own count
    // returns to zero.
    // 
    //     Slip in between a ref::ptr<Base> and a ref::counter<Derived> from
    // a ref::ptr<Derived>, and it allows a ref::ptr<Base> to share and
    // reference-count the object held by the ref::ptr<Derived>.  Proper
    // pointer conversions are performed each time the (Derived *) object
    // is accessed through the ref::ptr<Base> as a (Base *).
    // 
    //     For example:
    // 
    //	class Base {
    //	public:
    //	    int					base;
    //	};
    //	class Derived
    //	    : public Base {
    //	public:
    //	    int					derived;
    //	};
    //	class Derived1
    //	    : public Derived {
    //	public:
    //	    int					derived1;
    //	};
    //      class Other1 {
    //	public:
    //	    int					other1;	
    //	};
    //	class Derived2
    //	    : public Other1
    //	    , public Derived1 {
    //	public:
    //	    int					derived2;
    //	};
    //
    //	ref::ptr<Derived2>			d2	= new Derived2;
    //	d2->derived2		= 10002;
    //	d2->derived1		= 10001;
    //	d2->derived		= 10000;
    //	d2->other1		= 20001;
    //	d2->base		=  9999;
    //	ref::ptr<Derived1>			d1	= d2;
    //	assert.ISEQUAL( d2->derived1, 10001 );
    //	assert.ISEQUAL( d1->derived1, 10001 );
    // 
    template<class Base, class Derived>
    class count_adapter
	: public counter<Base> {
	counter<Derived>       *__ref_actual;

    public:
				count_adapter<Base,Derived>(
				    counter<Derived>   *actual )
	    throw()
				    : __ref_actual( actual )
	{	
	    ;
	}

	// 
	// ~ref::count_adapter<Base,Derived>
	// 
	///    Reference count has fallen to zero; class ref::counter is self-destructing.  We
	/// have nothing else to destroy; we've already reduced the reference count of the
	/// "adapted" object, in __ref_dec.
	// 
	virtual		       ~count_adapter<Base,Derived>()
	{
#if defined( REF_PTR_DEREF_TEST )
	    if ( ! __ref_actual ) {
		std::ostringstream	error;
		error << "ref::count_adapter" << this << " is attempting to auto-destruct and clean up a non-existant object!";
		throw std::logic_error( error.str() );
	    }
	    __ref_actual	= 0;
#endif
	}

	// 
	// __ref_getcnt
	// 
	/// The actual counter we're interested in, is the "adapted" object's counter -- not the
	/// counter of how many of our brethren happen to be sharing this adapter!
	// 
	virtual unsigned int	__ref_getcnt()
	    const
	    throw()
	{
	    return __ref_actual->__ref_getcnt();
	}

	// 
	// __ref_inc
	// __ref_dec
	//
	/// Adjust our reference count, and that of the "adapted" object.  When our reference
	/// count is about to reach 0, self-destruct (clean up this ref::count_adapter only)!  The
	/// __ref_actual->ref_dec() will have destroyed the underlying "adapted" object, as
	/// appropriate -- we may not be the only entity referencing the "adapted" object...
	virtual unsigned int	__ref_inc()
	    throw()
	{
	    ref::counter<Base>::__ref_inc();		// keep track of references to this ref::count_adapter()
	    return __ref_actual->__ref_inc();
	}
	virtual unsigned int	__ref_dec()
	{
	    unsigned int	remaining;
	    remaining	= __ref_actual->__ref_dec();
	    ref::counter<Base>::__ref_dec();		// triggers destructor when no more refs to this ref::count_adapter
	    return remaining;
	}

	// 
	// ref::count_adapter<Base,Derived>::__ref_getptr
	// 
	//     This is where the conversion magic happens.
	// 
	//     Return the underlying reference-counted actual (Derived *) object pointer,
	// converted to the desired (Base *) object pointer using the implicit conversion.  This
	// also works for converting (T *) to (const T *), so ref::ptr<const T> can share an
	// underlying ref::ptr<T> object.
	// 
	//     This is also where conversion type checking happens.  If there are no valid
	// conversions from (Derived *) to (Base *), the following return method will fail to
	// instantiate.
	// 
        virtual Base	       *__ref_getptr()
	    throw()
	{
	    return __ref_actual->__ref_getptr();
	}
    };

    // 
    // ref::ptr_tiny<T>		-- 1 x sizeof( T * ), but virtual method invoked on dereference
    // ref::ptr_fast<T>		-- 2 x sizeof( T * ), but simple pointer dereference
    // ref::ptr<T>		-- ref::ptr_tiny<T>, unless REF_PTR_DEREF_FAST defined!
    // 
    ///     Reference count a dynamically allocated object of type T (0 pointer is OK).
    /// Automatically delete T when its ref::counter object reaches 0.
    /// 
    ///     Very efficient implementation; no virtual methods, and a single pointer; same size as
    /// a normal pointer, so no extra overhead (other than incrementing the referenced object's
    /// ref::counter) when passed by value.
    /// 
    ///     If class T is derived from ref::counter, then uses the object's own ref::counter.
    /// Otherwise, dynamically allocate a ref::count_other<T> object to do the counting.
    /// 
    ///     ref::ptr<T> is designed to be especially efficient with "empty" (0) pointers.  No
    /// matter what conversion is specified, or whether the type specified implements
    /// ref::counter<T> or not, only a 0 pointer is stored.  Only when a non-zero (T *) pointer is
    /// assigned, is there any storage or processing overhead required to copy the ref::ptr<T>.
    template <typename T> class ptr_fast;
    template <typename T>
    class ptr_tiny {
    protected:
	// 
	// The shared ref::counter object.  Either a ref::count_other<T>
	// managing reference counts to an arbitrary object of type T, OR
	// any object derived from ref::counter<T> itself!
	// 
	// This same ref::counter instance is shared by ALL instances of
	// ref::ptr<T> that reference the same <T>.  Therefore, if you have
	// just a FEW <T> objects shared by few ref::ptr<T>, then it is not
	// critical that <T> be derived from ref::counter<T>.  However, if
	// you plan to have MANY objects referenced by MANY ref::counters,
	// then it is probably wise to derive your <T> objects from
	// ref::counter<T>, or use ref::dyn<T> to wrap your T objects with a
	// ref::counter.
	// 
	ref::counter<T>        *__ref_counter;

	// 
	// __ref_assign( ref::counter<T> * )	-- Selected if object implements ref::counter.  Cannot be const (we change counter)
	// __ref_assign( void * )		-- Selected if object does NOT implement its own ref::counter
	// __ref_assign( const void * )	-- Ditto, for const objects
	// 
	///     If an object is derived from ref::counter, use the object's own reference counter.
	/// The compiler will select which one of these methods is used by the Constructor, from
	/// the type T.  The (const void *) version will ONLY ever be triggered if T is already
	/// const; no need to add a redundant const to the static_cast<T*>.  Ensure that we never
	/// create a ref::count_other<> unless we have a non-0 pointee!
	ref::counter<T>	       *__ref_assign(
				    counter<T>         *pointee )
	{
	    return pointee;
	}
	ref::counter<T>	       *__ref_assign(
				    void	       *pointee )
	{
	    return ( pointee
		     ? new ref::count_other<T>( static_cast<T*>( pointee ))
		     : 0 );
	}
	ref::counter<T>	       *__ref_assign(
			            const void	       *pointee )
	{
	    return ( pointee
		     ? new ref::count_other<T>( static_cast<T*>( pointee ))
		     : 0 );
	}

    public:
	// 
	// ref::ptr_tiny<T>::swap( ref::ptr_tiny<T> & )
	// ref::ptr_tiny<T>::swap( ref::ptr_fase<T> & )
	// 
	///     Efficiently swap two identically typed ref::ptr<T>'s.  No need to manipulate
	/// either object's reference counter.  We MUST use the ptr_fast::swap method to implement
	/// swapping with ptr_tiny, because it knows about special ptr_fast implementation
	/// details!
	/// 
	void			swap(
				    ptr_tiny<T>        &rhs )
	    throw()
	{
	    std::swap( __ref_counter, rhs.__ref_counter );
	}
	void			swap(
				    ptr_fast<T>        &rhs )
	    throw()
	{
	    rhs.swap( *this );
	}

	//
	// __ref_getcnt
	// __ref_getcounter
	//
	///     Return the reference count of the underlying object (if any). Uses the actual
	/// ref::counter being used to count references to the object.
	//
	unsigned int		__ref_getcnt()
	    const
	    throw()
	{
	    if ( __ref_counter )
		return __ref_counter->__ref_getcnt();
	    return 0;
	}
	counter<T>	       *__ref_getcounter()
	    const
	    throw()
	{
	    return __ref_counter;
	}

	// 
	// ref::ptr<T>( ref::ptr<T> )
	// 
	///     Copy constructor for referencing another ref::ptr<T>'s object.  Shares the donor
	/// ref::ptr<T>'s ref::counter, and increments its usage count.
	// 
	// USAGE
	// 
	//     Initialization from a ref::ptr<T> at creation.
	// 
	//         ref::ptr<T>		a;
	//         ref::ptr<T>		b( a );
	// 	   	   
	//         void fun( ref::ptr<T>     c ) { ... }
	//         ...
	//         fun( a )
	// 
	//     And on some architectures, the ternary expression:
	// 
	//         ( bool_expr ? a : ref::ptr<T>( 0 ))
	// 
	// results in an extra copy operation, effectively:
	// 
	//         ( bool_expr ? ref::ptr<T>( a ) : ref::ptr<T>( 0 ))
	// 
				ptr_tiny<T>(
				    const ptr_tiny<T>  &rhs )
	    throw()
	{
	    __ref_counter		= rhs.__ref_counter;
	    if ( __ref_counter )
		__ref_counter->__ref_inc();
	}

	// 
	// ref::ptr<Base>( ref::ptr<Derived> )
	// 
	///     Class-heirarchy sensitive constructor.  Will result in compile time error if
	/// (Derived *) doesn't have a default conversion to (Base *).
	/// 
	///     If there are no conversions from (Derived *) to (T *), then the
	/// ref::count_adapter<T,Derived> template below will fail to instantiate.  However, we
	/// must also do an additional type conversion check here, as some compilers do not print
	/// a sufficient back trace to isolate exactly which statement is causing the template
	/// instantiation!  We'll make it optional, to speed up compilation in production code...
	template<class Derived>	ptr_tiny<T>(
				    const ptr_tiny<Derived> &rhs )
	{
#if defined( REF_PTR_DEREF_TEST )
	    // Check type conversion, and do nothing (avoiding an unused
	    // variable warning)
	    T 		       *check	= (Derived *)0;
	    if ( check ) { ; };
#endif
	    if ( rhs.__ref_getcounter() ) {
		__ref_counter		= new count_adapter<T,Derived>( rhs.__ref_getcounter() );
		__ref_counter->__ref_inc();
	    } else
		__ref_counter		= 0;
	}

	// 
	// ref::ptr<T>( T* )		-- Assignment (and default) constructor
	// 
	///     Assignment constructor for new, dynamically allocated T* objects.  If a non-zero
	/// object pointer provided, assign the object pointer, and increment the reference count.
	/// 
	///     Ensures that the __ref_counter is initialized before __ref_assign() is used, to
	/// preserve __ref_assign()'s ability to do something with a prior ref::counter<T> before
	/// assigning a new one.  Initially, the __ref_counter is initialised to 0 (empty).
	// 
	// USAGE
	// 
	//     Initialization from a T* at creation.
	// 
	//         ref::ptr<T>		a( new T );
	//         ref::ptr<T>		a	= new T;
	// 
				ptr_tiny<T>(
				    T	       	       *pointee 	= 0 )
	{
	    __ref_counter		= __ref_assign( pointee );
	    if ( __ref_counter )
		__ref_counter->__ref_inc();
	}

	/// Destroy a ref::ptr<T>, and decrement the __ref_counter object pointed to.  If it's a
	/// ref::count_other object we created, it will destroy itself, and the arbitrary object
	/// pointed to.  It it's a natively ref::counter derived object, then it will destroy
	/// itself.
			       ~ptr_tiny<T>()
	{
	    if ( __ref_counter )
		__ref_counter->__ref_dec();
	}

	// 
	// ref::ptr<T>::get			-- Interface to obtain pointer to underlying ref-counted object (if any)
	// 
	///     Return the object being counted (if any).  Used by ref::ptr, in cases where it is
	/// unsure if a __ref_counter has been assigned.  Any code that is certain that a
	/// __ref_counter exists may use ref::counter<T>::__ref_getptr() directly, instead, to
	/// avoid the test.
	T		       *get()
	    const
	    throw()
	{
	    if ( __ref_counter )
		return __ref_counter->__ref_getptr();
	    return 0;
	}

	// 
	// ref::ptr<T> = (T *)			-- assign arbitrary objects; need a counter object created
	// ref::ptr<T>.reset(T *)		-- same as operator=(T *); for compatibility with boost::shared_ptr
	// ref::ptr<T> = ref::counter<T>*	-- derived from ref::counter<T>; have their own counter
	// ref::ptr<T> = ref::ptr<T>		-- assignment from another ref::ptr<T>; reference RHS counter
	// 
	//     Assign a pointer to a new dynamic object (or 0 is OK).  We need to decrement the
	// old __ref_counter, and then forget about it; if it didn't decrement to zero, then some
	// other ref::ptr must have a copy of it.  Then, create a new ref::count_other just like
	// in the constructor.
	// 
	// NOTE: This operator is not strictly necessary; if not present, then given:
	// 
	//      T		       *t
	//      ref::ptr<T>		a;
	// 
	// the expression:
	// 
	// 	    a		= t;
	// 
	// is simply implemented by the C++ compiler as:
	// 
	//      {
	// 	    ref::ptr<T>		tmp( t );	// creates a ref::counter<T>, increments its counter
	//          a		= tmp;			// a reuses tmp's ref::counter<T>, increments its counter
	//      }					// tmp destroyed, decrements the shared ref::counter<T>.
	// 
	// Thus, this assignment operator simply eliminates the creation of a temporary, and
	// couple of indirect increment/decrement operations.  Over many accesses, this would add
	// up...
	// 
	// USAGE
	// 
	//     Initializing with a new dynamically allocated object.
	// 
	//         ref::ptr<int>     r;			// OK, no referenced object
	//         r = new int;				// OK, reference the new object
	//         r = new int;				// OK, blow away the old int, and reference the new one
	//         r = 0;				// OK, blow away the new one, too!
	// 
	// WARNING
	// 
	//     Ensure that the same dynamically allocated object T* is NEVER assigned to multiple
	// ref::ptr<T>'s!  Doing so will ensure the destruction of your program's heap.
	// 
	//        {
	//            int  	       *p	= new int;
	//            ref::ptr<int>	q	= p;
	//            ref::ptr<int>	r	= p;	// Bang.  You're dead.
	//        } 
	//        // Actually, you're dead right here, when q
	//        // and r go out of scope, and they both delete *p
	// 
	//     Just assume a policy of only assigning ref::ptr<T> managed objects right into a
	// ref::ptr<T>:
	//     
	//        {
	//            ref::ptr<int>	p	= new int;
	//            ref::ptr<int>	q	= p;
	//            ref::ptr<int>	r	= p;	// A-OK!  p, q and r are one big, happy clone family.
	//        }
	//        // They all (and their one shared dynamically allocated int) blink out of existance at once.
	// 
	///    Assume control over a (dynamically allocated or 0) T* pointer.  To handle
	/// self-assignment (where the old and new __ref_counter points to the same
	/// ref::counter<T> object), we must decrement the original after incrementing the new
	/// counter.  This will occur any time both ref::ptr<T>'s reference-count the same object.
	/// The object would disappear as soon as the __ref_dec() is invoked, and then the
	/// __ref_inc() would be operating on a delete'd object...
	ptr_tiny<T>	       &operator=(
				    T		       *rhs )
	{
	    counter<T>         *original = __ref_counter;

	    // If it's an arbitrary object, create a new reference counter
	    // object for it which will destroy itself and the object.  If
	    // it's derived from ref::counter, then use it as its own
	    // ref::counter object.  The compiler will use the correct
	    // version of __ref_assign(), depending on the type T.
	    __ref_counter		= __ref_assign( rhs );
	    if ( __ref_counter )
		__ref_counter->__ref_inc();
	    if ( original )
		original->__ref_dec();

	    return *this;
	}
	void			reset(
				    T		       *rhs	= 0 )
	{
	    *this 			= rhs;			// Just implement in terms of operator=, above
	}

	// 
	// ref::ptr<T> = ref::ptr<T>
	// 
	//     ref::ptr<int> q( new int );
	//     ref::ptr<int> r( new int );
	//
	// USAGE
	// 
	//     q = r; 		// blow away q's original int and reference r's
	//     r = 0;		// r forgets about it
	//     r = q;		// it remembers it again.
	//     q = new int;	// q forgets about it
	//     r = q;		// r's original int is deleted; both reference newly allocated one
	//     q = 0;		// q forgets newly allocate one
	//     r = 0;		// r forgets; the newly allocated int is deleted, too
	// 
	///     Copy a ref::ptr.  Forget whatever we're already pointing to, and reference the new
	/// object.  This just references the object's __ref_counter and increments it.  Thus,
	/// when the original ref::ptr is destructed, it will decrement the reference count on
	/// it's __ref_counter, and the object will not be destroyed, because we have incremented
	/// the count.
	/// 
	///     To handle self-assignment, we must increment the new counter, before decrementing the
	/// original...
	/// 
	///     This is not strictly necessary, but it avoids the creation of a temporary followed
	/// by the use of the Constructor ptr_type<T>( const ptr_tiny<T>& )
	ptr_tiny<T>            &operator=(
				    const ptr_tiny<T>  &rhs )
	{
	    if ( rhs.__ref_counter )
		rhs.__ref_counter->__ref_inc();				// increment the ref count on the new object
	    if ( __ref_counter )
		__ref_counter->__ref_dec();				// decrement the ref count on the old object
	    __ref_counter			= rhs.__ref_counter;	// and remember the new object
	    return *this;
	}

	// 
	// ref::ptr<Base> = ref::ptr<Derived>
	// 
	///     Avoids creation of a temporary and use of the templated constructor
	/// ref::ptr<T>( ref::ptr_tiny<Derived>& ).  This is not strictly necessary,
	/// but certain architectures don't destruct the temporary 'til end of scope,
	/// making testing difficult (reference counts get incremented twice on the
	/// assignment, and don't get decremented 'til the temporary destructs at
	/// end of scope, instead of being correct right after the assignment).
	/// 
	template<class Derived>
	ptr_tiny<T>	       &operator=(
				    const ptr_tiny<Derived> &rhs )
	{
#if defined( REF_PTR_DEREF_TEST )
	    // Check type conversion, and do nothing (avoiding an unused
	    // variable warning)
	    T 		       *check	= (Derived *)0;
	    if ( check ) { ; };
#endif
	    
	    // There is no chance that we are self-assigning (assignee is of a different type!)
	    // However, we COULD be indirectly pointing to the same original object!  So, we
	    // cannot safely pre-decrement the reference counter we hold.
	    counter<T>         *original = __ref_counter;

	    if ( rhs.__ref_getcounter() ) {
		__ref_counter		= new count_adapter<T,Derived>( rhs.__ref_getcounter() );
		__ref_counter->__ref_inc();
	    } else
		__ref_counter		= 0;

	    // Now it is safe to reduce our original counter.
	    if ( original )
		original->__ref_dec();
	    return *this;
	}

	// 
	// operator bool		-- Indicator of whether there is a reference-counted object assigned
	// ref::ptr<T> == T*	 	-- Test a pointer to the reference counted object
	// ref::ptr<T> == ref::ptr<T>	-- Comparison of ref::counted objects compares ptrs to underlying object
	// ref::ptr<T>  < T*		--   ; useful for seeing if a ref::ptr<T> contains a certain T*
	// ref::ptr<T>  < ref::ptr<T>	--   ; useful when using std:: containers of ref::ptr<T>'s
	// ref::ptr<T>->		-- indirection; will return 0 (just like a real pointer) and fail
	// *ref::ptr<T>			-- dereference; will fail if 0 (just like a real pointer), but we'll throw instead of segfault.
	// 
	//     Getting a pointer to the reference counted object, using an
	// ref::counter<T>::operator T*() method, is NOT allowed (hence it is not implemented);
	// this would make it very, very easy to accidentally (and incorrectly) pass around direct
	// copies of the pointer to a reference counted object.  The copy constructor is used
	// instead, to create a new ref::ptr<T>, which shares a reference to (and increments the
	// underlying ref::counter of) the original object.
	// 
	//     We provide an operator bool() equivalent, and operator !/==/!=, to do the typical
	// kind of NULL pointer checking a user would do.  Optionally, if comparison of ref::ptr's
	// by underlying pointer is desired, it can be enabled via REF_PTR_COMPARE.  This is
	// required, if (for example), you want to sort containers of ref::ptr<T> by the address
	// of the ref-counted object, by default.  It is suggested that you define an appropriate
	// comparison operator for each specific ref::ptr<T> type, which accesses the underlying
	// pointer address via ref::ptr<T>::get(), and compares them directly.
	// 
	//     If the __ref_counter IS the object (because it's derived from ref::counter<T>),
	// it'll just return a pointer to itself.  Otherwise, if the __ref_counter is a
	// ref::count_other<T>, then it will return a pointer to it's reference counted object.
	// 
	//     If no object is presently being reference-counted we'll be returning 0.
	// 
	//     We do NOT want to have implicit conversion to a numeric type, as this would allow
	// promotion to ANY numeric type, and would thus (for example) provide an unintended
	// "default" sorting for containers, allow for assignments from ref::ptr<T> to numeric
	// types, etc.  Automatic promotion to (void *) would be almost perfect, except then
	// assignment of (nonsensical) values to void* pointers would proceed.  No, we will return
	// a zero or non-zero pointer-to-member (thanks to boost::shared_ptr for this solution).
	// 
	typedef counter<T> * ref::ptr_tiny<T>::*
				unspecified_bool_type;
	                        operator unspecified_bool_type()
	    const
	    throw()
	{
	    return __ref_counter == 0 ? 0 : &ref::ptr_tiny<T>::__ref_counter;
	}			
	bool			operator!()
	    const
	    throw()
	{
	    return __ref_counter == 0;
	}			

	bool			operator==(
				    const T	       *rhs )
	    const
	    throw()
	{
	    return ( get() == rhs );
	}
	bool			operator!=(
				    const T    	       *rhs )
	    const
	    throw()
	{
	    return ( get() != rhs );
	}
	bool			operator==(
			            const ref::ptr_tiny<T>
				    		       &rhs )
	    const
	    throw()
	{
	    return ( get() == rhs.get() );
	}
	bool			operator!=(
				    const ref::ptr_tiny<T>
				    		       &rhs )
	    const
	    throw()
	{
	    return ( get() != rhs.get() );
	}
#if defined( REF_PTR_COMPARE )
	bool			operator<(
			            const T    	       *rhs )
	    const
	    throw()
	{
	    return ( get() < rhs );
	}
	bool			operator<(
				    const ref::ptr_tiny<T>
				    		       &rhs )	
	    const
	    throw()
	{
	    return ( get() < rhs.get() );
	}
#endif
	// 
	// T *ref::ptr<T>::operator->
	// T &ref::ptr<T>::operator*
	// 
	//     Dereference a ref-counted pointer.  The const versions still return the underlying
	// object, but would be used (for example) in cases where a const ref::ptr<T> object is
	// used as a key in a std::map<> or std::set<>; the ref::ptr<T> is const, but the
	// underlying object may or may not be.  NOTE: it obviously doesn't make sense to have a
	// const reference counter!  Thus, things like:
	// 
	//     ref::ptr<const ref::dyn<int> >
	// or
	//     class X : ref::counter<X> { ... };
	//     ref::ptr<const X>
	// 
	// are nonsensical; they can't possibly be reference counted directly!  However, they can
	// be reference counted indirectly using a ref::count_adapter<const X>, which shares the
	// underlying ref::counter<X> object's counter...
	// 
	T		       *operator->()
	    const
#if ! defined( REF_PTR_DEREF_TEST )
	    throw()
#endif
	{
	    T		       *pointer;
	    if ( ! __ref_counter ) {
#if defined( REF_PTR_DEREF_TEST )
		std::ostringstream	error;
		error << "T *ref::ptr<T>::operator-> " << this 
		      << " is attempting to dereference, but no ref-counted object has been assigned! (probably use of 0 pointer)";
		throw std::logic_error( error.str() );
#endif
		pointer			= 0;
	    } else {
		pointer			= __ref_counter->__ref_getptr();
#if defined( REF_PTR_DEREF_TEST )
		if ( ! pointer ) {
		    std::ostringstream	error;
		    error << "T *ref::ptr<T>::operator-> " << this 
			  << " is attempting to dereference, ref-counter didn't reference any object! (probably ref::ptr implementation error)";
		    throw std::logic_error( error.str() );
		}
#endif
	    }

	    return pointer;
	}

	T		       &operator*()
	    const
#if ! defined( REF_PTR_DEREF_TEST )
	    throw()
#endif
	{
	    T		       *pointer;
	    if ( ! __ref_counter ) {
#if defined( REF_PTR_DEREF_TEST )
		std::ostringstream	error;
		error << "T &ref::ptr<T>::operator*  " << this 
		      << " is attempting to dereference, but no ref-counted object has been assigned! (probably use of 0 pointer)";
		throw std::logic_error( error.str() );
#endif
		pointer			= 0;
	    } else {
		pointer			= __ref_counter->__ref_getptr();
#if defined( REF_PTR_DEREF_TEST )
		if ( ! pointer ) {
		    std::ostringstream	error;
		    error << "T &ref::ptr<T>::operator*  " << this 
			  << " is attempting to dereference, ref-counter didn't reference any object! (probably ref::ptr implementation error)";
		    throw std::logic_error( error.str() );
		}
#endif
	    }

	    return *pointer;
	}
    };


    // 
    // ref::ptr_fast<T>
    // 
    /// A ref::ptr_tiny<T> with a cached (T *).
    // 
    template <class T>
    class ptr_fast
    : public ptr_tiny<T> {

	T		       *_cachedptr;

    public:

	// 
	// ref::ptr_fast<T>::swap( ref::ptr_fast<T> & )
	// 
	//     Efficiently swap two identically typed ref::ptr_fast<T>'s.  Swaps the underlying
	// ptr_tiny<T> bits, and our _cachedptr.
	// 
	void			swap(
				    ptr_fast<T>        &rhs )
	    throw()
	{
	    ptr_tiny<T>::swap( rhs );
	    std::swap( _cachedptr, rhs._cachedptr );
	}
	void			swap(
				    ptr_tiny<T>        &rhs )
	    throw()
	{
	    ptr_tiny<T>::swap( rhs );
	    _cachedptr			= ptr_tiny<T>::get();
	}

	// 
	// Constructors		-- recognize objects, from most to least likely a ref::ptr*<T>
	// 
	// ref::ptr_fast<T>( const ref::ptr_fast<T> & )	-- use the already cached T *
	// ref::ptr_fast<T>( const ref::ptr_tiny<T> & )	-- cache the T *
	// 
				ptr_fast<T>(
				    const ptr_fast<T>  &rhs )
	    throw()
				    : ptr_tiny<T>( rhs )
	{
	    _cachedptr		= rhs._cachedptr;
	}
				ptr_fast<T>(
				    const ptr_tiny<T>  &rhs )
	    throw()
				    : ptr_tiny<T>( rhs )
	{
	    _cachedptr		= ptr_tiny<T>::get();
	}

	// 
	// ref::ptr_fast<T>( ref::ptr_tiny<Derived> )	-- Class heirarchy sensitive converting constructor
	// 
	template<class Derived>	ptr_fast<T>(
				    const ptr_tiny<Derived> &rhs )
				    : ptr_tiny<T>( rhs )
	{
	    _cachedptr		= ptr_tiny<T>::get();
	}

	// 
	// ref::ptr_fast<T>( T * )			-- Arbitrary T * pointer (assume it is dynamically allocated)
	// 
				ptr_fast<T>(
				    T	       	       *pointee 	= 0 )
				    : ptr_tiny<T>( pointee )
	{
	    _cachedptr		= ptr_tiny<T>::get();
	}

	// 
	// Destructor		-- Nothing.  All the magic is in ~ref::ptr_tiny
	// 
	//     Invalidates the _cachedptr, if we are running in
	// REF_PTR_DEREF_TEST mode.
	// 
			       ~ptr_fast<T>()
	{
#if defined( REF_PTR_DEREF_TEST )
	    _cachedptr		= 0;
#endif
	}

	// 
	// Override all methods that use get(), or could change _cachedptr, to use _cachedptr
	// instead.  If a ref::ptr_fast ever "forgets" that it is really a ref::ptr_fast, and is
	// used as a ref::ptr_tiny, that is OK; it will simply forget about using its cached
	// pointer...

	// 
	// ref::ptr<T>::get			-- Interface to obtain pointer to underlying ref-counted object (if any)
	// 
	///     Return the object being counted (if any).  Always simply returns the cached
	/// pointer.
	///
	T		       *get()
	    const
	    throw()
	{
	    return _cachedptr;
	}


	// 
	// ref::ptr_fast = ...		-- Assigning new value.  Update _cachedptr
	// ref::ptr_fast.reset()	-- Assignment, compatible with boost::shared_ptr
	// 
	ref::ptr_fast<T>       &operator=(
				    T		       *rhs )
	{
	    ref::ptr_tiny<T>::operator=( rhs );
	    _cachedptr			= ref::ptr_tiny<T>::get();
	    return *this;
	}
	void			reset(
				    T		       *rhs	= 0 )
	{
	    *this 			= rhs;				// Just implement in terms of operator=, above
	}
	ref::ptr_fast<T>       &operator=(
				    const ref::ptr_fast<T> &rhs )
	    throw()
	{
	    (void) ref::ptr_tiny<T>::operator=( rhs );
	    _cachedptr			= rhs._cachedptr;
	    return *this;
	}
	ref::ptr_fast<T>       &operator=(
				    const ref::ptr_tiny<T> &rhs )
	    throw()
	{
	    (void) ref::ptr_tiny<T>::operator=( rhs );
	    _cachedptr			= rhs.get();
	    return *this;
	}
	template <class Derived>
	ref::ptr_fast<T>       &operator=(
			            const ref::ptr_tiny<Derived>
				    		       &rhs )
	{
	    ref::ptr_tiny<T>::operator=( rhs );
	    _cachedptr			= rhs.get();			// uses slow get()
	    return *this;
	}
	template <class Derived>
	ref::ptr_fast<T>       &operator=(
			            const ref::ptr_fast<Derived>
				    		       &rhs )
	{
	    ref::ptr_tiny<T>::operator=( rhs );
	    _cachedptr			= rhs.get();			// uses fast (cached) get()	
	    return *this;
	}

	// 
	// operator x( ref::ptr_fast )	-- Test _cachedptr
	// 
	//     We will simply inherit re::ptr_tiny<T>'s operator bool() equivalent and operator!()
	// 
	bool			operator==(
				    T		       *rhs )
	    const
	    throw()
	{
	    return ( _cachedptr == rhs );
	}
	bool			operator!=(
				    T    	       *rhs )
	    const
	    throw()
	{
	    return ( _cachedptr != rhs );
	}
	bool			operator==(
			            const ref::ptr_fast<T>
				    		       &rhs )
	    const
	    throw()
	{
	    return ( _cachedptr == rhs._cachedptr );
	}
	bool			operator==(
			            const ref::ptr_tiny<T>
				    		       &rhs )
	    const
	    throw()
	{
	    return ( _cachedptr == rhs.get() );
	}
	bool			operator!=(
				    const ref::ptr_fast<T>
				    		       &rhs )
	    const
	    throw()
	{
	    return ( _cachedptr != rhs._cachedptr );
	}
	bool			operator!=(
				    const ref::ptr_tiny<T>
				    		       &rhs )
	    const
	    throw()
	{
	    return ( _cachedptr != rhs.get() );
	}
#if defined( REF_PTR_COMPARE )
	bool			operator<(
			            const T    	       *rhs )
	    const
	    throw()
	{
	    return ( _cachedptr < rhs );
	}
	bool			operator<(
				    const ref::ptr_fast<T>
				    		       &rhs )	
	    const
	    throw()
	{
	    return ( _cachedptr < rhs._cachedptr );
	}
	bool			operator<(
				    const ref::ptr_tiny<T>
				    		       &rhs )	
	    const
	    throw()
	{
	    return ( _cachedptr < rhs.__ref_getptr() );
	}
#endif
	// 
	// T *ref::ptr_fast::operator->	-- Follow pointer to object using _cachedptr
	// T &ref::ptr_fast::operator*	-- Deref. pointer to object using _cachedptr
	// 
	T		       *operator->()
	    const
#if ! defined( REF_PTR_DEREF_TEST )
	    throw()
#endif
	{
	    T		       *pointer	= _cachedptr;

#if defined( REF_PTR_DEREF_TEST )
	    if ( ! pointer ) {
		std::ostringstream	error;
		error << "T &ref::ptr<T>::operator-> "<< this << " is attempting to dereference, ref-counter didn't reference any object! (probably ref::ptr implementation error)";
		throw std::logic_error( error.str() );
	    }
#endif

	    return pointer;
	}

	T		       &operator*()
	    const
#if ! defined( REF_PTR_DEREF_TEST )
	    throw()
#endif
	{
	    T		       *pointer	= _cachedptr;

#if defined( REF_PTR_DEREF_TEST )
	    if ( ! pointer ) {
		std::ostringstream	error;
		error << "T &ref::ptr<T>::operator*  " << this << " is attempting to dereference, ref-counter didn't reference any object! (probably ref::ptr implementation error)";
		throw std::logic_error( error.str() );
	    }
#endif

	    return *pointer;
	}
    };


    // 
    // ref::ptr<T>		-- is a ref::ptr_tiny<T> OR is a ref::ptr_fast<T>
    // 
    //     Implements ref::ptr<T> as either ref::ptr_tiny<T> or ref::ptr_fast<T>.  Must implement
    // at least the "default" copy/assignment constructors, if necessary.  The default copy
    // constructor invokes the base class constructor, which is correct.  However, the defualt
    // assignment operator just does a member-wise copy, which is not ideal (correct, just
    // inefficient; it creates an extra temporary); we want to invoke the base-class assignment
    // operator, instead...
    // 
    // WARNING
    // 
    //     DO NOT mix code compiled with different implementations of ref::ptr<T>!  Really, the
    // REF_PTR_DEREF_FAST setting should only be used for testing purposes; so that unit tests may
    // be written where the same implementation can be tested using ref::ptr_tiny<T> and
    // ref::ptr_fast<T>.  If you DO mix code (say, you compile two libraries with different
    // REF_PTR_DEREF_FAST settings), prepare for probable anihilation.
    // 
    //     Some architectures will only use assignment operators defined in the derived class;
    // others will inherit those defined in the base class.  Define them all here, to ensure they
    // get used.
    // 
#if ! defined( REF_PTR_DEREF_FAST )

    template <class T>
    class ptr
    : public ptr_tiny<T> {
    public:
	void			swap(
				    ptr  	       &rhs )
	    throw()
	{
	    ptr_tiny<T>::swap( rhs );
	}

				ptr<T>(
				    const ptr_tiny<T>  &rhs )
	    throw()
				    : ptr_tiny<T>( rhs )
	{
	    ;
	}

	template<class Derived>	ptr<T>(
				    const ptr<Derived> &rhs )
				    : ptr_tiny<T>( rhs )
	{
	    ;
	}
				ptr<T>(
				    T	       	       *pointee 	= 0 )
				    : ptr_tiny<T>( pointee )
	{
	    ;
	}
			       ~ptr<T>()
	{
	    ;
	}
#if 1
	ptr<T>	               &operator=(
				    const ptr_tiny<T>  &rhs )
	{
	    ptr_tiny<T>::operator=( rhs );
	    return *this;
	}
	template<class Derived>
	ptr<T>		       &operator=(
				    const ptr_tiny<Derived> &rhs )
	{
	    ptr_tiny<T>::operator=( rhs );
	    return *this;
	}
	ptr<T>		       &operator=(
				    T		       *pointee )
	{
	    ptr_tiny<T>::operator=( pointee );
	    return *this;
	}
#endif
    };

#else // defined( REF_PTR_DEREF_FAST )

    template <class T>
    class ptr
    : public ptr_fast<T> {
    public:
	void			swap(
				    ptr  	       &rhs )
	    throw()
	{
	    ptr_fast<T>::swap( rhs );
	}

				ptr<T>(
				    const ptr_fast<T>  &rhs )
	    throw()
				    : ptr_fast<T>( rhs )
	{
	    ;
	}
				ptr<T>(
				    const ptr_tiny<T>  &rhs )
	    throw()
				    : ptr_fast<T>( rhs )
	{
	    ;
	}
	template<class Derived>	ptr<T>(
				    const ptr<Derived> &rhs )
				    : ptr_fast<T>( rhs )
	{
	    ;
	}
				ptr<T>(
				    T	       	       *pointee 	= 0 )
				    : ptr_fast<T>( pointee )
	{
	    ;
	}
			       ~ptr<T>()
	{
	    ;
	}
#if 1
	ptr<T>	               &operator=(
				    const ptr_tiny<T>  &rhs )
	{
	    ptr_fast<T>::operator=( rhs );
	    return *this;
	}
	ptr<T>	               &operator=(
				    const ptr_fast<T>  &rhs )
	{
	    ptr_fast<T>::operator=( rhs );
	    return *this;
	}
	template <class Derived>
	ptr<T> 	       	       &operator=(
			            const ptr_tiny<Derived>
				    		       &rhs )
	{
	    ptr_fast<T>::operator=( rhs );
	    return *this;
	}
	template <class Derived>
	ptr<T>                 &operator=(
			            const ptr_fast<Derived>
				    		       &rhs )
	{
	    ptr_fast<T>::operator=( rhs );
	    return *this;
	}
#if 0
	template<class Derived>
	ptr<T>		       &operator=(
				    const ptr<Derived> &rhs )
	{
	    ptr_fast<T>::operator=( rhs );
	    return *this;
	}
#endif
	ptr<T>		       &operator=(
				    T		       *pointee )
	{
	    ptr_fast<T>::operator=( pointee );
	    return *this;
	}
#endif
    };

#endif

    // 
    // ref::dyn<T>
    // 
    //     Implement a dynamically self-destructing reference-counted
    // version of class T.  Given the declaration:
    // 
    //     ref::ptr<ref::dyn<X> > 	x;
    // 
    // *x will return an "& ref::dyn<X>", which is trivially convertible
    // into an X.  So, x can be used virtually anywhere that a normal (X*)
    // pointer could be used: x->X_method(), etc.
    // 
    // WARNING
    // 
    //     Although a 'ref::dyn<X>' is normally dynamically allocated, and used in conference with
    // a 'ref::ptr<ref::dyn<X> >', it need not be.  If a ref::dyn<X> is created (eg. via 'new', or
    // as an automatic stack variable, or as a static variable) and is NEVER assigned to a
    // ref::ptr<ref::dyn<X> >, then its reference counter will never be incremented, and hence
    // will never be decremented to 0; it will be destructed only when it either goes out of
    // scope, OR when it is 'delete'-ed.
    // 
    //     HOWEVER, if you DO NOT dynamically allocate a ref::dyn<X>, then DO NOT assign it to a
    // 'ref::ptr<ref::dyn<X> >'!  If you do, you will reap the consequences of your iniquity, and
    // your heap will be corrupted, your vinyards will be salted, and your house will be burned
    // and used as a dung-heap.  In other words, it will try to spontaneously 'delete' itself as
    // soon as it's (internal) ref::counter once again becomes zero, and will corrupt the heap...
    // 
    // USAGE
    //     ref::ptr<ref::dyn<std::string> >	  q;
    //     { 
    //         ref::ptr<ref::dyn<std::string> > s;
    // 
    //         s		= new ref::dyn<std::string>;	// reference count == 1
    //         *s	       += "Hello";
    //         q		= s;				// *q and *s are the same ref::dyn<std::string>; reference count == 2
    //         *s	       += ", ";
    //     }							// 's' out of scope ;reference count back to 1
    //     *q    	       += "World!"			// *q == "Hello, World!";
    //     
    //     q		= 0;					// reference count == 0; object self-destructs
    // 
    template <class T>
    class dyn
	: public counter< dyn<T> >
	, public T {

	// Return the actual object being reference counted.
	virtual ref::dyn<T>    *__ref_getptr()
	    throw()
	{
	    return this;
	}

    public:
	// Destructor.  Required for type checking only.  There is no
	// ref::dyn<T> specific data to destruct.
	virtual		       ~dyn<T>()
	{
	    ;
	}
    };


    // 
    // ref::auto_array<T>	-- Overall, just a really bad idea...
    // 
    ///     Yes, you'll see if you use it.  Go ahead, try to fix it.  Better yet, just use ref::ptr,
    /// or boost::shared_ptr.
    /// 
    ///     Just like auto_ptr<T>, except for arrays of T's.  Auto-destructs the T array when the
    /// auto_array<T> goes out of scope.  NO reference counting!!  auto_ptr<T> assumes ownership of
    /// any T* assigned to it; don't manually delete it, once it has been assigned to an
    /// auto_array<T>!  If you assign another T array to the auto_array<T>, the last array is
    /// delete'd during the assignment.
    /// 
    ///     {
    ///         T		       *a	= new T[10];
    ///         ref::auto_array<T>	acln( a );
    ///         ref::auto_array<T>	b( new T[20] );
    ///         a[ 9] = ...
    ///         b[19] = ...
    ///         T		       *c	= new T[100];
    ///	       b				= c;		// b delete'd
    ///     }
    ///     // a and c deleted.
    /// 
    template<class T>
    class auto_array {
    private:
	T		       *_array;

    public:
	// 
	// Constructor	-- Manage a dynamically allocated array of T's
	// 
	///     Manages an externally dynamically allocated array.  Once assigned to an
	/// auto_array<T>, the T* is owned by the auto_array<T>.  Assignment and copying transfers
	/// ownership.  If the assigned is a const auto_array<T>, then we cannot assume ownership,
	/// and hence we hide that constructor.
	///  
				auto_array<T>()
	    throw()
	    : _array( 0 )
	{
	    ;
	}
	explicit		auto_array<T>(
			            T	    	       *array )
	    throw()
	    : _array( array )
	{
	    ;
	}
	explicit		auto_array<T>(
			            auto_array<T>      &rhs )
	    throw()
	{
	    _array			= rhs._array;
	    rhs._array			= 0;
	}
    private:

				auto_array<T>(
				    const auto_array<T> &rhs );

    public:

	// Destructor	-- auto-destruct the instance's array (if set).
			       ~auto_array<T>()
	{
	    if ( _array )
		delete [] _array;
	}

	// 
	// auto_array<T>::operator=( T* )
	// auto_array<T>::operator=( auto_array<T>& )
	// auto_array<T>::operator=( const auto_array<T>& )
	/// 
	///     Assignment releases any previously held buffer.  
	/// 
	///     If the assigned is a const auto_array<T>, then we cannot assume ownership, and
	/// hence we hide that method.
	auto_array<T>	       &operator=(
				    T	       	       *array )
	    throw()
	{
	    if ( _array )
		delete [] _array;
	    _array			= array;
	    return *this;
	}
	auto_array<T>	       &operator=(
				    auto_array<T>      &rhs )
	    throw()
	{
	    *this			= rhs._array;
	    rhs._array			= 0;
	    return *this;
	}

    private:

	auto_array<T>	       &operator=(
				    const auto_array<T> & );

    public:

	// 
	//       (T *&)              auto_array<T>
	//       (T * const &) const auto_array<T>
	//       (T  &)              auto_array<T>::operator*
	//       (T  &) const        auto_array<T>::operator*
	// 
	//     Allow access to the array wherever the auto_array<T> instance is used as a (T*).  We
	// pass back a reference to the original T* pointer, so that if the caller takes the address
	// of it, he'll get a pointer to the underlying _array pointer, NOT a pointer to his COPY of
	// the _array pointer!  There are identical const and non-const versions, to allow taking the
	// address of auto_array<T> arrays within const objects.
	// 
	// Empty arrays are allowed; we do NOT check for 0 pointers.  We couldn't do this by simply
	// adding an T&operator*() like:
	// 
	//    T			       &operator *()
	//        const
	//    {
	//        if ( ! _array ) {
	//	      std::ostringstream error;
	//            error << "auto_array " << this << " is attempting to dereference, but no array has been assigned!";
	// 	      throw std::logic_error( error.str() );
	//	  }
	//        return *_array;
	//    }
	// 
	// because often programs allow (and check for) 0 array pointers; some compilers (g++ 3.3.2,
	// etc.)  prefer to use T&operator*() (if it exists) for ALL array accesses; even those NOT
	// resulting in dereferencing of the array.  In any case, for complete 0 pointer dereference
	// checking, we would probably have to add operator+() and operator[]() anyway.  So, don't do it
	// at all.  If you want 0-pointer dereference checking, use ref::ptr<>.  If you want
	// auto-destruction of arrays (a la auto_ptr<>), then use auto_array<>.
	// 
				operator T * &()
	{
	    return _array;
	}
				operator T * const &()
	    const
	{
	    return _array;
	}
#if 0
	// Elide operatorT*() alternatives, due to compiler differences.  Basically, the different
	// compilers (gcc, aCC) have different methods of selecting what method to use, and complain
	// about too many choices, or ambiguous template-generated operators.  By not having these
	// options, you cannot use a const auto_array<T> as a (T*), severely limiting the usefulness of
	// auto_array<T> as a class member...
				operator const T * &()
	{
	    return _array;
	}
				operator T * const &()
	    const
	{
	    return _array;
	}
				operator const T * const &()
	    const
	{
	    return _array;
	}
#endif

	// 
	// (T **)       auto_array<T>::operator&
	// (T **) const auto_array<T>::operator&
	// 
	//     Taking the address of an auto_array<T> is like taking the
	// address of a pointer.  If the user replaces the memory with a
	// new (dynamically allocated, of course) array (eg. with a new
	// different sized buffer), the caller is responsible for deleting
	// the old buffer.
	// 
	T 		      **operator &()
	{
	    return &_array;
	}
	T 		      **operator &()
	    const
	{
	    return &_array;
	}
    };

    // 
    // ref::array
    // 
    ///     Raw type array template.  Avoids dynamic allocation completely, for arrays of items of
    /// known size.  Has an item-wise comparison operator, iterators, and (optionally)
    /// bounds-checked element access.
    /// 
    /// EXAMPLE
    ///     const char	        s[]	= "Hello";
    ///     ref::array<char,10>	a( s, s + sizeof s );
    ///
    template <typename T, unsigned S>
    struct array {
	T			val[S];
	typedef T	       *iterator;
	typedef const T	       *const_iterator;

	template <typename Iter>
	    			array<T,S>(
				    Iter	        beg_,
				    Iter                end_ )
	{
	    iterator  	        t;	
	    for ( t			= val
		 ; t < end() && beg_ < end_
		 ; ++t, ++beg_ ) {
		*t			= *beg_;
	    }
	    for ( ; t < end()
		  ; ++t )
		*t			= T();
	}
	    			array<T,S>(
				    const_iterator      beg_	= 0,
				    const_iterator      end_	= 0 )
	{
	    iterator  	        t;	
	    for ( t			= val
		 ; t < end() && beg_ < end_
		 ; ++t, ++beg_ ) {
		*t			= *beg_;
	    }
	    for ( ; t < end()
		  ; ++t )
		*t			= T();
	}
	    			array<T,S>(
				    const array<T,S>   &rhs )
	{
	    iterator   	        t;
	    const_iterator      u;
	    for ( t			= begin()
		 , u			= rhs.begin()
		 ; t < end()
		 ; ++t, ++u ) {
		*t			= *u;
	    }
	}

	    		       ~array<T,S>()
	{
	    ;
	}

	unsigned		size()
	    const
	{
	    return S;
	}
	const_iterator		begin()
	    const
	{
	    return val;
	}
	iterator		begin()
	{
	    return val;
	}
	const_iterator		end()
	    const
	{
	    return val + size();
	}
	T		       &front()
	{
	    return val[0];
	}
	const T		       &front()
	    const
	{
	    return val[0];
	}

	// 
	// array<T,S> < array<T,R>
	// 
	///     Lexicographical comparison of arrays.  The arrays may have differing sizes.
	template <unsigned R>
	bool		operator<(
			    const array<T,R>   	       &rhs )
	    const
	{
	    return std::lexicographical_compare(     val,     val + S, 
						 rhs.val, rhs.val + R );
	}

	const T	       	       &operator[](
				    unsigned    	i )
	    const
	{
#if defined( REF_PTR_DEREF_TEST )
	    if ( i >= S )
		throw std::logic_error( "const ref::array bounds exceeded" );
#endif
	    return val[i];
	}
	T	       	       &operator[](
				    unsigned    	i )
	{
#if defined( REF_PTR_DEREF_TEST )
	    if ( i >= S )
		throw std::logic_error( "ref::array bounds exceeded" );
#endif
	    return val[i];
	}
    }; // struct array
}; // namespace ref

namespace std {

#if ! defined( REF_PTR_NO_SWAP )

    // 
    // std::swap		-- Specialise std::swap to know how to swap various ref::ptr...<T> variants
    // 
    //     This should be relatively safe, so we'll turn it on by default
    // 
    template <typename T>
    inline
    void			swap(
				    ref::ptr_tiny<T>   &lhs,
				    ref::ptr_tiny<T>   &rhs )
        throw()
    {
	lhs.swap( rhs );
    }

    template <typename T>
    inline
    void			swap(
				    ref::ptr_fast<T>   &lhs,
				    ref::ptr_fast<T>   &rhs )
        throw()
    {
	lhs.swap( rhs );
    }

    template <typename T>
    inline
    void			swap(
				    ref::ptr<T>	       &lhs,
				    ref::ptr<T>        &rhs )
        throw()
    {
	lhs.swap( rhs );
    }
#endif // ! REF_PTR_NO_SWAP

#if defined( REF_PTR_ITER_SWAP )
    // 
    // std::iter_swap		-- Specialise std::iter_swap to use std::swap, if possible
    // 
    ///     Many STL implementations pre-pessimize std::iter_swap (eg. GNU C++ pre-4.0), by
    /// choosing to NOT use std::swap when both iterators are of identical type.  However, there
    /// are situations where it might not be desirable, so we'll leave this turned off by default.
    /// Mostly, we define this for testing purposes.  If you use the STL algorithms, and you want
    /// to avoid forcing std::iter_swap to use temp/copy/assign swapping for objects, then I would
    /// suggest defining the following template somewhere in one of your high-level include
    /// files...
    /// 
    ///     Generally, iterators are expected to be passed around by copying, so that's wey
    /// we don't use (const iter_t &) here...  The point is, that this optimisation is only
    /// used when the iterators are of identical type.
    ///
    template <typename iter_t>
    inline
    void                        iter_swap(
				  iter_t              lhs,
				  iter_t              rhs )
        throw()
    {
	std::swap( *lhs, *rhs );
    }
#endif

} // namespace std

#endif // _INCLUDE_REF_H
