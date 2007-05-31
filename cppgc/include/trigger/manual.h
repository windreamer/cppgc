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
				static bool test(TRIGGER_POINT t)
				{
					return false;
				};
			};

		}
	}

}
#endif//__MANUAL_H__

