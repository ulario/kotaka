/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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
# include <kernel/kernel.h>
# include <kernel/user.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>

inherit a SECOND_AUTO;
inherit w LIB_SYSTEM_WIZTOOL;
inherit "~/lib/struct/list";
inherit "~/lib/utility/compile";

private object user;		/* associated user object */

static void process(string str);

/*
 * NAME:	create()
 * DESCRIPTION:	initialize object
 */
static void create(int clone)
{
    if (clone) {
	w::create(1);

	user = this_user();

	if (!user) {
	    user = SYSTEM_USERD->query_this_user();
	}
    }
}

/*
 * NAME:	message()
 * DESCRIPTION:	pass on a message to the user
 */
static void message(string str)
{
    user->message(str);
}

/*
 * NAME:	input()
 * DESCRIPTION:	deal with input from user
 */
void input(string str)
{
    if (previous_object() == user) {
	call_limited("process", str);
    } else {
	error("Security violation:  Caller is "
		+ object_name(previous_object())
		+ " but user is " + object_name(user)
	);
    }
}

/*
 * NAME:	process()
 * DESCRIPTION:	process user input
 */
static void process(string str)
{
    string arg;

    if (query_editor(this_object())) {
	if (strlen(str) != 0 && str[0] == '!') {
	    str = str[1 ..];
	} else {
	    str = editor(str);
	    if (str) {
		message(str);
	    }
	    return;
	}
    }

    if (str == "") {
	return;
    }

    sscanf(str, "%s %s", str, arg);
    if (arg == "") {
	arg = nil;
    }

    switch (str) {
    case "code":
    case "history":
    case "clear":
    case "compile":
    case "clone":
    case "destruct":

    case "cd":
    case "pwd":
    case "ls":
    case "cp":
    case "mv":
    case "rm":
    case "mkdir":
    case "rmdir":
    case "ed":

    case "access":
    case "grant":
    case "ungrant":
    case "quota":
    case "rsrc":

    case "people":
    case "status":
    case "swapout":
    case "statedump":
    case "shutdown":
    case "reboot":

    case "dormants":
    case "orphans":
    case "hotboot":
    case "rebuild":
	call_other(this_object(), "cmd_" + str, user, str, arg);
	break;

    default:
	message("No command: " + str + "\n");
	break;
    }
}

/*
 * NAME:	cmd_statedump()
 * DESCRIPTION:	create a state dump, optionally making it incremental
 */
static void cmd_statedump(object user, string cmd, string str)
{
    if (str && str != "-i") {
	message("Usage: " + cmd + " [-i]\n");
	return;
    }

    if (str == "-i") {
	dump_state(1);
    } else {
	dump_state();
    }
}

static void cmd_hotboot(object user, string cmd, string arg)
{
	if (arg) {
		message("Usage: hotboot\n");
		return;
	}

	dump_state(1);
	shutdown(1);
}

private void list_dormants(string dir, mixed **list)
{
	string *names;
	mixed *objs;
	int *times;
	int *sizes;
	int sz;

	if (dir == "/") {
		dir = "";
	}

	({ names, sizes, times, objs }) = get_dir(dir + "/*");

	for (sz = sizeof(names); --sz >= 0; ) {
		if (sizes[sz] == -2) {
			if (names[sz] != "lib") {
				list_dormants(dir + "/" + names[sz], list);
			}

			continue;
		}

		if (!sscanf(names[sz], "%*s.c")) {
			continue;
		}

		if (!objs[sz]) {
			list_push_back(list, dir + "/" + names[sz]);
		}
	}
}

static void cmd_dormants(object user, string cmd, string arg)
{
	mixed **list;

	if (arg) {
		message("Usage: dormants\n");
		return;
	}

	list = ({ nil, nil });

	list_dormants("/", list);

	while (!list_empty(list)) {
		message(list_front(list) + "\n");
		list_pop_front(list);
	}
}

static void cmd_rebuild(object user, string cmd, string arg)
{
	if (arg) {
		message("Usage: rebuild\n");
		return;
	}

	rlimits (0; -1) {
		mixed **list;

		list = OBJECTD->query_program_indices();

		while (!list_empty(list)) {
			int index;
			object pinfo;
			string path;

			index = list_front(list);
			list_pop_front(list);

			pinfo = OBJECTD->query_program_info(index);

			if (!pinfo || pinfo->query_destructed()) {
				continue;
			}

			path = pinfo->query_path();

			if (sscanf(path, "%*s" + INHERITABLE_SUBDIR) || !file_info(path + ".c")) {
				destruct_object(path);
			}
		}

		purge_dir("/");
		compile_dir("/");
	}
}

static void cmd_orphans(object user, string cmd, string arg)
{
	mixed **list;

	if (arg) {
		message("Usage: orphans\n");
		return;
	}

	list = OBJECTD->query_program_indices();

	rlimits(0; -1) {
		while (!list_empty(list)) {
			int index;
			string path;
			object pinfo;

			index = list_front(list);
			list_pop_front(list);

			pinfo = OBJECTD->query_program_info(index);

			if (pinfo->query_destructed()) {
				continue;
			}

			path = pinfo->query_path();

			if (!file_info(path + ".c")) {
				message(path + "\n");
			}
		}
	}
}
