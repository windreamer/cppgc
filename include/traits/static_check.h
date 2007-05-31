#ifndef __STATIC_CHECK__
#define __STATIC_CHECK__
namespace windreamer
{
	namespace traits
	{
		template <bool> 
		struct CompileTimeChecker;
		template <> 
		struct CompileTimeChecker<true>{};
	}
}
#endif//__STATIC_CHECK__
