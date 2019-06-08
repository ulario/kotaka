#include <kotaka/privilege.h>

static void test()
{
	"System/main"->schedule_test();
}

void schedule_test()
{
	ACCESS_CHECK(TEST());

	call_out("test", 0);
}
