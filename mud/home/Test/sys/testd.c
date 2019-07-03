#include <kotaka/privilege.h>

private void run_tests(string dirname)
{
	string *names;
	int *sizes;
	int *times;
	object *objs;
	int sz;

	({ names, sizes, times, objs }) = get_dir("test" + dirname + "/*");

	for (sz = sizeof(names); --sz >= 0; ) {
		if (sizes[sz] == -2) {
			run_tests(dirname + "/" + names[sz]);
		} else if (sscanf(names[sz], "%*s.c")) {
			objs[sz]->schedule_test();
		}
	}
}

void schedule_tests()
{
	ACCESS_CHECK(TEST());

	run_tests("");
}
