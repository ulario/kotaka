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

inherit SECOND_AUTO;
inherit "~System/lib/struct/list";

private mapping queues;

private void append_node(string file, string fragment)
{
	mixed **list;

	if (!queues) {
		queues = ([ ]);
	}

	list = queues[file];

	if (!list) {
		list = ({ nil, nil });
		queues[file] = list;
	}

	list_push_back(list, fragment);
}

private void write_node(string file)
{
	mixed **list;
	mixed *info;

	list = queues[file];

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
		queues[file] = nil;
	}

	if (!map_sizeof(queues)) {
		queues = nil;
	}
}

private void flush_one()
{
	if (queues && map_sizeof(queues)) {
		string *files;
		int sz;

		files = map_indices(queues);

		sz = sizeof(files);

		write_node(files[random(sz)]);
	}
}

static string timestamp()
{
	mixed *mtime;
	string stamp, mstamp;

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

	return stamp;
}

static void write_secret_log(string file, string message)
{
	if (!file) {
		error("Invalid filename");
	}

	if (!message) {
		error("Invalid message");
	}


	if (message[strlen(message) - 1] != '\n') {
		message += "\n";
	}

	append_node(file, message);

	call_out_unique("secret_flush", 0);
}

static void secret_flush()
{
	flush_one();

	if (queues) {
		call_out_unique("secret_flush", 0);
	}
}

void flush()
{
	while (queues) {
		flush_one();
	}
}
