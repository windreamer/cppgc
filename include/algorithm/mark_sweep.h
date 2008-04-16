#ifndef __MARK_SWEEP_H__
#define __MARK_SWEEP_H__
#include <new>
#include <cstdlib>
#include <set>
#include <stack>
#include <algorithm>
#include "thread/single_thread.h"
#include "trigger/trigger_common.h"
#include "trigger/manual.h"
namespace windreamer
{
	namespace gc
	{
		namespace algorithm
		{
			template <typename Trigger=triggers::Manual,template<typename> class Allocator=std::allocator, typename ThreadingModel=thread::policy::SingleThread>
			struct MarkSweep
			{
				typedef typename ThreadingModel::Lock Lock;
				struct HandleBase;

				struct Controller
				{

					static std::set<int> getRootSet()
					{
						std::set<int> result;
						std::set<int>::const_iterator begin=HandleBase::getRegistry().begin();
						std::set<int>::const_iterator end=HandleBase::getRegistry().end();
						std::remove_copy_if(begin,end,std::insert_iterator<std::set<int> >(result,result.begin()),isNotRoot);
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
					static bool isNotRoot(int i)
					{
						return !((reinterpret_cast<HandleBase*>(i))->isRoot());
					}

					static void mark()
					{
						std::stack<int> s;
						std::set<int> roots=getRootSet();
						for (std::set<int>::const_iterator iter=roots.begin();iter!=roots.end();++iter)
						{
							s.push(*iter);
						}
						while(!s.empty())
						{
							int address=s.top();
							s.pop();
							HandleBase* ptr=reinterpret_cast<HandleBase*>(address);
							ptr->ptr->alive=true;
							std::set<int> members=ptr->getMemberHandles();
							for (std::set<int>::const_iterator iter=members.begin();iter!=members.end();++iter)
							{
								if (!reinterpret_cast<HandleBase*>(*iter)->ptr->alive)
								{
									s.push(*iter);
								}
							}

						}


					}

					static void sweep()
					{
						for (std::set<int>::const_iterator it = WrapperBase::getRegistry().begin(); it != WrapperBase::getRegistry().end();)
						{
							WrapperBase* ptr=reinterpret_cast<WrapperBase*>(*it);
							it++;
							WrapperBase::check_delete(ptr);
						}

					}
					static void reset()
					{
						std::for_each(WrapperBase::getRegistry().begin(),WrapperBase::getRegistry().end(),resetAlive);
					}

					static void destroyDead(int p)
					{
						WrapperBase* ptr=reinterpret_cast<WrapperBase*>(p);
						if(!ptr->alive)	delete ptr;
					}

					static void resetAlive(int p)
					{
						WrapperBase* ptr=reinterpret_cast<WrapperBase*>(p);
						ptr->alive=false;
					}

				};

				template <typename U>
				static Allocator<U> makeAllocator()
				{
					return Allocator<U>();
				}
				template<triggers::TRIGGER_POINT t>
				static void trigger()
				{
					if (Trigger::test(t))
					{
						Controller::forceGC();
					}
				}

				template <typename T>
				struct Registry
				{
					void insert(T* t)
					{
						Lock lock;
						void* ptr=t;
						s.insert(reinterpret_cast<int>(ptr));
					}

					void erase(T* t)
					{
						Lock lock;
						void* ptr=t;
						s.erase(reinterpret_cast<int>(ptr));
					}

					std::set<int>::const_iterator upper_bound(const void* ptr)
					{
						return s.upper_bound(reinterpret_cast<int>(ptr));
					}


					std::set<int>::const_iterator lower_bound(const void* ptr)
					{
						return s.lower_bound(reinterpret_cast<int>(ptr));
					}

					std::set<int>::const_iterator begin()
					{
						return s.begin();
					}

					std::set<int>::const_iterator end()
					{
						return s.end();
					}

					int size()
					{
						return s.size();
					}
				private:
					std::set<int> s;
				};

				struct WrapperBase
				{
					WrapperBase()
						:alive(false)
					{
						Registry<WrapperBase>& reg=getRegistry();
						reg.insert(this);
					}
					virtual ~WrapperBase()
					{
						Registry<WrapperBase>& reg=getRegistry();
						reg.erase(this);
					}
				protected:
					static void check_delete(WrapperBase * ptr)
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
					virtual size_t getSize()=0;
				};

				struct HandleBase{
					HandleBase(WrapperBase* p=0)
						:ptr(p),root(-1)
					{
						Registry<HandleBase>& reg=getRegistry();
						static int temp(std::atexit(Controller::forceGC));
						reg.insert(this);
					}
					virtual ~HandleBase()
					{
						Registry<HandleBase>& reg=getRegistry();
						reg.erase(this);
						ptr=0;
					}
					HandleBase(const HandleBase& rhs)
						:ptr(0),root(-1)
					{
						copy(rhs);
					}
					bool operator==(const HandleBase& rhs) const
					{
						return ptr==rhs.ptr;
					}
					bool isRoot() const
					{
						if(root==-1)
						{
							int size=0;
							Registry<WrapperBase>& reg=WrapperBase::getRegistry();
							int nthis=reinterpret_cast<int>(this);
							std::set<int>::const_iterator i=reg.upper_bound(this);
							if (i==reg.begin())
							{
								root=1;
								return true;
							} 
							else
							{
								int address=*(--i);
								WrapperBase* p=reinterpret_cast<WrapperBase*>(address);
								size=p->getSize();
								bool result= nthis>(address+size);
								root=result;
								return result;
							}
						}
						return (root!=0);
					}

					std::set<int> getMemberHandles() const
					{
						int address=reinterpret_cast<int>(ptr);
						int size=ptr->getSize();
						std::set<int> result;
						std::set<int>::const_iterator begin=getRegistry().lower_bound(ptr);
						std::set<int>::const_iterator end=getRegistry().upper_bound(reinterpret_cast<void*>(address+size));
						std::copy(begin,end,std::insert_iterator<std::set<int> >(result,result.begin()));
						return result;
					}

				protected:
					void copy(const HandleBase & rhs)
					{
						ptr=rhs.ptr;
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
				};
			private:
			};
		}
	}
}
#endif//__MARK_SWEEP_H__
