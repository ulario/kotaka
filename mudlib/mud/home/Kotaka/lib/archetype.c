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
#include <kotaka/checkarg.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

private object *archetypes;

nomask static void initialize_archetype()
{
	ACCESS_CHECK(previous_program() == LIB_OBJECT);

	if (!archetypes) {
		archetypes = ({ });
	}
}

nomask int is_archetype_of(object test)
{
	int index;
	int sz;
	object *arch;

	arch = test->query_archetypes();
	sz = sizeof(arch);

	if (sizeof(arch & ({ this_object() }))) {
		return 1;
	}

	for(index = 0; index < sz; index++) {
		if (is_archetype_of(arch[index])) {
			return 1;
		}
	}

	return 0;
}

nomask object *query_archetypes()
{
	return archetypes -= ({ nil });
}

nomask void set_archetypes(object *new_archs)
{
	int i;
	int sz;

	object *old_archs;
	object *check;

	archetypes -= ({ nil });
	new_archs = new_archs -= ({ nil });
	check = new_archs - ({ archetypes });

	sz = sizeof(check);

	for (i = 0; i < sz; i++) {
		if (!check[i] <- LIB_OBJECT) {
			error("Bad argument 1 for function set_archetypes (found non LIB_OBJECT)");
		}
		if (is_archetype_of(check[i])) {
			error("Circular reference");
		}
	}

	archetypes = new_archs;
}

nomask void clear_archetypes()
{
	archetypes = ({ });
}

nomask void add_archetype(object new_arch)
{
	CHECKARG(new_arch, 1, "add_archetype");
	CHECKARG(new_arch <- LIB_OBJECT, 1, "add_archetype");

	if (is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	archetypes -= ({ nil });
	archetypes += ({ new_arch });
}

nomask void add_archetype_at(object new_arch, int position)
{
	CHECKARG(new_arch, 1, "add_archetype_at");
	CHECKARG(new_arch <- LIB_OBJECT, 1, "add_archetype_at");

	CHECKARG(position >= -1, 2, "add_archetype_at");
	CHECKARG(position <= sizeof(archetypes), 2, "add_archetype_at");

	archetypes -= ({ nil });

	if (position == -1) {
		archetypes += ({ new_arch });
	} else {
		archetypes = archetypes[0 .. position - 1]
			+ ({ new_arch }) + archetypes[position ..];
	}
}

nomask void del_archetype(object old_arch)
{
	archetypes -= ({ nil, old_arch });
}
