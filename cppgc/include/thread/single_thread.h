#ifndef __SINGLE_THREAD_H__
#define __SINGLE_THREAD_H__
namespace windreamer
{
    namespace thread
    {
        namespace policy
        {
            struct SingleThread
            {
                struct Lock
                {
					~Lock(){}
                };

            };
        }
    }
}
#endif//__SINGLE_THREAD_H__

