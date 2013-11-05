/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/paths/bigstruct.h>

private void aswap(mixed *arr, int a, int b)
{
	mixed tmp;

	tmp = arr[a];
	arr[a] = arr[b];
	arr[b] = tmp;
}

void qsort(mixed *arr, int begin, int end, varargs string compfunc)
{
	while (begin < end) {
		int low, mid, high, sign;
		mixed pivot;

		low = begin;
		mid = (begin + end) / 2;
		high = end - 1;

		pivot = arr[mid];
		aswap(arr, mid, high);

		while (low < high) {
			if (compfunc) {
				sign = call_other(previous_object(), compfunc, arr[low], pivot);
			} else if (arr[low] > pivot) {
				sign = 1;
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

private void baswap(object arr, int a, int b)
{
	mixed tmp;

	tmp = arr->query_element(a);
	arr->set_element(a, arr->query_element(b));
	arr->set_element(b, tmp);
}

void bqsort(object LIB_BIGSTRUCT_ARRAY_ROOT arr, int begin, int end, varargs string compfunc)
{
	int sign;

	while (begin < end) {
		int low, mid, high;
		mixed pivot;

		low = begin;
		mid = (begin + end) / 2;
		high = end - 1;

		pivot = arr->query_element(mid);
		baswap(arr, mid, high);

		while (low < high) {
			if (compfunc) {
				sign = call_other(previous_object(), compfunc, arr->query_element(low), pivot);
			} else if (arr->query_element(low) > pivot) {
				sign = 1;
			} else {
				sign = 0;
			}
			if (sign > 0) {
				baswap(arr, low, --high);
			} else {
				low++;
			}
		}

		mid = low;
		baswap(arr, end - 1, mid);

		if (mid - begin < end - mid) {
			bqsort(arr, begin, mid, compfunc);
			begin = mid + 1;
		} else {
			bqsort(arr, mid + 1, end, compfunc);
			end = mid;
		}
	}
}
