//
//  Copyright 1999, 2000 Gregory Colvin
//  All Rights Reserved.
//

#include <assert.h>
#include <vector>
#include <list>
#include <algorithm>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <typeinfo>
#include <boost/smart_ptr.hpp>
#include "linked_ptr.hpp"
#include "fast_shared_ptr.hpp"
#include "shared_in_ptr.hpp"
#include "cyclic_ptr.hpp"
#include "cyclic_ptr.cpp"

#include <ref>


struct S {
private:
    int		       	       	val;
public:
    				S()
				    : val( rand() )
    {
	;
    }

    bool 			operator<(
				    const S & other )
	const
    { 
	return val < other.val;
    }
};

#if defined( REF_PTR_REVERSE )

class B 
    : public S 
    , public boost::shared_in_base<std::size_t> {
};

struct R 
    : S 
    , ref::counter<R> {			// Like shared_in_base, but for ref::ptr...
    R          		       *__ref_getptr()  { return this; }
};

#else

class B 
    : public boost::shared_in_base<std::size_t>
    , public S {
};

struct R 
    : ref::counter<R> 			// Like shared_in_base, but for ref::ptr...
    , S {
    R          		       *__ref_getptr()  { return this; }
};

#endif

using namespace std;
//using namespace boost;


template <typename ptr_obj, typename obj>
void test_fill_sort( int N )
{
   printf( "Testing %s\n", typeid(ptr_obj).name() );

   {  clock_t start = clock();
      vector<ptr_obj> container;
      for (int i = 0; i < N; i++ )
         container.push_back(ptr_obj(new obj()));
      printf("fill vector: %ld\n",(long)clock() - start);
      sort(container.begin(), container.end());
      printf("sort vector: %ld\n",(long)clock() - start);
   }
   {  clock_t start = clock();
      list<ptr_obj> container;
      for (int i = 0; i < N; i++ )
         container.push_back(ptr_obj(new obj()));
      printf("fill list: %ld\n",(long)clock() - start);
      container.sort();
      printf("sort list: %ld\n",(long)clock() - start);
   }
   {  clock_t start = clock();
      set<ptr_obj> container;
      for (int i = 0; i < N; i++ )
         container.insert(ptr_obj(new obj()));
      printf("fill set: %ld\n",(long)clock() - start);
   }
}

int main( int, char** ) {
   srand( (unsigned)time( NULL ) );
   const int N = 300000;

   //typedef B* ptr_obj;
   test_fill_sort<B *, B                          >( N );
   //typedef shared_ptr<B> ptr_obj;
   test_fill_sort<boost::shared_ptr<B>, B         >( N );
   //typedef fast_shared::shared_ptr<B> ptr_obj;
   test_fill_sort<fast_shared::shared_ptr<B>, B   >( N );
   //typedef smart_pointers::linked_ptr<B> ptr_obj;
   test_fill_sort<smart_pointers::linked_ptr<B>, B>( N );
   //typedef boost::cyclic_ptr<B> ptr_obj;
   test_fill_sort<boost::cyclic_ptr<B>, B         >( N );
   //typedef boost::shared_in_ptr<B> ptr_obj;
   test_fill_sort<boost::shared_in_ptr<B>, B      >( N );


   test_fill_sort<ref::ptr_tiny<R>, R             >( N );
   test_fill_sort<ref::ptr_tiny<S>, S             >( N );
   test_fill_sort<ref::ptr_fast<R>, R             >( N );
   test_fill_sort<ref::ptr_fast<S>, S             >( N );

   return 0;
}
