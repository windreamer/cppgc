#include <tut/tut.hpp>
#include "algorithm/mark_sweep.h"
#include "trigger/counting.h"
#include "gc.h"

using namespace windreamer::gc;

namespace tut
{
	struct CountingTestData
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
	bool CountingTestData::exists=false;
	typedef test_group<CountingTestData> testGroup;
	typedef testGroup::object object;
	testGroup countingGroup("test_counting_trigger");
	typedef GarbageCollector<algorithm::MarkSweep<triggers::Counting<50> > > GC;

	template<>
	template<>
	void object::test<1>()
	{
		for(int i=1;i<50;i++)//49 times
			GC::Handle<existing> h=new GC::Wrapper<existing>();
		ensure_equals(GC::Controller::getWrapperCount(),49);
		GC::Handle<existing> h=new GC::Wrapper<existing>();
		ensure_equals(GC::Controller::getWrapperCount(),1);
	}
}
