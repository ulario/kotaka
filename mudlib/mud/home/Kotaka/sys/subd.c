/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/property.h>
#include <kotaka/log.h>
#include <kotaka/bigstruct.h>

#include <type.h>

int binary_search_floor(mixed *arr, mixed value)
{
	int begin;
	int end;

	begin = 0;
	end = sizeof(arr);

	if (begin == end) {
		/* zero size array */
		return -1;
	}

	if (value < arr[begin]) {
		/* completely out of range */
		return -1;
	}

	if (value >= arr[end - 1]) {
		/* it floats to the top floor */
		return end - 1;
	}

	while (end - begin > 1) {
		int midpoint;
		mixed probe;

		midpoint = (begin + end) >> 1;

		probe = arr[midpoint];

		if (value < probe) {
			/* too low */
			end = midpoint;
		} else {
			begin = midpoint;
		}
	}

	return begin;
}

/* find the smallest value greater than or equal to the target */
int binary_search_ceiling(mixed *arr, mixed value)
{
	int begin;
	int end;

	begin = 0;
	end = sizeof(arr);

	if (value <= arr[begin]) {
		/* floats to the first floor */
		return 0;
	}

	if (value > arr[end - 1]) {
		/* goes into orbit */
		return end;
	}

	while (end - begin > 1) {
		int midpoint;
		mixed probe;

		midpoint = (begin + end) >> 1;

		probe = arr[midpoint];

		if (value <= probe) {
			end = midpoint;
		} else {
			/* too high */
			begin = midpoint;
		}
	}

	return begin;
}

mixed deep_copy(mixed in, varargs mapping dupes)
{
	switch (typeof(in)) {
	case T_NIL:
		return nil;
	case T_INT:
	case T_FLOAT:
	case T_STRING:
		return in;
	default:
		if (typeof(in) == T_OBJECT
			&& sscanf(object_name(in), "%*s#-1") == 0) {
			return in;
		}
	}

	if (!dupes) {
		dupes = ([ ]);
	}

	if (!dupes[in]) {
		int index;
		mixed *ind;
		mixed *val;

		switch (typeof(in)) {
		case T_OBJECT:
			{
				mixed data;
				object new;

				new = new_object(in);
				dupes[in] = new;
				new->finish_deep_copy(dupes);
			}
			break;

		case T_ARRAY:
			dupes[in] = allocate(sizeof(in));

			for(index = 0; index < sizeof(in); index++) {
				dupes[in][index] =
					deep_copy(in[index], dupes);
			}

			break;

		case T_MAPPING:
			dupes[in] = ([ ]);

			ind = map_indices(in);
			val = map_values(in);

			for (index = 0; index < sizeof(ind); index++) {
				ind[index] = deep_copy(ind[index], dupes);
				val[index] = deep_copy(val[index], dupes);

				dupes[in][ind[index]] = val[index];
			}
		}
	}

	return dupes[in];
}

string ptime(int time)
{
	string c;
	string hms;
	string w;
	string m;
	string d;
	string y;

	string dt;

	c = ctime(time);

	hms = c[11 .. 18];

	w = c[0 .. 2];
	m = c[4 .. 6];
	d = c[8 .. 9];
	y = c[20 .. 23];

	if (d[0] == ' ')
		d[0] = '0';

	dt = m + " " + d + ", " + y;
	return dt + " " + hms;
}

string pmtime(mixed *mtime)
{
	int msec;
	string ms;

	msec = (int) (mtime[1] * 1000.0);
	ms = "000" + (string) msec;
	ms = ms[strlen(ms) - 3..];
	return ptime(mtime[0]) + "." + ms;
}

/* random number generation */

float rnd()
{
	/* use 30 bits of randomness */
	return ldexp((float)random(1 << 30), -30);
}

float pi()
{
	return atan(1.0) * 4.0;
}

private void aswap(mixed *arr, int a, int b)
{
	mixed tmp;

	tmp = arr[a];
	arr[a] = arr[b];
	arr[b] = tmp;
}

void qsort(mixed *arr, int begin, int end, varargs string compfunc)
{
	int sign;

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

	tmp = arr->get_element(a);
	arr->set_element(a, arr->get_element(b));
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

		pivot = arr->get_element(mid);
		baswap(arr, mid, high);

		while (low < high) {
			if (compfunc) {
				sign = call_other(previous_object(), compfunc, arr->get_element(low), pivot);
			} else if (arr->get_element(low) > pivot) {
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
