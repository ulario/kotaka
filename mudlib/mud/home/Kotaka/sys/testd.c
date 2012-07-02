#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>

static void create()
{
}

private void test_qsort()
{
	int *arr, i;

	arr = allocate(1000);

	for (i = 0; i < 1000; i++) {
		arr[i] = random(1000000);
	}

	SUBD->qsort(arr, 0, 1000);

	for (i = 0; i < 999; i++) {
		ASSERT(arr[i] <= arr[i + 1]);
	}
}

void test()
{
	LOGD->post_message("test", LOG_DEBUG, "Testing qsort...");
	test_qsort();
}
