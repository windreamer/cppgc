#include <tut.h>
#include <memory>
#include "algorithm/mark_sweep.h"
#include "trigger/new_fail.h"
#include "gc.h"

using namespace windreamer::gc;

namespace tut
{
	static bool alwaysThrow=false;
	struct NewFailTestData
	{
		static bool exists;
		struct existing
		{
			existing()
			{
				exists=true;
			}
			~existing()
			{
				exists=false;
			}
		};
	};

	template <class T> 
	class test_allocator
	{
	public:

		void* allocate(size_t n, const void* = 0) {
			static bool first=true;
			if(first)
			{
				first=(!first)||alwaysThrow;
				throw std::bad_alloc();
			}
			void* p = std::malloc(n * sizeof(T));
			if (!p)
				throw std::bad_alloc();
			first=(!first)||alwaysThrow;
			return p;
		}
		void deallocate(void* p, size_t) {
			std::free(p);
		}
	};

	bool NewFailTestData::exists=false;
	typedef test_group<NewFailTestData> testGroup;
	typedef testGroup::object object;
	testGroup newfailGroup("test_new_fail_trigger");
	typedef GarbageCollector<algorithm::MarkSweep<triggers::NewFail,test_allocator> > GC;

	template<>
	template<>
	void object::test<1>()
	{
		bool succeeded=false;
		{
			GC::Handle<existing> h=new GC::Wrapper<existing>();
		}
		GC::Handle<existing> h=new GC::Wrapper<existing>();
		ensure_equals(GC::Controller::getWrapperCount(),1);
		alwaysThrow=true;
		try
		{
			GC::Handle<existing> h=new GC::Wrapper<existing>();
		}
		catch (const std::bad_alloc&)
		{
			succeeded=true;
		}
		ensure_equals(succeeded,true);
	}
}
