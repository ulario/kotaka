/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019  Raymond Jennings
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
#include <kotaka/log.h>

inherit "~System/lib/utility/secretlog";
inherit "~System/lib/struct/list";

mixed **buf;

static void flush()
{
	if (!buf) {
		return;
	}

	if (list_empty(buf)) {
		buf = nil;
		return;
	} else {
		string file;
		string msg;
		mixed *info;

		call_out("flush", 0);

		({ file, msg }) = list_front(buf);
		list_pop_front(buf);

		if (list_empty(buf)) {
			buf = nil;
		}

		SECRETD->make_dir(".");
		SECRETD->make_dir("log");

		if (info = SECRETD->file_info(file)) {
			/* ({ file size, file modification time, object }) */
			int size;
			int time;
			mixed obj;

			({ size, time, obj }) = info;

			if (size + strlen(msg) > (32 << 20)) {
				SECRETD->remove_file(file + ".old");
				SECRETD->rename_file(file, file + ".old");
			}
		}

		SECRETD->write_file(file, msg);
	}
}

void log_message(string sender, string message)
{
	ACCESS_CHECK(TEXT());

	write_secret_log(sender, timestamp() + " " + message);
}
