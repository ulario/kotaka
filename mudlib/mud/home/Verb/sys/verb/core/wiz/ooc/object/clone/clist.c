/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/algorithm.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>
#include <kernel/access.h>
#include <status.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

int compare(string a, string b)
{
	int na, nb;

	sscanf(a, "%s#%d", a, na);
	sscanf(b, "%s#%d", b, nb);

	if (na > nb) {
		return 1;
	}
	if (na < nb) {
		return -1;
	}
	return 0;
}

void main(object actor, mapping roles)
{
	string path;
	mixed *st;
	int i, sz;
	object cinfo;
	object obj;
	object arr;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to do a clone check.\n");
		return;
	}

	path = roles["raw"];

	st = status(path);

	if (!st) {
		send_out("No such object.\n");
		return;
	}

	cinfo = CLONED->query_clone_info(st[O_INDEX]);

	if (!cinfo) {
		send_out("No clone info for object.\n");
		return;
	}

	obj = cinfo->query_first_clone();
	arr = new_object(BIGSTRUCT_ARRAY_LWO);

	do {
		arr->push_back(object_name(obj));
		obj = obj->query_next_clone();
	} while (obj != cinfo->query_first_clone());

	arr->grant_access(find_object(SORTD), WRITE_ACCESS);
	SORTD->qsort(arr, 0, arr->query_size(), "compare");

	for (sz = arr->query_size(), i = 0; i < sz; i++) {
		send_out(arr->query_element(i) + "\n");
	}
}
