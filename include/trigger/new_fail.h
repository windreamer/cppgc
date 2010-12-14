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
				static bool test (const triggers::OnOutOfMemory&)
				{
                    return true;
				}

                template<typename T>
                static bool test (const T&)
                {
                    return false;
                }
			};		
		}
	}
}
#endif//__NEW_FAIL_H__
