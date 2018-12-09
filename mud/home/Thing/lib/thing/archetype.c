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
#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/log.h>
#include <kotaka/paths/utility.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit "/lib/tier";

/* old */
private object *archetypes;

/* new */
private object archetype;

static void create()
{
}

nomask void clean_archetypes()
{
	archetypes -= ({ nil });
}

nomask void patch_archetype()
{
	clean_archetypes();

	if (sizeof(archetypes)) {
		archetype = archetypes[0];
	} else {
		archetype = nil;
	}
}

nomask int is_archetype_of(object "archetype" instance)
{
	object iarch;
	int steps;

	for (steps = 64; --steps >= 0;) {
		iarch = instance->query_archetype();

		if (!iarch) {
			return 0;
		}

		if (iarch == this_object()) {
			return 1;
		}

		instance = iarch;
	}

	error("Archetype level too deep");
}

nomask void set_archetype(object "archetype" new_archetype)
{
	patch_archetype();

	if (new_archetype) {
		if (new_archetype->is_archetype_of(this_object())) {
			error("Cyclic archetype");
		}
	}

	archetype = new_archetype;
}

nomask object query_archetype()
{
	patch_archetype();

	return archetype;
}
