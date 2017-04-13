/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
