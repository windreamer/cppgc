#ifndef __NEW_FAIL_H__
#define __NEW_FAIL_H__
#include "trigger_common.h"
namespace windreamer
{
	namespace gc
	{
		namespace triggers
		{
			struct NewFail 
			{
				static bool test (triggers::TRIGGER_POINT t)
				{
					if (t==triggers::OUT_OF_MEMORY)
					{
							return true;
					}
					return false;
				}
			};		
		}
	}
}
#endif//__NEW_FAIL_H__
