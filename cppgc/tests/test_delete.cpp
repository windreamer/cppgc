#include <tut.h>
#include "algorithm/reference_count.h"
#include "gc.h"

using namespace windreamer::gc;

namespace tut
{
	struct DeleteTestData
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
		
	};
	bool DeleteTestData::exists=false;
	typedef test_group<DeleteTestData> testGroup;
	typedef testGroup::object object;
	testGroup deleteGroup("test_delete");
	typedef GarbageCollector<algorithm::ReferenceCount<> > GC;



	template<>
	template<>
	void object::test<1>()
	{
		GC::Handle<existing> h=new GC::Wrapper<existing>();
		ensure_equals("Constructed", exists, true);
		h.~Handle<existing>();
		ensure_equals("All destructed", exists, false);
	}

	template<>
	template<>
	void object::test<2>()
	{
		GC::Handle<existing> h=new GC::Wrapper<existing>();
		GC::Handle<existing> h1=h;
		ensure_equals("Constructed", exists, true);
		h.~Handle<existing>();
		ensure_equals(exists, true);
	}
}
