#ifndef __TRIGGER_AND_H__
#define __TRIGGER_AND_H__
#include "trigger_common.h"
namespace windreamer
{
	namespace gc
	{
		namespace triggers
		{
			template<typename T, typename U>
			struct Or
			{
                template<typename Tag>
				static bool test(const Tag& t)
				{
					return T::test(t)||U::test(t);
				};
			};

		}
	}

}
#endif//__TRIGGER_AND_H__
