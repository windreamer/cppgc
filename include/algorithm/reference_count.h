#ifndef __REFERENCE_COUNT_H__
#define __REFERENCE_COUNT_H__
#include <new>
#include "thread/single_thread.h"
#include "trigger/trigger_common.h"
namespace windreamer
{
	namespace gc
	{
		namespace algorithm
		{
			template < template<typename> class Allocator=std::allocator, typename ThreadingModel=thread::policy::SingleThread>
			struct ReferenceCount
			{
				template <typename U>
				static Allocator<U> makeAllocator()
				{
					return Allocator<U>();
				}
				template <triggers::TRIGGER_POINT>
				static void trigger()
				{
					//do nothing
				}

				typedef typename ThreadingModel::Lock Lock;
				struct Handle;
				struct Controller {};
				struct Wrapper
				{
					//所有引用计数对象的根对象
					Wrapper():count(0){}
					virtual ~Wrapper(){}
				protected:
					static void check_delete(Wrapper* ptr)
					{
						if(ptr->count==0)
							delete ptr;
					}
				private:
					long count;
					friend struct Handle;
					virtual size_t getSize()=0;

				};
				struct Handle
				{
					//引用计数的基类,模仿void*
					Handle(Wrapper* p=0)
						:ptr(p)
					{
						increase();
					}
					Handle(const Handle& rhs)
						:ptr(0)
					{
						copy(rhs);
					}
					virtual ~Handle()
					{
						decrease();
						ptr=0;
					}
					bool operator==(const Handle& rhs) const
					{
						return ptr==rhs.ptr;
					}
					int count() const
					{
						if(ptr==0) return -1;
						return ptr->count;
					}
				protected:
					void copy (const Handle & rhs)
					{
						decrease();
						ptr=rhs.ptr;
						increase();
					}
					Wrapper* ptr;
				private:
					void decrease()
					{
						Lock lock;

						if (ptr==0) return;
						--ptr->count;
						Wrapper::check_delete(ptr);

					}
					void increase()
					{
						Lock lock;

						if(ptr==0) return;
						++ptr->count;
					}
				};
			};
		}
	}
}
#endif//__REFERENCE_COUNT_H__

