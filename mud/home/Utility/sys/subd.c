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
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <type.h>

inherit LIB_SYSTEM;
inherit "/lib/time";
inherit "/lib/tier";
inherit "/lib/copy";

void gather_inheriters(int oindex, object hits)
{
	object pinfo;
	object inh;

	int i, sz;

	inh = PROGRAMD->query_inheriters(oindex);

	if (!inh) {
		return;
	}

	sz = inh->query_size();

	for (i = 0; i < sz; i++) {
		int lib;

		lib = inh->query_element(i);

		if (!hits->query_element(lib)) {
			hits->set_element(lib, 1);

			gather_inheriters(lib, hits);
		}
	}
}
