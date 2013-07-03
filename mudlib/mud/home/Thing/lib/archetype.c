/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

private object *archetypes;

static void create()
{
}

nomask void clean_archetypes()
{
	if (!archetypes) {
		return;
	}

	archetypes -= ({ nil });

	if (!sizeof(archetypes)) {
		archetypes = nil;
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
	clean_archetypes();

	if (archetypes) {
		return archetypes[..];
	} else {
		return ({ });
	}
}

nomask void set_archetypes(object *new_archs)
{
	int i;
	int sz;

	object *new, *old, *arch;
	object *check;

	arch = query_archetypes();

	old = arch - new_archs;
	new = new_archs - arch;

	old -= ({ nil });
	new -= ({ nil });

	sz = sizeof(new);

	for (i = 0; i < sz; i++) {
		object arch;

		arch = new[i];

		if (!(arch <- "thing")) {
			error("Invalid archetype");
		}

		if (is_archetype_of(arch)) {
			error("Circular reference");
		}
	}

	archetypes = new_archs;
}

nomask void clear_archetypes()
{
	archetypes = nil;
}

nomask void add_archetype(object new_arch)
{
	if (!new_arch || !(new_arch <- "thing")) {
		error("Invalid archetype");
	}

	if (is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	clean_archetypes();

	if (!archetypes) {
		archetypes = ({ new_arch });
	} else {
		archetypes += ({ new_arch });
	}
}

nomask void add_archetype_at(object new_arch, int position)
{
	if (!new_arch || !(new_arch <- "thing")) {
		error("Invalid archetype");
	}

	if (position < 0) {
		error("Invalid position");
	}

	if (archetypes) {
		if (position > sizeof(archetypes)) {
			error("Invalid position");
		}
	} else {
		if (position > 0) {
			error("Invalid position");
		}
	}

	if (is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	clean_archetypes();

	if (position == -1) {
		archetypes += ({ new_arch });
	} else {
		archetypes = archetypes[0 .. position - 1]
			+ ({ new_arch }) + archetypes[position ..];
	}
}

void del_archetype(object old_arch)
{
	archetypes -= ({ old_arch });

	clean_archetypes();
}
