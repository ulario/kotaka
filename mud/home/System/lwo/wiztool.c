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
# include <kernel/kernel.h>
# include <kernel/user.h>
#include <kotaka/paths/system.h>

inherit a SECOND_AUTO;
inherit w LIB_SYSTEM_WIZTOOL;

private object user;		/* associated user object */

static void process(string str);

/*
 * NAME:	create()
 * DESCRIPTION:	initialize object
 */
static void create(int clone)
{
    if (clone) {
	w::create(200);

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
	if (query_owner() == "admin") {
	    process(str);
	} else {
	    call_limited("process", str);
	}
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
    case "hotboot":
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

private void list_dormants(mixed **list, string dir)
{
	string *names;
	mixed *objs;
	mixed *dummy;
	int sz;

	if (dir == "/") {
		dir = "";
	}

	({ names, dummy, dummy, objs }) = get_dir(dir + "/*");

	for (sz = sizeof(names); --sz >= 0; ) {
		if (sizes[sz] == -2) {
			if (names[sz] != "lib") {
				list_dormants(list, dir + "/" + names[sz]);
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
	proxy = PROXYD->get_proxy(query_user()->query_name());

	list_dormants(list, proxy, "/");

	if (list_empty(list)) {
		send_out("There are no dormant LPC source files.\n");
		return;
	}

	send_out("These LPC files are not compiled:\n");

	while (!list_empty(list)) {
		send_out(list_front(list) + "\n");
		list_pop_front(list);
	}
}
