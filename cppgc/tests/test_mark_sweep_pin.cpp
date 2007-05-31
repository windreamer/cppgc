#include <tut.h>
#include "algorithm/mark_sweep.h"
#include "gc.h"

using namespace windreamer::gc;

namespace tut
{
	typedef GarbageCollector<algorithm::MarkSweep<> > GC;
	struct MarksweepPinTestData
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
	bool MarksweepPinTestData::exists=false;
	typedef test_group<MarksweepPinTestData> testGroup;
	typedef testGroup::object object;
	testGroup marksweeppinGroup("test_mark_sweep_pin");




	template<>
	template<>
	void object::test<1>()
	{
		{
			GC::Handle<existing> h=new GC::Wrapper<existing>();
			GC::Handle<existing>::Pin pin(h);
			GC::Controller::forceGC();
			ensure_equals(exists,true);
		}
		GC::Controller::forceGC();
		ensure_equals(exists,false);
	}

}

