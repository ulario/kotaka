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
#include <kotaka/assert.h>
#include <kotaka/privilege.h>

private object environment;
private object *inventory;

static void create()
{
	inventory = ({ });
}

int forbid_move(object new_env)
{
	return 0;
}

int forbid_insert(object obj)
{
	return 0;
}

int forbid_remove(object obj)
{
	return 0;
}

void move_notify(object old_env)
{
}

void insert_notify(object obj)
{
}

void remove_notify(object obj)
{
}

nomask void thing_add_inventory(object arriving)
{
	ACCESS_CHECK(THING());

	ASSERT(arriving);
	ASSERT(!sizeof(inventory & ({ arriving })));

	inventory = ({ arriving }) + (inventory - ({ nil }));
}

nomask void thing_del_inventory(object departing)
{
	ACCESS_CHECK(THING());

	ASSERT(departing);

	inventory -= ({ departing, nil });
}

nomask int is_container_of(object test)
{
	object env;
	object this;
	mapping seen;

	this = this_object();
	env = test->query_environment();

	seen = ([ ]);

	while (env) {
		if (seen[env]) {
			error("Cyclic containment detected");
		}

		if (env == this) {
			return 1;
		}

		seen[env] = 1;

		env = env->query_environment();
	}

	return 0;
}

nomask int query_depth()
{
	object obj;
	int depth;

	obj = environment;

	while (obj) {
		depth++;

		obj = obj->query_environment();
	}

	return depth;
}

nomask object query_environment()
{
	return environment;
}

nomask object *query_inventory()
{
	return (inventory -= ({ nil }))[..];
}

nomask atomic void move(object new_env)
{
	object old_env;
	object this;

	old_env = environment;

	this = this_object();

	PERMISSION_CHECK(!old_env || !old_env->forbid_remove(this));
	PERMISSION_CHECK(!forbid_move(new_env));
	PERMISSION_CHECK(!new_env || !new_env->forbid_insert(this));

	if (new_env == environment) {
		return;
	}

	if (new_env) {
		if (new_env == this) {
			error("Self containment attempted");
		}

		if (is_container_of(new_env)) {
			error("Cyclic containment attempted");
		}

		if (new_env->query_depth() > 20) {
			error("Excessive nesting attempted");
		}
	}

	if (environment) {
		environment->thing_del_inventory(this_object());
	}

	environment = new_env;

	if (environment) {
		environment->thing_add_inventory(this_object());
	}

	if (old_env) {
		old_env->remove_notify(this);
	}

	move_notify(old_env);

	if (new_env) {
		new_env->insert_notify(this);
	}
}
