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
				template <typename Tag>
				static void trigger(const Tag&)
				{
					//do nothing
				}

				typedef typename ThreadingModel::Lock Lock;
				struct HandleBase;
				struct Controller {};
				struct WrapperBase
				{
					//所有引用计数对象的根对象
					WrapperBase():count(0){}
					virtual ~WrapperBase(){}
				protected:
					static void check_delete(WrapperBase* ptr)
					{
						if(ptr->count==0)
							delete ptr;
					}
				private:
					size_t count;
					friend struct HandleBase;
					virtual size_t getSize()=0;

				};
				struct HandleBase
				{
					//引用计数的基类,模仿void*
					HandleBase(WrapperBase* p=0)
						:ptr(p)
					{
						increase();
					}
					HandleBase(const HandleBase& rhs)
						:ptr(0)
					{
						copy(rhs);
					}
					virtual ~HandleBase()
					{
						decrease();
						ptr=0;
					}
					bool operator==(const HandleBase& rhs) const
					{
						return ptr==rhs.ptr;
					}
					int count() const
					{
						if(ptr==0) return -1;
						return ptr->count;
					}
				protected:
					void copy (const HandleBase & rhs)
					{
						decrease();
						ptr=rhs.ptr;
						increase();
					}
					WrapperBase* ptr;
				private:
					void decrease()
					{
						Lock lock;

						if (ptr==0) return;
						--ptr->count;
						WrapperBase::check_delete(ptr);

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

