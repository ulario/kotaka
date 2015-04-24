/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <kotaka/assert.h>
#include <kotaka/paths/utility.h>
#include <status.h>

static void create()
{
}

static void test_qsort(int *arr)
{
	int i, sz;

	sz = sizeof(arr);

	SORTD->qsort(arr, 0, sz);

	for (i = 0; i < sz - 1; i++) {
		ASSERT(arr[i] <= arr[i + 1]);
	}
}

private void scramble(int *arr)
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

private void fill(int *arr)
{
	int sz;

	for (sz = sizeof(arr) - 1; sz >= 0; sz--) {
		arr[sz] = sz;
	}
}

void test()
{
	int *arr;

	arr = allocate(status(ST_ARRAYSIZE));

	fill(arr);
	scramble(arr);
	call_limited("test_qsort", arr);
}
