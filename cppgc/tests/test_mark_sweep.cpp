#include <tut.h>
#include "algorithm/mark_sweep.h"
#include "gc.h"

using namespace windreamer::gc;

namespace tut
{
	typedef GarbageCollector<algorithm::MarkSweep<> > GC;
	struct MarksweepTestData
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

		struct InnerHandle
		{
			char buf[128];
			GC::Handle<InnerHandle> h;
		};
	};
	bool MarksweepTestData::exists=false;
	typedef test_group<MarksweepTestData> testGroup;
	typedef testGroup::object object;
	testGroup marksweepGroup("test_mark_sweep");




	template<>
	template<>
	void object::test<1>()
	{
		GC::Controller::forceGC();
		{
			GC::Handle<existing> h=new GC::Wrapper<existing>();
			ensure_equals(exists,true);
			ensure_equals(GC::Controller::getWrapperCount(),1);
			GC::Controller::forceGC();
			ensure_equals(GC::Controller::getWrapperCount(),1);
			ensure_equals(exists,true);
		}
		GC::Controller::forceGC();
		ensure_equals(GC::Controller::getWrapperCount(),0);
		ensure_equals(exists,false);
	}

	template<>
	template<>
	void object::test<2>()
	{
		GC::Handle<existing> h1=new GC::Wrapper<existing>();
		ensure_equals(h1.isRoot(),true);
		ensure_equals(h1.isRoot(),true);
		GC::Handle<InnerHandle> h2=new GC::Wrapper<InnerHandle>();
		ensure_equals(h2.isRoot(),true);
		ensure_equals(h2.isRoot(),true);
		ensure_equals(h2->h.isRoot(),false);
		ensure_equals(h2->h.isRoot(),false);
		ensure_equals(GC::Controller::getRootSet().size(),2);
	}

	template<>
	template<>
	void object::test<3>()
	{
		GC::Handle<InnerHandle> h1=new GC::Wrapper<InnerHandle>();
		ensure_equals(h1.getMemberHandles().size(),1);
		GC::Handle<existing> h2=new GC::Wrapper<existing>();
		ensure_equals(h2.getMemberHandles().size(),0);
	}

	template<>
	template<>
	void object::test<4>()
	{
		GC::Controller::forceGC();
		ensure_equals(GC::Controller::getWrapperCount(),0);
		ensure_equals(exists,false);
		{
				GC::Handle<InnerHandle> h=new GC::Wrapper<InnerHandle>();
				h->h=new GC::Wrapper<InnerHandle>();
				h->h->h=h->h;
				GC::Controller::forceGC();
				ensure_equals(GC::Controller::getWrapperCount(),2);
		}
		GC::Controller::forceGC();
		ensure_equals(GC::Controller::getWrapperCount(),0);
	}

}

