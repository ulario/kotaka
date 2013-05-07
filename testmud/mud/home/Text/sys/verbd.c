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
#include <kotaka/privilege.h>

string *gather_dirs(string dir)
{
	string *buf;

	mixed **dirlist;
	string *names;
	int *sizes;

	int i, sz;

	dirlist = get_dir(dir + "/*");

	names = dirlist[0];
	sizes = dirlist[1];

	buf = ({ dir });
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		if (sizes[i] == -2) {
			buf += gather_dirs(dir + "/" + names[i]);
		}
	}

	return buf;
}

object find_verb(string command)
{
	object verb;
	string *dirs;
	int i, sz;

	dirs = gather_dirs("~/sys/verb");
	sz = sizeof(dirs);

	for (i = 0; i < sz; i++) {
		if (verb = find_object(dirs[i] + "/" + command)) {
			return verb;
		}
	}
}

int do_action(object actor, string command, string args)
{
	object ustate;
	object verb;

	ACCESS_CHECK((ustate = previous_object())<-TEXT_LIB_USTATE);

	verb = find_verb(command);

	if (!verb) {
		return FALSE;
	}

	TLSD->set_tls_value("Text", "ustate", ustate);

	verb->main(actor, args);

	if (this_object()) {
		TLSD->set_tls_value("Text", "ustate", nil);
	}

	return TRUE;
}
