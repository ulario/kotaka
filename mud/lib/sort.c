/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2022  Raymond Jennings
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
private void aswap(mixed *arr, int a, int b)
{
	mixed tmp;

	tmp = arr[a];
	arr[a] = arr[b];
	arr[b] = tmp;
}

/* strategy: */

/* pick a good pivot */
/* divide into lower and upper segments */
/* recurse into smaller segment */
/* reiterate over larger segment */

void quicksort(mixed *arr, int begin, int end, varargs string compfunc)
{
	int span;

	while ((span = end - begin) > 1) {
		int low, mid, high, sign;
		mixed pivot;

		low = begin;
		mid = random(end - begin) + begin;
		high = end - 1;

		pivot = arr[mid];
		aswap(arr, mid, high);

		while (low < high) {
			mixed le;

			le = arr[low];

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
			quicksort(arr, begin, mid, compfunc);
			begin = mid + 1;
		} else {
			quicksort(arr, mid + 1, end, compfunc);
			end = mid;
		}
	}

	if (span == 2) {
		if (arr[begin] > arr[begin + 1]) {
			aswap(arr, begin, begin + 1);
		}
	}
}
