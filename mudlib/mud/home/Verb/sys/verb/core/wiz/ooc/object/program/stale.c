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
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	/* Report all objects inheriting a destructed inheritable */
	object indices;
	int i, sz;

	if (query_user()->query_class() < 2) {
		send_out("Only a wizard can do that.\n");
		return;
	}

	indices = PROGRAMD->query_program_indices();
	sz = indices->get_size();

	for (i = 0; i < sz; i++) {
		int pindex;
		object pinfo;
		int *libs;
		int ssz, j;

		pindex = indices->query_element(i);
		pinfo = PROGRAMD->query_program_info(pindex);

		if (pinfo->query_destructed()) {
			continue;
		}

		libs = pinfo->query_inherits();
		ssz = sizeof(libs);

		for (j = 0; j < ssz; j++) {
			object pinfo2;

			pinfo2 = PROGRAMD->query_program_info(libs[j]);

			if (pinfo2->query_destructed()) {
				send_out(pinfo->query_path() + " inherits " + pinfo2->query_path() + "\n");
			}
		}
	}
}
