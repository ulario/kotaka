/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020, 2021  Raymond Jennings
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

#include <kotaka/paths/account.h>

inherit "/lib/time";
inherit "/lib/sort";

int message(string str);

static int compare_sitebans(mixed *a, mixed *b)
{
	mixed ae, be;

	ae = a[1]["expire"];
	be = b[1]["expire"];

	if (ae) {
		if (be) {
			if (ae < be) {
				return -1;
			} else if (ae > be) {
				return 1;
			}
		} else {
			return -1;
		}
	} else if (be) {
		return 1;
	} else {
		return 0;
	}
}
