////////////////////////////////////////////////////////////////////////////////
// cyclic_ptr.cpp
//
//  (C) Copyright Gregory Colvin 2000. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//
//  Revision History

#include "cyclic_ptr.hpp"
#include <memory>
#include <deque>
#ifndef NDEBUG
   #include <set>
#endif

#include <cassert>

namespace boost {

   recycler::sweep_phase recycler::phase = DONE;

   namespace {
      std::deque<recycler> handles;
      recycler* free_handle = 0;
      #ifndef NDEBUG
         std::set<void*> no_duplicates;
      #endif
   }

   // maintain pool of available handles in a deque
   recycler* recycler::alloc(void* p,deleter_base* action) {
      assert(!p || no_duplicates.insert(p).second == true);
      recycler* ph;
      if (free_handle) {
         ph = free_handle;
         free_handle = reinterpret_cast<recycler*>(ph->p);
      } else {
          handles.push_back(recycler());
          ph = &*(handles.end() - 1);
      }
      ph->p = p;
      ph->action = action;
      return ph;
   }
   void recycler::dealloc(recycler* ph) {
      if (ph->p) {
         assert(no_duplicates.erase(ph->p) == 1);
         assert(ph->n == 0);
         ph->action->destroy(ph->p);
         ph->p = 0;
      }
      if (ph->n_weak == 0) {
         ph->p = free_handle;
         free_handle = ph;
      }
      ph->action = 0;
   }

   // traverse all handles in deque three times to collect garbage
   void recycler::recycle() throw() {
      std::deque<recycler>::iterator i, end=handles.end();

      // reduce counts to find root pointers
      phase = SCAN;
      for (i=handles.begin(); i != end; ++i)
         if (i->action)
            i->action->recycle(i->p);

      // recursively mark all handles reachable from roots
      phase = MARK;
      for (i=handles.begin(); i != end; ++i)
         if (i->action && i->n > 0)
            i->action->recycle(i->p);

      // destroy unreachable objects and restore counts for the rest
      phase = FREE;
      for (i=handles.begin(); i != end; ++i)
         if (i->action)
            if (i->n == 0)
               dealloc(&*i);
            else
               i->action->recycle(i->p);

      phase = DONE;
   }

   void recycler::do_it() {
      switch (phase) {
      case SCAN:
         --n;                 // reduce count
         break;
      case MARK:
         n = -1;              // mark as root or reachable from root
         assert(action);
         action->recycle(p);  // recurse for cyclic_ptr members of this
         break;
      case FREE:
         if (n < 0)
            n = 1;            // restore count for marked object
         else if (n > 0)
            ++n;              // restore count
         break;
      case DONE:
         break;
      }
   }
}
