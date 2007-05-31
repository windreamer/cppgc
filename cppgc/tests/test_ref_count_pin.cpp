#include <tut.h>
#include "algorithm/reference_count.h"
#include "gc.h"

using namespace windreamer::gc;

namespace tut
{
	typedef GarbageCollector<algorithm::ReferenceCount<> > GC;
	struct ReferenceCountPinTestData
	{
		static bool exists;
		struct existing
		{
			int n;
			existing()
			{
				exists=true;
			}
			~existing()
			{
				exists=false;
			}
		};

		struct Big
		{
			GC::Handle<existing> h;
		};

	};
	bool ReferenceCountPinTestData::exists=false;
	typedef test_group<ReferenceCountPinTestData> testGroup;
	typedef testGroup::object object;
	testGroup referenceCountPinGroup("test_ref_count_pin");




	template<>
	template<>
	void object::test<1>()
	{
		{
			Big* pb=new Big();
			pb->h=new GC::Wrapper<existing>();
			GC::Handle<existing>::Pin pin(pb->h);
			delete pb;
			ensure_equals("pinned",exists,true);
		}
		ensure_equals("unpinned",exists,false);
	}

	template<>
	template<>
	void object::test<2>()
	{
			Big* pb=new Big();
			pb->h=new GC::Wrapper<existing>();
			delete pb;
			ensure_equals("pinned",exists,false);
	}


}
