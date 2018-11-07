/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/paths/utility.h>
#include <kotaka/paths/bigstruct.h>
#include <type.h>

private void aswap(mixed arr, int a, int b)
{
	mixed tmp;

	if (typeof(arr) == T_ARRAY) {
		tmp = arr[a];
		arr[a] = arr[b];
		arr[b] = tmp;
	} else {
		tmp = arr->query_element(a);
		arr->set_element(a, arr->query_element(b));
		arr->set_element(b, tmp);
	}
}

void qsort(mixed arr, int begin, int end, varargs string compfunc)
{
	int type;

	type = typeof(arr);

	while (begin < end) {
		int low, mid, high, sign;
		mixed pivot;

		low = begin;
		mid = random(end - begin) + begin;
		high = end - 1;

		pivot = (type == T_ARRAY) ? arr[mid] : arr->query_element(mid);
		aswap(arr, mid, high);

		while (low < high) {
			mixed le;

			le = (type == T_ARRAY) ? arr[low] : arr->query_element(low);

			if (compfunc) {
				sign = call_other(this_object(), compfunc, le, pivot);
			} else if (le > pivot) {
				sign = 1;
			} else if (le < pivot) {
				sign = -1;
			} else {
				sign = 0;
			}

			if (sign > 0) {
				aswap(arr, low, --high);
			} else {
				low++;
			}
		}

		mid = low;
		aswap(arr, end - 1, mid);

		if (mid - begin < end - mid) {
			qsort(arr, begin, mid, compfunc);
			begin = mid + 1;
		} else {
			qsort(arr, mid + 1, end, compfunc);
			end = mid;
		}
	}
}

void quicksort(mixed arr, int begin, int end, varargs string compfunc)
{
	qsort(arr, begin, end, compfunc);
}

void gnomesort(mixed arr, int begin, int end, varargs string compfunc)
{
	int type;
	int cursor;

	type = typeof(arr);

	cursor = begin;

	while (cursor + 1 < end) {
		mixed le, he;
		int sign;

		if (type == T_ARRAY) {
			le = arr[cursor];
			he = arr[cursor + 1];
		} else {
			le = arr->query_element(cursor);
			he = arr->query_element(cursor + 1);
		}

		if (compfunc) {
			sign = call_other(this_object(), compfunc, le, he);
		} else if (le > he) {
			sign = 1;
		} else if (le < he) {
			sign = -1;
		} else {
			sign = 0;
		}

		if (sign == 1) {
			aswap(arr, cursor, cursor + 1);

			if (cursor > begin) {
				cursor--;
			}
		} else {
			cursor++;
		}
	}
}
