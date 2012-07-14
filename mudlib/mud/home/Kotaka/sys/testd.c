#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>

#include <status.h>

int *arr;

static void create()
{
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

private void fill()
{
	int sz;

	for (sz = sizeof(arr) - 1; sz >= 0; sz--) {
		arr[sz] = sz;
	}
}

void test()
{
	int l, sz;

	arr = allocate(status(ST_ARRAYSIZE));

	fill();
	scramble();
	test_qsort();
}
