/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/paths/utility.h>
#include <status.h>
#include <kotaka/privilege.h>

inherit "/lib/linked_list";

mixed **buf;

static void create()
{
	buf = ({ nil, nil });
}

void upgrade()
{
	int sz;
	mixed **callouts;

	ACCESS_CHECK(SYSTEM());

	buf = ({ nil, nil });
	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}
}

void post_message(string file, string msg)
{
	ACCESS_CHECK(TEXT());

	if (list_empty(buf)) {
		call_out("flush", 0);
	}

	list_push_back(buf, ({ file, msg }) );
}

static void flush()
{
	string file;
	string msg;

	({ file, msg }) = list_front(buf);
	list_pop_front(buf);

	SECRETD->make_dir(".");
	SECRETD->make_dir("log");
	SECRETD->write_file(file, msg);

	if (!list_empty(buf)) {
		call_out("flush", 0);
	}
}
