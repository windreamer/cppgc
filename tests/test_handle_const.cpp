#include <tut.h>
#include "algorithm/mark_sweep.h"
#include "gc.h"

using namespace windreamer::gc;

namespace tut
{
	struct HandleConstTestData
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
	bool HandleConstTestData::exists=false;
	typedef test_group<HandleConstTestData> testGroup;
	typedef testGroup::object object;
	testGroup handleConstGroup("test_handle_const");
	typedef GarbageCollector<algorithm::MarkSweep<> > GC;



	template<>
	template<>
	void object::test<1>()
	{
		{
			GC::Handle<existing> const h=new GC::Wrapper<existing>();
			ensure(GC::Controller::getWrapperCount()==1);
			ensure(h.operator ->() !=0);
			ensure_equals("Constructed", exists, true);
		}
		GC::Controller::forceGC();
		ensure_equals("All destructed", exists, false);
	}

	template<>
	template<>
	void object::test<2>()
	{
		{
			GC::Handle<existing> const h1=new GC::Wrapper<existing>();
			GC::Handle<existing> const h2=h1;
			ensure_equals("Copy ctor made ptr the same", h1.operator ->(), h2.operator ->());
		}
		GC::Controller::forceGC();
		ensure_equals("All destructed", exists, false);
	}


	template<>
	template<>
	void object::test<3>()
	{

		GC::Handle<existing> const h;
		existing* ptr=h.operator ->();
		ensure("Default ctor made ptr 0", ptr==0);
	}

	template<>
	template<>
	void object::test<4>()
	{

		GC::Handle<existing> const h=new GC::Wrapper<existing>();
		existing* ptr=h.operator ->();
		h->n=5;
		ensure_equals("Handle could access member", ptr->n,5);
	}

	template<>
	template<>
	void object::test<5>()
	{

		GC::Handle<existing> const h1=new GC::Wrapper<existing>();
		GC::Handle<existing> const h2=h1;
		ensure("The operator == worked", h1==h2);
	}
}
