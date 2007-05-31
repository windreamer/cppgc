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
				static bool test (triggers::TRIGGER_POINT t)
				{
					static int counter=0;
					if (t==triggers::EVERY_NEW_START)
					{
						if(++counter==N)
						{
							counter=0;
							return true;
						}
					}
					return false;
				}
			};
		}
	}
}

#endif//__COUNTING_H__
