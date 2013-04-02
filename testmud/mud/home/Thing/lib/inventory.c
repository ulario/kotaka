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
#include <kotaka/checkarg.h>
#include <kotaka/paths.h>
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

/* trusted */

nomask object thing_query_environment()
{
	ACCESS_CHECK(THING());

	return environment;
}

nomask object *thing_query_inventory()
{
	ACCESS_CHECK(THING());

	inventory -= ({ nil });

	return inventory[..];
}

nomask void thing_add_inventory(object arriving)
{
	ACCESS_CHECK(THING());

	inventory = ({ arriving }) + (inventory - ({ nil }));
}

nomask void thing_del_inventory(object departing)
{
	ACCESS_CHECK(THING());

	inventory -= ({ departing });
}

nomask int thing_is_container_of(object test)
{
	object env;
	object this;
	int steps;

	ACCESS_CHECK(THING());

	this = this_object();
	env = test->query_environment();

	while (env) {
		if (env == this) {
			return 1;
		}

		steps++;

		if (steps > 100) {
			error("Nested too deeply");
		}

		env = env->query_environment();
	}

	return 0;
}

nomask int thing_query_depth()
{
	object obj;
	int depth;

	ACCESS_CHECK(THING());

	obj = environment;

	while (obj) {
		depth++;

		obj = obj->query_environment();
	}

	return depth;
}

nomask void thing_move(object new_env)
{
	ACCESS_CHECK(THING());

	if (environment) {
		environment->thing_del_inventory(this_object());
	}

	environment = new_env;

	if (environment) {
		environment->thing_add_inventory(this_object());
	}
}

/* untrusted */

object query_environment()
{
	return environment;
}

object *query_inventory()
{
	inventory -= ({ nil });

	return inventory[..];
}

int is_container_of(object test)
{
	return thing_is_container_of(test);
}

int query_depth()
{
	return thing_query_depth();
}

void move(object new_env)
{
	object old_env;
	object this;

	CHECKARG(!new_env || new_env <- "inventory", 1, "move");

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
			error("Self containment");
		}

		if (is_container_of(new_env)) {
			error("Recursive containment");
		}

		if (new_env->query_depth() >= 50) {
			error("Nested too deeply");
		}
	}

	thing_move(new_env);

	if (old_env) {
		old_env->remove_notify(this);
	}

	move_notify(old_env);

	if (new_env) {
		new_env->insert_notify(this);
	}
}
