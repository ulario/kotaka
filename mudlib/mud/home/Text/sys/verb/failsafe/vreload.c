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
#include <kotaka/paths.h>
#include <text/paths.h>

inherit LIB_RAWVERB;
inherit UTILITY_COMPILE;

private void destruct_verbs(object proxy)
{
	object proglist;
	int sz;
	int i;

	proglist = PROGRAMD->query_program_indices();

	sz = proglist->get_size();

	for (i = 0; i < sz; i++) {
		object pinfo;
		string path;

		pinfo = PROGRAMD->query_program_info(proglist->get_element(i));
		path = pinfo->query_path();

		if (path == object_name(this_object())) {
			continue;
		}

		if (sscanf(path, USR_DIR + "/Game/sys/verb/%*s")) {
			proxy->destruct_object(path);
			continue;
		}

		if (sscanf(path, USR_DIR + "/Text/sys/verb/%*s")) {
			proxy->destruct_object(path);
			continue;
		}
	}
}

private void load_verbs(string directory, object proxy)
{
	mixed **files;
	string *names;
	mixed *objs;
	int *sizes;
	int index;

	names = proxy->get_dir(directory + "/*")[0];

	for (index = 0; index < sizeof(names); index++) {
		mixed *info;
		string name;

		name = names[index];

		info = proxy->file_info(directory + "/" + name);

		if (info[0] == -2) {
			load_verbs(directory + "/" + name, proxy);
			continue;
		}

		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			continue;
		}

		name = name[ .. strlen(name) - 3];

		proxy->compile_object(directory + "/" + name);
	}
}

atomic void main(object actor, string args)
{
	object user;
	object *users;
	object proxy;
	int i, sz;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to reload the verb collection.\n");
		return;
	}

	proxy = PROXYD->get_proxy(user->query_name());

	destruct_verbs(proxy);
	load_verbs(USR_DIR + "/Game/sys/verb", proxy);
	load_verbs(USR_DIR + "/Text/sys/verb", proxy);
	compile_object(object_name(this_object()));
}
