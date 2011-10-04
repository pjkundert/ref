//
//  linked_ptr.hpp
//
//  (C) Copyright Gavin Collings 2000. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//

#ifndef _linked_ptr_hpp
#define _linked_ptr_hpp


#include <boost/config.hpp>
#include <cstddef>            // std::size_t
#include <algorithm>          // std::swap


namespace smart_pointers
{
   template <typename T> class linked_ptr;


   class link_sharer
   {
   public:

      std::size_t use_count() const throw() 
      {
         const link_sharer * current = next;
         std::size_t         count   = 1;

         while ( current != this )
         {
            ++count;
            current = current->next;
         }

         return count;
      }

      bool unique() const throw()
      {
         return next == this;
      }

      void swap( link_sharer & other ) throw()
      {
         link_sharer * that           = &other;
         bool          this_isolated  = unique();
         bool          that_isolated  = that->unique();

         if ( this_isolated )
         {                                // if both isolated, do nothing
            if ( !that_isolated )         // but if only this is:
            {
               next = that->next;
               prev = that->prev;

               next->prev = this;         // adjust back pointers
               prev->next = this;

               that->isolate();
            }
         }
         else if ( that_isolated )        // and this isn't
         {                                
            that->next = next;
            that->prev = prev;
         
            next->prev = that;            // adjust back pointers
            prev->next = that;

            isolate();
         }
         else                             // neither isolated - full blown swap
         {
            next->prev = that;            // adjust back pointers
            prev->next = that;

            std::swap( next, that->next );
            std::swap( prev, that->prev );

            next->prev = this;            // adjust back pointers
            prev->next = this;
         }
      }

      void isolate() throw()
      {
         next = this;
         prev = this;
      }

      mutable const link_sharer * next;
      mutable const link_sharer * prev;
   };



   template <typename T> class linked_ptr : public link_sharer
   {
   public:

      typedef T element_type;

      linked_ptr( const linked_ptr & other ) throw()
      :  body( other.body )
      { 
         (next = other.next)->prev = this;
         (prev = &other    )->next = this;
      }

      explicit linked_ptr( T * p = 0 ) : body( p ) { next = prev = this; }

      ~linked_ptr()
      {
         if ( next == this )
         {
            delete body;
         }
         else
         {
            prev->next = next;   
            next->prev = prev;
         }
      }

      linked_ptr & operator=( const linked_ptr & other )
      {
         if ( body != other.body )
         {
            if ( next == this )
            {
               delete body;
            }
            else
            {
               prev->next = next;   
               next->prev = prev;
            }

            body  = other.body;
         
            (next = other.next)->prev = this;
            (prev = &other    )->next = this;
         }
 
         return *this;
      }

      void reset( T * p = 0 )
      {
         if ( p != body )
         {
            if ( next == this )
            {
               delete body;
            }
            else
            {
               prev->next = next;   
               next->prev = prev;
            }

            body = p;
            next = prev = this;
         }
      }

      bool operator <( const linked_ptr & other ) const { return *body < *other.body; }

      T & operator*()  const throw() { return *body; }
      T * operator->() const throw() { return  body; }
      T * get()        const throw() { return  body; }

      void swap( linked_ptr & other ) throw() {
                                                 std::swap( body, other.body );
                                                 link_sharer::swap( other );
                                              }
   private:

      T * body;

      friend class link_sharer;
   };




   template <typename T> class dumb_ptr
   {
   public:

      typedef T element_type;

      explicit dumb_ptr( T * p = 0 )    : body( p )          {}
      dumb_ptr( const dumb_ptr & other ): body( other.body ) {}

      ~dumb_ptr() {}

      dumb_ptr & operator=( const dumb_ptr & other ) { body = other.body; return *this; }

      void release() { delete body; }

      T & operator*()  const { return *body; }
      T * operator->() const { return  body; }
      T * get()        const { return  body; }

   private:

      T * body;
   };

}

#endif
