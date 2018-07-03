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

object query_common_container(object a, object b)
{
	mapping la, ra;
	object le, re;

	le = a;
	re = b;

	la = ([ ]);
	ra = ([ ]);

	while (le && re) {
		la[le] = 1;
		ra[re] = 1;

		if (la[re]) {
			return re;
		}

		if (ra[le]) {
			return le;
		}

		le = le->query_environment();
		re = re->query_environment();
	}

	while (le) {
		if (ra[le]) {
			return le;
		}

		le = le->query_environment();
	}

	while (re) {
		if (la[re]) {
			return re;
		}

		re = re->query_environment();
	}
}
