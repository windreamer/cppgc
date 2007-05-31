#ifndef __GC__H__
#define __GC__H__
#include <memory>
#include "trigger/trigger_common.h"
#include "traits/conversion.h"
#include "traits/static_check.h"
namespace windreamer
{
	namespace gc
	{
		template <typename Algorithm> 
		class GarbageCollector
		{
			typedef Algorithm Algo;
			typedef typename Algo::Lock Lock;
		public:
			typedef typename Algo::Handle Void;
			typedef typename Algo::Controller Controller;

			template <typename T> class Handle;
			template <typename T> class Wrapper;

			//可回收对象包装类
			template <typename T>
			class Wrapper
				:public Algo::Wrapper
			{
				~Wrapper(){}
			public:
				Wrapper(){}
				explicit Wrapper(const T& rhs):t(rhs){}
				//placement new
				static void* operator new (size_t size) 
				{
					Lock lock;
					Controller::template trigger<triggers::EVERY_NEW_START>();
					void* result;
					try
					{
						result=Algo::template makeAllocator<Wrapper>().allocate(1);
					}
					catch (const std::bad_alloc&)
					{
						Controller::template trigger<triggers::OUT_OF_MEMORY>();
						result=Algo::template makeAllocator<Wrapper>().allocate(1);
					}
					return result;
				}
				//placement delete
				static void operator delete(void* p) 
				{
					Lock lock;
					Algo::template makeAllocator<Wrapper>().deallocate(static_cast<Wrapper*>(p),1);
				}
				friend class Handle<T>;
			private:
				T t;
				virtual size_t getSize()
				{
					return sizeof(*this);
				}

			};


			template <typename T>
			class Handle
				:public Algo::Handle
			{
			public:
				Handle()
					:Algo::Handle(0)
				{}
				Handle(Wrapper<T>* p)
					:Algo::Handle(p)
				{}
				Handle(const Handle& rhs)
					:Algo::Handle(rhs)
				{}
				Handle& operator =(const Handle& rhs)
				{
					if (this==&rhs) return *this;
					Algo::Handle::copy(rhs);
					return *this;
				}
				T& operator *() const
				{
					if (Algo::Handle::ptr==0)
					{
						return 0;
					}
					return static_cast<Wrapper<T>*>(Algo::Handle::ptr)->t;
				}
				T* operator ->() const
				{
					if (Algorithm::Handle::ptr==0)
					{
						return 0;
					}
					return &(static_cast<Wrapper<T>*>(Algo::Handle::ptr)->t);
				}

				template <typename U>
				operator Handle<U>()  const
				{
					sizeof(traits::CompileTimeChecker<traits::Conversion<T*,U*>::exists>);
					return Handle<U>(static_cast<Wrapper<U>* >(Algo::Handle::ptr));
				}

				//TODO:more operators
				class Pin
				{
					Handle handle;
					Pin(const Pin&);
					Pin& operator =(const Pin&);
				public:
					explicit Pin(const Handle& h)
						:handle(h)
					{
					}
					~Pin()
					{
					}
					T* getPtr()
					{
						return handle.operator->();

					}
				};
			};
		};
	}
}
#endif//__GC__H__
