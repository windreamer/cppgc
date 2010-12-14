#ifndef __MARK_SWEEP_H__
#define __MARK_SWEEP_H__

#include "thread/single_thread.h"
#include "trigger/manual.h"
#include "trigger/trigger_common.h"

#include <stdint.h>
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <new>
#include <set>

namespace windreamer {
namespace gc {
namespace algorithm {

template <
    typename Trigger = triggers::Manual,
    template<typename> class Allocator = std::allocator,
    typename ThreadingModel = thread::policy::SingleThread
>
struct MarkSweep
{
    typedef uintptr_t AddressType;

    typedef std::set<AddressType> AddressSet;

    typedef typename ThreadingModel::Lock Lock;

    struct HandleBase;
    struct WrapperBase;

    struct Controller
    {

        static AddressSet getRootSet()
        {
            AddressSet result;

            std::remove_copy_if(
                    HandleBase::getRegistry().begin(),
                    HandleBase::getRegistry().end(),
                    std::inserter(result, result.begin()),
                    isNotRoot);

            return result;
        }

        static int getWrapperCount()
        {
            return WrapperBase::getRegistry().size();
        }

        static void forceGC()
        {
            mark();
            sweep();
            reset();
        }

    private:
        static bool isNotRoot(AddressType i)
        {
            return !(reinterpret_cast<HandleBase*>(i)->isRoot());
        }

        static void mark()
        {
            std::deque<AddressType> stack;
            AddressSet roots = getRootSet();

            std::copy(roots.begin(), roots.end(), std::front_inserter(stack));

            while(!stack.empty())
            {
                AddressType address = stack.front();
                stack.pop_front();

                HandleBase* ptr = reinterpret_cast<HandleBase*>(address);
                ptr->ptr->alive = true;

                AddressSet members = ptr->getMemberHandles();

                for (AddressSet::const_iterator iter = members.begin(); iter != members.end(); ++iter)
                {
                    if (!reinterpret_cast<HandleBase*>(*iter)->ptr->alive)
                    {
                        stack.push_front(*iter);
                    }
                }
            }
        }

        static void sweep()
        {
            for (AddressSet::const_iterator it = WrapperBase::getRegistry().begin();
                    it != WrapperBase::getRegistry().end();)
            {
                WrapperBase* ptr = reinterpret_cast<WrapperBase*>(*it);
                it++;
                WrapperBase::check_delete(ptr);
            }
        }

        static void reset()
        {
            std::for_each(WrapperBase::getRegistry().begin(), WrapperBase::getRegistry().end(), resetAlive);
        }

        static void destroyDead(AddressType p)
        {
            WrapperBase* ptr = reinterpret_cast<WrapperBase*>(p);
            if (!ptr->alive)
                delete ptr;
        }

        static void resetAlive(AddressType p)
        {
            reinterpret_cast<WrapperBase*>(p)->alive = false;
        }

    };  //  struct Controller

    template <typename U>
    static Allocator<U> makeAllocator()
    {
        return Allocator<U>();
    }

    template<typename T>
    static void trigger(const T& t)
    {
        if (Trigger::test(t))
        {
            Controller::forceGC();
        }
    }

    template <typename T>
    struct Registry
    {
        virtual ~Registry()
        {
            Controller::forceGC();
        }
        void insert(T* t)
        {
            Lock lock;
            void* ptr = t;
            s.insert(reinterpret_cast<AddressType>(ptr));
        }

        void erase(T* t)
        {
            Lock lock;
            void* ptr=t;
            s.erase(reinterpret_cast<AddressType>(ptr));
        }

        AddressSet::const_iterator upper_bound(const void* ptr)
        {
            return s.upper_bound(reinterpret_cast<AddressType>(ptr));
        }

        AddressSet::const_iterator lower_bound(const void* ptr)
        {
            return s.lower_bound(reinterpret_cast<AddressType>(ptr));
        }

        AddressSet::const_iterator begin()
        {
            return s.begin();
        }

        AddressSet::const_iterator end()
        {
            return s.end();
        }

        std::size_t size()
        {
            return s.size();
        }

    private:
        AddressSet s;

    };

    struct WrapperBase
    {
        WrapperBase()
            :alive(false)
        {
            Registry<WrapperBase>& reg = getRegistry();
            reg.insert(this);
        }

        virtual ~WrapperBase()
        {
            Registry<WrapperBase>& reg = getRegistry();
            reg.erase(this);
        }

    protected:
        static void check_delete(WrapperBase* ptr)
        {
            if (!ptr->alive)
            {
                delete ptr;
            }
        }

    private:
        bool alive;

        friend struct HandleBase;
        friend struct Controller;

        static Registry<WrapperBase>& getRegistry()
        {
            static Registry<WrapperBase> reg;
            return reg;
        }

        virtual size_t getSize() = 0;

    };

    struct HandleBase{
        HandleBase(WrapperBase* p = 0) :
            ptr(p),
            root(-1)
        {
            Registry<HandleBase>& reg = getRegistry();
            reg.insert(this);
        }

        virtual ~HandleBase()
        {
            Registry<HandleBase>& reg = getRegistry();
            reg.erase(this);
            ptr = 0;
        }

        HandleBase(const HandleBase& rhs) :
            ptr(0),
            root(-1)
        {
            copy(rhs);
        }

        bool operator==(const HandleBase& rhs) const
        {
            return ptr == rhs.ptr;
        }

        bool isRoot() const
        {
            if (root == -1)
            {
                int size = 0;
                Registry<WrapperBase>& reg = WrapperBase::getRegistry();
                AddressType nthis = reinterpret_cast<AddressType>(this);

                AddressSet::const_iterator i = reg.upper_bound(this);
                if (i == reg.begin())
                {
                    root = 1;
                    return true;
                } 
                else
                {
                    AddressType address = *(--i);
                    WrapperBase* p = reinterpret_cast<WrapperBase*>(address);
                    size = p->getSize();
                    bool result = nthis > (address + size);
                    root = result;
                    return result;
                }
            }

            return (root != 0);
        }

        AddressSet getMemberHandles() const
        {
            AddressType address = reinterpret_cast<AddressType>(ptr);
            std::size_t size = ptr->getSize();

            AddressSet result;

            std::copy(
                    getRegistry().lower_bound(ptr),
                    getRegistry().upper_bound(reinterpret_cast<void*>(address + size)),
                    std::inserter(result, result.begin()));

            return result;
        }

    protected:
        void copy(const HandleBase & rhs)
        {
            ptr = rhs.ptr;
        }

        WrapperBase* ptr;

    private:
        mutable int root;

        static Registry<HandleBase>& getRegistry()
        {
            static Registry<HandleBase> reg;
            return reg;
        }

        friend struct Controller;

    };  //  struct HandleBase

};  //  struct MarkSweep

}   //  namespace algorithm
}   //  namespace gc
}   //  namespace windreamer

#endif//__MARK_SWEEP_H__
