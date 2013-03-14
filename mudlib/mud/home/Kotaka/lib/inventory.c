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
#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

private object environment;
private object *inventory;

nomask static void initialize_inventory()
{
	ACCESS_CHECK(previous_program() == LIB_OBJECT);

	if (!inventory) {
		inventory = ({ });
	}
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

nomask object kotaka_query_environment()
{
	return environment;
}

nomask object *kotaka_query_inventory()
{
	inventory -= ({ nil });

	return inventory[..];
}

nomask void kotaka_add_inventory(object arriving)
{
	ASSERT(arriving);
	ASSERT(!sizeof( ({ arriving }) & inventory ));

	ACCESS_CHECK(KOTAKA());

	inventory = ({ arriving }) + (inventory - ({ nil }));
}

nomask void kotaka_del_inventory(object departing)
{
	ASSERT(departing);

	ACCESS_CHECK(KOTAKA());

	inventory -= ({ departing });
}

nomask int kotaka_is_container_of(object test)
{
	object env;
	object this;
	int steps;

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

nomask int kotaka_query_depth()
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

nomask void kotaka_move(object new_env)
{
	object old_env;
	object this;

	CHECKARG(!new_env || new_env <- LIB_OBJECT, 1, "move");

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

	if (environment) {
		environment->kotaka_del_inventory(this_object());
	}

	environment = new_env;

	if (environment) {
		environment->kotaka_add_inventory(this_object());
	}

	if (old_env) {
		old_env->remove_notify(this);
	}

	move_notify(old_env);

	if (new_env) {
		new_env->insert_notify(this);
	}
}

/* untrusted */

nomask object query_environment()
{
	return environment;
}

nomask object *query_inventory()
{
	inventory -= ({ nil });

	return inventory[..];
}

nomask void kotaka_add_inventory(object arriving)
{
	ASSERT(arriving);
	ASSERT(!sizeof( ({ arriving }) & inventory ));

	ACCESS_CHECK(KOTAKA());

	inventory = ({ arriving }) + (inventory - ({ nil }));
}

nomask void kotaka_del_inventory(object departing)
{
	ASSERT(departing);

	ACCESS_CHECK(KOTAKA());

	inventory -= ({ departing });
}

nomask int is_container_of(object test)
{
	object env;
	object this;
	int steps;

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

void move(object new_env)
{
	object old_env;
	object this;

	CHECKARG(!new_env || new_env <- LIB_OBJECT, 1, "move");

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

	if (environment) {
		environment->kotaka_del_inventory(this_object());
	}

	environment = new_env;

	if (environment) {
		environment->kotaka_add_inventory(this_object());
	}

	if (old_env) {
		old_env->remove_notify(this);
	}

	move_notify(old_env);

	if (new_env) {
		new_env->insert_notify(this);
	}
}
