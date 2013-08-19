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
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>
#include <type.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	mixed *status;
	object hits;
	object list;

	object libs;
	object objs;
	object proxy;

	int i, sz;
	int oindex;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to reinherit.\n");
		return;
	}

	args = DRIVER->normalize_path(args, "/");

	list = PROGRAMD->query_includers(args);

	if (!list) {
		send_out("No programs include that file.\n");
		return;
	}

	hits = new_object(BIGSTRUCT_MAP_LWO);
	hits->set_type(T_INT);
	hits->grant_access(find_object(SUBD), WRITE_ACCESS);

	sz = list->get_size();

	for (i = 0; i < sz; i++) {
		SUBD->gather_inheriters(list->get_element(i), hits);
	}

	list = hits->get_indices();
	sz = list->get_size();

	libs = new_object(BIGSTRUCT_ARRAY_LWO);
	objs = new_object(BIGSTRUCT_ARRAY_LWO);

	for (i = 0; i < sz; i++) {
		int oindex;
		object pinfo;
		string path;

		oindex = list->get_element(i);

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

	sz = libs->get_size();
	proxy = PROXYD->get_proxy(query_user()->query_name());

	for (i = 0; i < sz; i++) {
		proxy->destruct_object(libs->get_element(i));
	}

	sz = objs->get_size();

	for (i = 0; i < sz; i++) {
		proxy->compile_object(objs->get_element(i));
	}
}
