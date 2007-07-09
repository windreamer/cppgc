#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>

using std::exception;
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::ostream_iterator;
using std::copy;
using std::string;

using tut::reporter;
using tut::groupnames;

namespace tut
{
	test_runner_singleton runner;
}

int main(int argc, const char* argv[])
{
	reporter visi;

	tut::runner.get().set_callback(&visi);

	try
	{
		if (argc == 1)
		{
			tut::runner.get().run_tests();
		}
		else if (argc == 2 && string(argv[1]) == "list")
		{
			cout << "registered test groups:" << endl;
			groupnames gl = tut::runner.get().list_groups();
			copy(gl.begin(),gl.end(),ostream_iterator<string>(cout,"\n"));
		}
		else if (argc == 2)
		{
			tut::runner.get().run_tests(argv[1]);
		}
		else
		{
			tut::runner.get().run_test(argv[1],::atoi(argv[2]));
		}
	}
	catch (const exception& ex)
	{
		cerr << "tut raised ex: " << ex.what() << endl;
		return 1;
	}

	return visi.all_ok()?0:1;
}
