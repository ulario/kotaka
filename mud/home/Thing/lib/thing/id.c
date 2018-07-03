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
#include <kotaka/paths/string.h>

object query_environment();
object *query_inventory();

#define ID(x, y) (((y) > 1) ? ((x) + "#" + (y)) : (x))

private string id_base;
private int id_number;

static void create()
{
}

static void validate_base_id(string new_base)
{
	if (!STRINGD->is_valid_base_id(new_base)) {
		error("Bad base ID");
	}

	switch(new_base) {
	case "ENV":
	case "THIS":
		error("Reserved base ID");
	default:
		break;
	}
}

int query_lowest_free(string trial, object exclude)
{
	mapping taken;
	object *inv;
	int i;
	int sz;

	taken = ([ ]);

	inv = query_inventory() - ({ exclude });

	for (i = 0; i < sizeof(inv); i++) {
		object check;

		check = inv[i];

		if (check->query_id_base() != trial) {
			continue;
		}

		taken[check->query_id_number()] = 1;
	}

	i = 1;

	while(taken[i]) {
		i++;
	}

	return i;
}


string query_id()
{
	if (!id_base) {
		return nil;
	}

	return ID(id_base, id_number);
}

string query_id_base()
{
	return id_base;
}

int query_id_number()
{
	return id_number;
}

void set_id(string new_id)
{
	object env;
	string new_base;
	int new_number;
	string old_id;

	if (!new_id) {
		id_base = nil;
		id_number = 0;
		return;
	}

	if (!sscanf(new_id, "%s#%d", new_base, new_number)) {
		new_base = new_id;
		new_number = 1;
	}

	validate_base_id(new_base);

	old_id = ID(id_base, id_number);
	new_id = ID(new_base, new_number);

	if (old_id == new_id) {
		return;
	}

	env = query_environment();

	if (env) {
		object test;

		test = env->find_by_id(new_id);

		if (test && test != this_object()) {
			error("Duplicate ID");
		}
	}

	id_base = new_base;
	id_number = new_number;
}

void set_id_base(string new_base)
{
	object env;
	int new_number;
	string old_id;
	string new_id;

	if (!new_base) {
		id_base = nil;
		id_number = 0;
		return;
	}

	validate_base_id(new_base);

	env = query_environment();

	if (env) {
		new_number = env->query_lowest_free(
			new_base, this_object()
		);
	} else {
		new_number = 1;
	}

	id_base = new_base;
	id_number = new_number;
}

void set_id_number(int new_number)
{
	object env;
	object other;
	string old_id;

	if (!id_base) {
		error("Invalid ID (number requires base)");
	}

	if (new_number <= 0) {
		error("Invalid ID (number must be positive)");
	}

	env = query_environment();

	if (new_number == id_number) {
		return;
	}

	if (env) {
		other = env->find_by_id(ID(id_base, new_number));

		if (other && other != this_object()) {
			error("Duplicate ID");
		}
	}

	old_id = ID(id_base, id_number);
	id_number = new_number;
}

static void reset_id_number()
{
	object new_env;
	int new_number;

	if (!id_base) {
		return;
	}

	new_env = query_environment();

	if (new_env) {
		new_number = new_env->query_lowest_free(id_base, this_object());
	} else {
		new_number = 1;
	}

	set_id_number(new_number);
}

object find_by_id(string id)
{
	string base;
	object *inv;
	int number;
	int index;
	int sz;

	if (!sscanf(id, "%s#%d", base, number)) {
		base = id;
		number = 1;
	}

	if (number <= 0) {
		error("Invalid ID (bad number)");
	}

	if (base == "ENV") {
		object env;

		env = this_object();

		for (index = 0; index < number; index++) {
			if (!env) {
				return nil;
			}

			env = env->query_environment();
		}

		return env;
	}

	if (base == "THIS") {
		if (number != 1) {
			error("Bad ID");
		}

		return this_object();
	}

	inv = query_inventory();
	sz = sizeof(inv);

	for (index = 0; index < sz; index++) {
		if (inv[index]->query_id() == id) {
			return inv[index];
		}
	}
}
