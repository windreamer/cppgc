#ifndef __COUNTING_H__
#define __COUNTING_H__
#include "trigger_common.h"
namespace windreamer
{
	namespace gc
	{
		namespace triggers
		{
			template<int N>
			struct Counting 
			{
                static bool test (const triggers::OnStartMalloc&)
                {
                    static int counter=0;
                    if(++counter==N)
                    {
                        counter=0;
                        return true;
                    }
                    return false;
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

#endif//__COUNTING_H__
