#ifndef __CONVERSION_H__
#define __CONVERSION_H__
namespace windreamer
{
	namespace traits
	{
		template <typename T,typename U>
		class Conversion
		{
			typedef char Small;
			class Big{char dummy[2];};
			static Small Test(U);
			static Big Test(...);
			static T MakeT();
		public:
			enum {exists=sizeof(Test(MakeT()))==sizeof(Small)};
		};
	}
}
#endif//__CONVERSION_H__
