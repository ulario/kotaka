#include <kotaka/privilege.h>

static void test()
{
	"struct"->schedule_test();
	"sysstruct"->schedule_test();
}

void schedule_test()
{
	ACCESS_CHECK(TEST());

	call_out("test", 0);
}
