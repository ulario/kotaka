/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2019  Raymond Jennings
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
#include <status.h>

inherit "~System/lib/struct/list";

private mapping buffers;

private void append_node(string file, string fragment)
{
	mixed **list;

	if (!buffers) {
		buffers = ([ ]);
	}

	list = buffers[file];

	if (!list) {
		list = ({ nil, nil });
		buffers[file] = list;
	}

	list_append_string(list, fragment);
}

private void write_node(string file)
{
	mixed **list;
	mixed *info;

	list = buffers[file];

	info = SECRETD->file_info("logs/" + file + ".log");

	if (info && info[0] >= 1 << 25) {
		SECRETD->remove_file("logs/" + file + ".log.old");
		SECRETD->rename_file("logs/" + file + ".log", "logs/" + file + ".log.old");
	}

	SECRETD->make_dir(".");
	SECRETD->make_dir("logs");
	SECRETD->write_file("logs/" + file + ".log", list_front(list));

	list_pop_front(list);

	if (list_empty(list)) {
		buffers[file] = nil;
	}
}

static nomask void secret_flush()
{
	while (buffers && map_sizeof(buffers)) {
		string *files;
		int sz;

		files = map_indices(buffers);

		sz = sizeof(files);

		write_node(files[random(sz)]);
	}

	buffers = nil;
}

static void flush()
{
	call_out("secret_flush", 0);
}

static void write_secret_log(string file, string message)
{
	mixed *mtime;
	string stamp, mstamp;
	mixed **callouts;
	int sz;

	mtime = millitime();

	/* ctime format: */
	/* Tue Aug  3 14:40:18 1993 */
	/* 012345678901234567890123 */
	stamp = ctime(mtime[0]);
	stamp = stamp[0 .. 2] + ", " + stamp[4 .. 9] + ", " + stamp[20 .. 23] + " " + stamp[11 .. 18];

	stamp += ".";

	/* millitime resolution is 1m anyway */
	mstamp = "" + floor(mtime[1] * 1000.0 + 0.5);
	mstamp = "000" + mstamp;
	mstamp = mstamp[strlen(mstamp) - 3 ..];

	stamp += mstamp;

	append_node(file, stamp + " " + message + "\n");

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		if (callouts[sz][CO_FUNCTION] == "secret_flush") {
			return;
		}
	}

	call_out("secret_flush", 0);
}
