#include <tut.h>
#include "algorithm/reference_count.h"
#include "gc.h"

using namespace windreamer::gc;

namespace tut
{
	struct ReferenceCountTestData
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
	bool ReferenceCountTestData::exists=false;
	typedef test_group<ReferenceCountTestData> testGroup;
	typedef testGroup::object object;
	testGroup referenceCountGroup("test_ref_count");
	typedef GarbageCollector<algorithm::ReferenceCount<> > GC;



	template<>
	template<>
	void object::test<1>()
	{
		{
			GC::Handle<existing> h=new GC::Wrapper<existing>();
			ensure(h.count()==1);
			ensure(h.operator ->() !=0);
			ensure_equals("Constructed", exists, true);
		}
		// ptr left scope
		ensure_equals("All destructed", exists, false);
	}

	template<>
	template<>
	void object::test<2>()
	{
		{
			GC::Handle<existing> h1=new GC::Wrapper<existing>();
			GC::Handle<existing> h2=h1;
			ensure_equals("Copy ctor made ptr the same", h1.operator ->(), h2.operator ->());
			ensure_equals("Copy ctor increased count ", h1.count(), 2);
		}
		ensure_equals("All destructed", exists, false);
	}

	template<>
	template<>
	void object::test<3>()
	{
		{
			GC::Handle<existing> h1=new GC::Wrapper<existing>();
			GC::Handle<existing> h2;
			h2=h1;
			ensure_equals("The operator = made ptr the same", h1.operator ->(), h2.operator ->());
			ensure_equals("The operator = worked fine", h1.count(), 2);
		}
		ensure_equals("All destructed", exists, false);
	}

	template<>
	template<>
	void object::test<4>()
	{

		GC::Handle<existing> h;
		existing* ptr=h.operator ->();
		ensure("Default ctor made ptr 0", ptr==0);
	}

	template<>
	template<>
	void object::test<5>()
	{

		GC::Handle<existing> h=new GC::Wrapper<existing>();
		existing* ptr=h.operator ->();
		h->n=5;
		ensure_equals("Handle could access member", ptr->n,5);
	}

	template<>
	template<>
	void object::test<6>()
	{

		GC::Handle<existing> h1=new GC::Wrapper<existing>();
		GC::Handle<existing> h2=h1;
		ensure("The operator == worked", h1==h2);
	}
}
