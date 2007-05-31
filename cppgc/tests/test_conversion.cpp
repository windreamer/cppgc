#include <tut.h>
#include "algorithm/reference_count.h"
#include "traits/conversion.h"
#include "gc.h"

using namespace windreamer::gc;
using namespace windreamer::traits;

namespace tut
{
	struct ConversionTestData
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

		struct Base{int n;};
		struct Derived:Base{};

	};
	bool ConversionTestData::exists=false;
	typedef test_group<ConversionTestData> testGroup;
	typedef testGroup::object object;
	testGroup conversionGroup("test_conversion");
	typedef GarbageCollector<algorithm::ReferenceCount<> > GC;



	template<>
	template<>
	void object::test<1>()
	{
		GC::Handle<existing> h=new GC::Wrapper<existing>();
		h->n=1;
		GC::Handle<const existing> h1=h;
		ensure_equals(h1->n,1);
		ensure(Conversion<int*,const int*>::exists);
	}

	template<>
	template<>
	void object::test<2>()
	{
		const GC::Handle<existing> h=new GC::Wrapper<existing>();
		const GC::Handle<const existing> h1=h;
		ensure(Conversion<int*const ,const int*const>::exists);
	}

	template<>
	template<>
	void object::test<3>()
	{
		GC::Handle<existing> h=new GC::Wrapper<existing>();
		const GC::Handle<const existing> h1=h;
		ensure(Conversion<int*,const int*const>::exists);
	}

	template<>
	template<>
	void object::test<4>()
	{
		GC::Handle<Derived> h=new GC::Wrapper<Derived>();
		h->n=1;
		GC::Handle<Base> h1=h;
		ensure_equals(h1->n,1);
		ensure(Conversion<Derived*,Base*>::exists);
	}

}
