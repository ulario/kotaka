#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>

#include <status.h>

int *arr;

static void create()
{
	int i, sz;

	arr = allocate(sz = status(ST_ARRAYSIZE));

	for (i = 0; i < sz; i++) {
		arr[i] = random(1000000000);
	}
}

private void test_qsort()
{
	int i, sz;

	sz = sizeof(arr);

	SUBD->qsort(arr, 0, sz);

	for (i = 0; i < sz - 1; i++) {
		ASSERT(arr[i] <= arr[i + 1]);
	}
}

private void scramble()
{
	int i, sz;

	sz = sizeof(arr);

	for (i = 0; i < sz - 1; i++) {
		int j;
		int tmp;

		j = random(sz);

		tmp = arr[i];
		arr[i] = arr[j];
		arr[j] = tmp;
	}
}

void test()
{
	LOGD->post_message("test", LOG_DEBUG, "Testing qsort...");

	scramble();
	test_qsort();
}
