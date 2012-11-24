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
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

private void discover_clones();

static void create()
{
	discover_clones();
}

void add_clone(object obj)
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

void remove_clone(object obj)
{
	ACCESS_CHECK(previous_program() == OBJECTD);
}

private void discover_clones()
{
	string *owners;
	int i, sz;
	int count;

	owners = KERNELD->query_owners();
	sz = sizeof(owners);

	for (i = 0; i < sz; i++) {
		object first;

		first = KERNELD->first_link(owners[i]);

		if (first) {
			object obj;
			string name;

			obj = first;

			do {
				name = object_name(obj);

				if (sscanf(name, "%*s#%*d")) {
					LOGD->post_message("clone", LOG_DEBUG, "Found clone " + name);
					// deal with clone here
					count++;
				}
				obj = KERNELD->next_link(obj);
			} while (obj != first);
		}
	}
}
