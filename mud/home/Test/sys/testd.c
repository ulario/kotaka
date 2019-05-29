#include <kotaka/privilege.h>

static void test()
{
	"tests/main"->schedule_test();
}

void schedule_tests()
{
	call_out("test", 0);
}
