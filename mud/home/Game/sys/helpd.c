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
#include <kotaka/paths/help.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/privilege.h>

inherit "~System/lib/struct/list";

void load_help();

static void create()
{
	load_help();
}

static void load_helpfile(string dir, string entry)
{
	dir = (dir ? dir + "/" : "");

	HELPD->add_topic(dir + entry,
		read_file("~/data/help/" + dir + entry + ".hlp")
	);
}

static void load_helpdir(string dir, mixed **clist, mixed **tlist)
{
	mixed **dirlist;
	string *names;
	int *sizes;
	int sz;
	int i;

	HELPD->add_category(dir);

	dirlist = get_dir("~/data/help/" + dir + "/*");

	names = dirlist[0];
	sizes = dirlist[1];
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string entry;

		entry = names[i];

		if (sizes[i] == -2) {
			list_push_back(clist, dir + "/" + entry);
		} else if (sscanf(entry, "%s.hlp", entry)) {
			list_push_back(tlist, ({ dir, entry }) );
		}
	}
}

static void load_tick(mixed **clist, mixed **tlist)
{
	if (!list_empty(clist)) {
		string category;

		category = list_front(clist);
		list_pop_front(clist);

		load_helpdir(category, clist, tlist);

		call_out("load_tick", 0, clist, tlist);
	} else if (!list_empty(tlist)) {
		string dir;
		string entry;

		({ dir, entry }) = list_front(tlist);
		list_pop_front(tlist);

		load_helpfile(dir, entry);

		call_out("load_tick", 0, clist, tlist);
	}
}

static void load_rootdir()
{
	mixed **dirlist;
	string *names;
	int *sizes;
	int sz;
	int i;

	mixed **clist, **tlist;

	clist = ({ nil, nil });
	tlist = ({ nil, nil });

	dirlist = get_dir("~/data/help/*");

	names = dirlist[0];
	sizes = dirlist[1];
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string entry;

		entry = names[i];

		if (sizes[i] == -2) {
			list_push_back(clist, entry);
		} else if (sscanf(entry, "%s.hlp", entry)) {
			list_push_back(tlist, ({ nil, entry }) );
		}
	}

	call_out("load_tick", 0, clist, tlist);
}

void load_help()
{
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	HELPD->reset();
	load_rootdir();
}
