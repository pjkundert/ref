//
// smarttest.cpp
//
//  (C) Copyright Gavin Collings 2000. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//

#ifdef _MSC_VER
   #pragma warning( disable: 4786 )    // truncation of long template names
#endif

#include <boost/config.hpp>
#include <cstddef>
#include <time.h>
#include <stdio.h>
#include <utility>
#include <iostream>

#include <boost/smart_ptr.hpp>
#include "cyclic_ptr.hpp"
#include "fast_shared_ptr.hpp"
#include "linked_ptr.hpp"
#include "shared_in_ptr.hpp"

#include "cyclic_ptr.cpp"


#include <ref>


struct S {
    int			_;
};

#if defined( REF_PTR_REVERSE )
struct B 
    : S
    , boost::shared_in_base<std::size_t> {
    ;
};

struct R 
    : S 
    , ref::counter<R> {				// Like shared_in_base, but for ref::ptr...
    R			       *__ref_getptr()  { return this; }
};

#else

struct B
    : boost::shared_in_base<std::size_t>
    , S {
    ;
};

struct R
    : ref::counter<R>
    , S {
    R			       *__ref_getptr()  { return this; }
};
#endif // REF_PTR_REVERSE


template <typename ptr_type, typename obj_type> double test_smart_pointer( std::size_t num_of_it, std::size_t num_of_sets, std::size_t num_in_set )
{
   std::size_t it, i, j;
   clock_t     start, finish;
   clock_t     rolling = 0;

   ptr_type*   pointers     = new ptr_type[num_in_set];
   obj_type**  raw_pointers = new obj_type*[num_of_sets];;

   for ( it = 0; it < num_of_it; ++it )
   {
      for ( i = 0; i < num_of_sets; ++i )
      {
         raw_pointers[i] = new obj_type;
      }

      start = clock();

      for ( i = 0; i < num_of_sets; ++i )
      {
         ptr_type ident( raw_pointers[i] );

         for ( j = 0; j < num_in_set; ++j )
         {
            pointers[j] = ptr_type( ident );
         }
      }

      for ( j = 0; j < num_in_set; ++j )
      {
         pointers[j].reset();
      }

      finish = clock();

      rolling += finish - start;
   }

   delete [] raw_pointers;
   delete [] pointers;

   return (double( rolling ) / double( CLOCKS_PER_SEC ));
}


double test_dumb_pointer( std::size_t num_of_it, std::size_t num_of_sets, std::size_t num_in_set )
{
   typedef smart_pointers::dumb_ptr<B> ptr_type;

   std::size_t it, i, j;
   clock_t     start, finish;
   clock_t     rolling = 0;

   ptr_type*   pointers     = new ptr_type[num_in_set];
   B**         raw_pointers = new B*[num_of_sets];;

   for ( it = 0; it < num_of_it; ++it )
   {
      for ( i = 0; i < num_of_sets; ++i )
      {
         raw_pointers[i] = new B;
      }

      start = clock();

      for ( i = 0; i < num_of_sets; ++i )
      {
         ptr_type ident( raw_pointers[i] );

         for ( j = 0; j < num_in_set; ++j )
         {
            pointers[j] = ptr_type( ident );
         }

         ident.release();
      }

      finish = clock();

      rolling += finish - start;
   }

   delete [] raw_pointers;
   delete [] pointers;

   return (double( rolling ) / double( CLOCKS_PER_SEC ));
}





double test_raw_pointer( std::size_t num_of_it, std::size_t num_of_sets, std::size_t num_in_set )
{
   std::size_t it, i, j;
   clock_t     start, finish;
   clock_t     rolling = 0;

   B**   pointers     = new B*[num_in_set];
   B**   raw_pointers = new B*[num_of_sets];
   B*    pointer_copy;

   for ( it = 0; it < num_of_it; ++it )
   {
      for ( i = 0; i < num_of_sets; ++i )
      {
         raw_pointers[i] = new B;
      }

      start = clock();

      for ( i = 0; i < num_of_sets; ++i )
      {
         B* ident = raw_pointers[i];

         for ( j = 0; j < num_in_set; ++j )
         {
            pointers[j]  = pointer_copy = ident;
         }

         delete ident;
      }

      for ( j = 0; j < num_in_set; ++j )
      {
      }

      finish = clock();

      rolling += finish - start;
   }

   delete [] raw_pointers;
   delete [] pointers;

   return (double( rolling ) / double( CLOCKS_PER_SEC ));
}

void test_data_sizes()
{
   std::cout << "# sizeof( boost::shared_ptr<B> )          " << sizeof ( boost::shared_ptr<B>          ) << std::endl;
   std::cout << "# sizeof( boost::shared_in_ptr<B> )       " << sizeof ( boost::shared_in_ptr<B>       ) << std::endl;
   std::cout << "# sizeof( fast_shared::shared_ptr<B> )    " << sizeof ( fast_shared::shared_ptr<B>    ) << std::endl;
   std::cout << "# sizeof( smart_pointers::linked_ptr<B> ) " << sizeof ( smart_pointers::linked_ptr<B> ) << std::endl;
   std::cout << "# sizeof( boost::cyclic_ptr<B> )          " << sizeof ( boost::cyclic_ptr<B>          ) << std::endl;
   std::cout << "# sizeof( ref::ptr_tiny<R> )              " << sizeof ( ref::ptr_tiny<R>              ) << std::endl;
   std::cout << "# sizeof( ref::ptr_fast<R> )              " << sizeof ( ref::ptr_fast<R>              ) << std::endl;
}

int main()
{
    test_data_sizes();

   const double convert_to_ns = 1000000000.0;

   std::size_t number_of_iterations[] = { 100000, 100000, 100000, 10000, 10000, 10000, 10000, 5000, 2000, 1000 };
   std::size_t number_of_sets[]       = {    100,     50,     25,   167,   125,   100,    67,   67,  100,  100 };
   std::size_t number_in_set[]        = {      1,      2,      4,     6,     8,    10,    15,   30,   50,  100 };

   std::size_t number_of_tests = sizeof( number_in_set ) / sizeof( number_in_set[0] );

   FILE* out = fopen( "smarts.txt", "w" );

   for ( size_t i = 0; i < number_of_tests; ++i )
   {
      std::size_t n_iterations = number_of_iterations[i];
      std::size_t n_sets       = number_of_sets[i];
      std::size_t n_in_set     = number_in_set[i];
      std::size_t n_objects    = n_iterations * n_sets;
      std::size_t n_operations_per_el  = 2;
      std::size_t n_operations_per_set = (n_operations_per_el * n_in_set);    // copy operations (assignment + copy cons)
      std::size_t n_operations_per_it  = n_operations_per_set * n_sets;
      std::size_t n_operations         = n_operations_per_it  * n_iterations;

#if defined( REF_PTR_ONLY )
      double cyclic_time      = 0;
      double fast_shared_time = 0;
      double linked_time      = 0;
      double shared_time      = 0;
      double shared_in_time   = 0;
#else
      double cyclic_time      = test_smart_pointer<boost::cyclic_ptr<B>, B>
	  						( n_iterations, n_sets, n_in_set );

      double fast_shared_time = test_smart_pointer<fast_shared::shared_ptr<B>, B>
	  						( n_iterations, n_sets, n_in_set );

      double linked_time      = test_smart_pointer<smart_pointers::linked_ptr<B>, B>
	  						( n_iterations, n_sets, n_in_set );

      double shared_time      = test_smart_pointer<boost::shared_ptr<B>, B>
	  						( n_iterations, n_sets, n_in_set );

      double shared_in_time   = test_smart_pointer<boost::shared_in_ptr<B>, B>
	  						( n_iterations, n_sets, n_in_set );
#endif
      // R implements ref::counter, so the object contains the counter
      double ref_ptr_tiny_in_time = test_smart_pointer<ref::ptr<R>, R>
	  						( n_iterations, n_sets, n_in_set );
      // S doesn't implement ref::counter, so an external one is used
      double ref_ptr_tiny_no_time = test_smart_pointer<ref::ptr<S>, S>
	  						( n_iterations, n_sets, n_in_set );
      // R implements ref::counter, so the object contains the counter
      double ref_ptr_fast_in_time = test_smart_pointer<ref::ptr_fast<R>, R>
	  						( n_iterations, n_sets, n_in_set );
      // S doesn't implement ref::counter, so an external one is used
      double ref_ptr_fast_no_time = test_smart_pointer<ref::ptr_fast<S>, S>
	  						( n_iterations, n_sets, n_in_set );


      double dumb_time        = test_dumb_pointer( n_iterations, n_sets, n_in_set );
      double raw_time         = test_raw_pointer ( n_iterations, n_sets, n_in_set );
      double overhead_time    = raw_time; // std::min( dumb_time, raw_time ); VC problem

      double cyclic_overhead      = ( cyclic_time      - overhead_time ) / n_operations;
      double fast_shared_overhead = ( fast_shared_time - overhead_time ) / n_operations;
      double linked_overhead      = ( linked_time      - overhead_time ) / n_operations;
      double shared_overhead      = ( shared_time      - overhead_time ) / n_operations;
      double shared_in_overhead   = ( shared_in_time   - overhead_time ) / n_operations;
      double ref_ptr_tiny_in_overhead  = ( ref_ptr_tiny_in_time  - overhead_time ) / n_operations;
      double ref_ptr_tiny_no_overhead  = ( ref_ptr_tiny_no_time  - overhead_time ) / n_operations;
      double ref_ptr_fast_in_overhead  = ( ref_ptr_fast_in_time  - overhead_time ) / n_operations;
      double ref_ptr_fast_no_overhead  = ( ref_ptr_fast_no_time  - overhead_time ) / n_operations;

      cyclic_overhead        *= convert_to_ns;
      fast_shared_overhead   *= convert_to_ns;
      linked_overhead        *= convert_to_ns;
      shared_overhead        *= convert_to_ns;
      shared_in_overhead     *= convert_to_ns;
      ref_ptr_tiny_in_overhead *= convert_to_ns;
      ref_ptr_tiny_no_overhead *= convert_to_ns;
      ref_ptr_fast_in_overhead *= convert_to_ns;
      ref_ptr_fast_no_overhead *= convert_to_ns;

      fprintf( out, "%7d, %8d, %8d: (Cy,FS,L,S,SI, RPTI,RPTN,RPFI,RPFN, dumb, Raw): (%7.3f,%7.3f,%7.3f,%7.3f,%7.3f, %7.3f,%7.3f,%7.3f,%7.3f, %7.3f, %7.3f )\n",
	       n_in_set, n_objects, n_operations, 
	       cyclic_time, fast_shared_time, linked_time, shared_time, shared_in_time,
	       ref_ptr_tiny_in_time,ref_ptr_tiny_no_time,
	       ref_ptr_fast_in_time,ref_ptr_tiny_no_time,
	       dumb_time, raw_time );
      printf (      "%7d, %8d, %8d: (Cy,FS,L,S,SI, RPTI,RPTN,RPFI,RPFN, dumb, Raw): (%7.3f,%7.3f,%7.3f,%7.3f,%7.3f, %7.3f,%7.3f,%7.3f,%7.3f, %7.3f, %7.3f )\n",
		    n_in_set, n_objects, n_operations, 
		    cyclic_time, fast_shared_time, linked_time, shared_time, shared_in_time,
		    ref_ptr_tiny_in_time,ref_ptr_tiny_no_time,
		    ref_ptr_fast_in_time,ref_ptr_tiny_no_time,
		    dumb_time, raw_time );
   }

   fclose( out );

   return 0;
}



