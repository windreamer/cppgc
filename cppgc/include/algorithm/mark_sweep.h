#ifndef __MARK_SWEEP_H__
#define __MARK_SWEEP_H__
#include <new>
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
				template <typename U>
				static Allocator<U> makeAllocator()
				{
					return Allocator<U>();
				}

				typedef typename ThreadingModel::Lock Lock;
				struct Handle;

				struct Controller
				{
					template<triggers::TRIGGER_POINT t>
					static void trigger()
					{
						if (Trigger::test(t))
						{
							forceGC();
						}
					}
					static std::set<int> getRootSet()
					{
						std::set<int> result;
						std::set<int>::const_iterator begin=Handle::getRegistry().begin();
						std::set<int>::const_iterator end=Handle::getRegistry().end();
						std::remove_copy_if(begin,end,std::insert_iterator<std::set<int> >(result,result.begin()),isNotRoot);
						return result;
					}
					static int getWrapperCount()
					{
						return Wrapper::getRegistry().size();
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
						return !((reinterpret_cast<Handle*>(i))->isRoot());
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
							Handle* ptr=reinterpret_cast<Handle*>(address);
							ptr->ptr->alive=true;
							std::set<int> members=ptr->getMemberHandles();
							for (std::set<int>::const_iterator iter=members.begin();iter!=members.end();++iter)
							{
								if (!reinterpret_cast<Handle*>(*iter)->ptr->alive)
								{
									s.push(*iter);
								}
							}

						}
						

					}

					static void sweep()
					{
						for (std::set<int>::const_iterator it = Wrapper::getRegistry().begin(); it != Wrapper::getRegistry().end();)
						{
							Wrapper* ptr=reinterpret_cast<Wrapper*>(*it);
							it++;
							Wrapper::check_delete(ptr);
						}

					}
					static void reset()
					{
						std::for_each(Wrapper::getRegistry().begin(),Wrapper::getRegistry().end(),resetAlive);
					}

					static void destroyDead(int p)
					{
						Wrapper* ptr=reinterpret_cast<Wrapper*>(p);
						if(!ptr->alive)	delete ptr;
					}

					static void resetAlive(int p)
					{
						Wrapper* ptr=reinterpret_cast<Wrapper*>(p);
						ptr->alive=false;
					}

				};

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

				struct Wrapper
				{
					Wrapper()
						:alive(false)
					{
						Registry<Wrapper>& reg=getRegistry();
						reg.insert(this);
					}
					virtual ~Wrapper()
					{
						Registry<Wrapper>& reg=getRegistry();
						reg.erase(this);
					}
				protected:
					static void check_delete(Wrapper * ptr)
					{
						if (!ptr->alive)
						{
							delete ptr;
						}
					}
				private:
					bool alive;
					friend struct Handle;
					friend struct Controller;
					static Registry<Wrapper>& getRegistry()
					{
						static Registry<Wrapper> reg;
						return reg;
					}
					virtual size_t getSize()=0;
				};

				struct Handle{
					Handle(Wrapper* p=0)
						:ptr(p),root(-1)
					{
						Registry<Handle>& reg=getRegistry();
						reg.insert(this);
					}
					virtual ~Handle()
					{
						Registry<Handle>& reg=getRegistry();
						reg.erase(this);
						ptr=0;
					}
					Handle(const Handle& rhs)
						:ptr(0),root(-1)
					{
						copy(rhs);
					}
					bool operator==(const Handle& rhs) const
					{
						return ptr==rhs.ptr;
					}
					bool isRoot() const
					{
						if(root==-1)
						{
							int size=0;
							Registry<Wrapper>& reg=Wrapper::getRegistry();
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
								Wrapper* p=reinterpret_cast<Wrapper*>(address);
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
					void copy(const Handle & rhs)
					{
						ptr=rhs.ptr;
					}
					Wrapper* ptr;
				private:
					mutable int root;
					static Registry<Handle>& getRegistry()
					{
						static Registry<Handle> reg;
						return reg;
					}
					friend struct Controller;
				};

			};
		}
	}
}
#endif//__MARK_SWEEP_H__
