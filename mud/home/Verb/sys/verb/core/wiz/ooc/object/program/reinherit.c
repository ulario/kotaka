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
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>
#include <kotaka/paths/utility.h>
#include <type.h>
#include <status.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	mixed *status;
	string path;

	object hits;
	object list;

	object libs;
	object objs;
	object proxy;

	int sz;
	mixed oindex;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to reinherit.\n");
		return;
	}

	path = roles["raw"];

	path = DRIVER->normalize_path(path, "/");

	oindex = status(path, O_INDEX);

	if (oindex == nil) {
		send_out("No such program.\n");
		return;
	}

	hits = new_object(BIGSTRUCT_MAP_LWO);
	hits->claim();
	hits->set_type(T_INT);
	hits->grant_access(find_object(SUBD), WRITE_ACCESS);

	SUBD->gather_inheriters(oindex, hits);

	list = hits->query_indices();

	libs = new_object(BIGSTRUCT_ARRAY_LWO);
	libs->claim();
	objs = new_object(BIGSTRUCT_ARRAY_LWO);
	objs->claim();

	for (sz = list->query_size(); --sz >= 0; ) {
		int oindex;
		object pinfo;
		string path;

		oindex = list->query_element(sz);

		pinfo = PROGRAMD->query_program_info(oindex);

		if (pinfo->query_destructed()) {
			continue;
		}

		path = pinfo->query_path();

		if (sscanf(path, "%*s" + INHERITABLE_SUBDIR + "%*s")) {
			libs->push_back(path);
		} else {
			objs->push_back(path);
		}
	}

	proxy = PROXYD->get_proxy(query_user()->query_name());

	for (sz = libs->query_size(); --sz >= 0; ) {
		proxy->destruct_object(libs->query_element(sz));
	}

	proxy->destruct_object(path);

	for (sz = objs->query_size(); --sz >= 0; ) {
		proxy->compile_object(objs->query_element(sz));
	}
}
