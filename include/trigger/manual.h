#ifndef __MANUAL_H__
#define __MANUAL_H__
#include "trigger_common.h"
namespace windreamer
{
	namespace gc
	{
		namespace triggers
		{
			struct Manual
			{
                template<typename T>
				static bool test(const T&)
				{
					return false;
				};
			};

		}
	}

}
#endif//__MANUAL_H__

