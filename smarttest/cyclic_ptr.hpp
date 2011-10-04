////////////////////////////////////////////////////////////////////////////////
// cyclic_ptr.hpp
//
//  (C) Copyright Gregory Colvin 2000. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

#ifndef BOOST_CYCLIC_PTR_HPP
#define BOOST_CYCLIC_PTR_HPP

#include <boost/config.hpp>
#include <memory>          // std::auto_ptr

namespace boost {

   template<typename> struct weak_ptr;

   /////////////////////////////////////////////////////////////////////////////
   // Each handle refers to a deleter_base object, and cyclic_ptr constructor
   // takes an optional deleter argument.  Derive from deleter to customize
   // destroy function, but don't mess with recycle.

   struct deleter_base {
      virtual void destroy(void*)=0;
      virtual void recycle(void*)=0;
   };

   template<typename T> struct deleter : deleter_base {
      void destroy(void* p) { delete(reinterpret_cast<T*>(p)); }
      void recycle(void* p) { if (T* q = reinterpret_cast<T*>(p)) *q = *q; }
   };


   /////////////////////////////////////////////////////////////////////////////
   // Recycler handle class for cyclic_ptr and weak_ptr.
   // Collect cycles of cyclic_ptr by calling recycler::recycle().

   struct recycler {
      static void recycle() throw();

#if !defined( BOOST_NO_MEMBER_TEMPLATE_FRIENDS )
   private:
      template<typename> friend struct recyclable_base;
      template<typename> friend struct cyclic_ptr;
      template<typename> friend struct weak_ptr;
#endif

      recycler() : p(0),n(0),n_weak(0),action(0) {}
      void* p;
      long n, n_weak;
      deleter_base* action;
      void do_it();
      void decrement() {
         if (phase != FREE && --n == 0)
            dealloc(this);
      }
      void decrement_weak() {
         if (phase != FREE && --n_weak == 0 && n == 0)
            dealloc(this);
      }
      static recycler* alloc(void*,deleter_base*);
      static void dealloc(recycler*);
      static enum sweep_phase { FREE, MARK, SCAN, DONE } phase;
   };


   /////////////////////////////////////////////////////////////////////////////
   // Base class for cyclic_ptr and weak_ptr.
   // Initializes handle with appropriate deleter.

   template<typename T> class recyclable_base {

#if !defined( BOOST_NO_MEMBER_TEMPLATE_FRIENDS )
      template<typename> friend struct cyclic_ptr;
      template<typename> friend struct weak_ptr;
#else
   public:
#endif

      recycler* ph;
      T* p;
      recycler* new_handle(T* q,deleter_base* d) { return recycler::alloc(q,d);}
      static deleter<T>& default_deleter() { static deleter<T> r; return r; }
   };


   /////////////////////////////////////////////////////////////////////////////
   // The object pointed to is deleted when the last cyclic_ptr pointing to it
   // is destroyed, or when a recycler::recycle() call finds no cyclic_ptr to
   // the object outside a cycle.

   template<typename T> struct cyclic_ptr : recyclable_base<T> {

      typedef T element_type;

      long use_count() const throw() { return ph->n; }
      bool unique() const throw() { return use_count() == 1; }

      explicit
      cyclic_ptr(T* q=0,deleter<T>& r=default_deleter()) {
         p = q, ph = new_handle(q,&r), ph->n = 1;
      }

      cyclic_ptr(const cyclic_ptr& r) throw() {
         p = r.p, ph = r.ph, ++ph->n;
      }

      ~cyclic_ptr() { ph->decrement(); }

      cyclic_ptr& operator=(const cyclic_ptr& r) throw() {
         copy(r);
         return *this;
      }

      bool operator <( const cyclic_ptr & other ) const { return *p < *other.p; }

#if !defined(BOOST_NO_MEMBER_TEMPLATES)

      template<typename U> cyclic_ptr(const cyclic_ptr<U>& r) throw() {
         p = r.p, ph = r.ph, ++ph->n;
      }
      template<typename U> cyclic_ptr&
         operator=(const cyclic_ptr<U>& r) throw() {
            copy(r);
            return *this;
         }

      template<typename U> cyclic_ptr(std::auto_ptr<U>& r) {
         p = r.release(), ph = new_handle(p,&default_deleter()), ph->n = 1;
      }
      template<typename U> cyclic_ptr& operator=(std::auto_ptr<U>& r) {
         p = r.release(), ph = new_handle(p,&default_deleter()), ph->n = 1;
         return *this;
      }

      template<typename U> cyclic_ptr(const weak_ptr<U>&) throw();
      template<typename U> cyclic_ptr& operator=(const weak_ptr<U>&) 
throw();

#else

      cyclic_ptr(std::auto_ptr<T>& r) {
         p = r.release(), ph = new_handle(p,&default_deleter()), ph->n = 1;
      }
      cyclic_ptr& operator=(std::auto_ptr<T>& r) {
         p = r.release(), ph = new_handle(p,&default_deleter()), ph->n = 1;
         return *this;
      }

      cyclic_ptr(const weak_ptr<T>&) throw();
      cyclic_ptr& operator=(const weak_ptr<T>&) throw();

#endif

      void reset(T* ptr=0,deleter<T>& r=default_deleter()) {
         if (--ph->n == 0)
            recycler::dealloc(ph);
         ph = new_handle(ptr,&r), ph->n = 1;
      }

      T* get()        const throw() { return p; }
      T& operator*()  const throw() { return *get(); }
      T* operator->() const throw() { return get(); }
      operator T*()   const throw() { return get(); }

   private:

#if !defined(BOOST_NO_MEMBER_TEMPLATES)
      template<typename U> void copy(const recyclable_base<U>& r) {
         if (ph != r.ph) {
            this->~cyclic_ptr();
            p = r.p;
            ph = r.ph;
            ++ph->n;
         } else
            ph->do_it();   // has no effect except during recycling
      }
#else
      void copy(const recyclable_base<T>& r) {
         if (ph != r.ph) {
            this->~cyclic_ptr();
            p = r.p;
            ph = r.ph;
            ++ph->n;
         } else
            ph->do_it();   // has no effect except during recycling
      }
#endif
   };


   /////////////////////////////////////////////////////////////////////////////
   //   A weak_ptr does not prevent a shared object from being destroyed, but
   //   when it is destroyed weak_ptr::get() returns 0.

   template<typename T> struct weak_ptr : recyclable_base<T> {

      typedef T element_type;

      weak_ptr() { p = 0, ph = new_handle(0,0), ph->n_weak = 1; }

      weak_ptr(const weak_ptr& r) throw() { p = r.p, ph = r.ph, ++ph->n_weak; }


      ~weak_ptr() { if (ph) ph->decrement_weak(); }

      weak_ptr& operator=(const weak_ptr& r) throw() {
         copy(r);
         return *this;
      }

#if !defined(BOOST_NO_MEMBER_TEMPLATES)

      template<typename U> weak_ptr(const weak_ptr<U>& r) throw() {
         p = r.p, ph = r.ph, ++ph->n_weak;
      }
      template<typename U> weak_ptr& operator=(const weak_ptr<U>& r) throw() {
         copy(r);
         return *this;
      }

      template<typename U> weak_ptr(const cyclic_ptr<U>& r) throw() {
         p = r.p, ph = r.ph, ++ph->n_weak;
      }
      template<typename U> weak_ptr&
         operator=(const cyclic_ptr<U>& r) throw() {
            copy(r);
            return *this;
         }

#else

      weak_ptr(const cyclic_ptr<T>& r) throw() {
         p = r.p, ph = r.ph, ++ph->n_weak;
      }
      weak_ptr& operator=(const cyclic_ptr<T>& r) throw() {
         copy(r);
         return *this;
      }

#endif

      T* get()        const throw() { return ph->n ? p : 0; }
      T& operator*()  const throw() { return *get(); }
      T* operator->() const throw() { return get(); }
      operator T*()   const throw() { return get(); }


   private:

#if !defined(BOOST_NO_MEMBER_TEMPLATES)
      template<typename U> void copy(const recyclable_base<U>& r) {
         this->~weak_ptr();
         p = r.p;
         ph = r.ph;
         ++ph->n_weak;
      }
#else
      void copy(const recyclable_base<T>& r) {
         this->~weak_ptr();
         p = r.p;
         ph = r.ph;
         ++ph->n_weak;
      }
#endif
   };

#if !defined(BOOST_NO_MEMBER_TEMPLATES)
   template<typename T> template<typename U>
      inline cyclic_ptr<T>::cyclic_ptr(const weak_ptr<U>& r) throw() {
          p = r.p, ph = r.ph, ++ph->n;
      }
   template<typename T> template<typename U>
      inline cyclic_ptr<T>&
         cyclic_ptr<T>::operator=(const weak_ptr<U>& r) throw() {
            copy(r);
            return *this;
         }
#else
   template<typename T>
      inline cyclic_ptr<T>::cyclic_ptr(const weak_ptr<T>& r) throw() {
          p = r.p, ph = r.ph, ++ph->n;
      }
   template<typename T>
      inline cyclic_ptr<T>&
         cyclic_ptr<T>::operator=(const weak_ptr<T>& r) throw() {
            copy(r);
            return *this;
         }
#endif

}

#endif


