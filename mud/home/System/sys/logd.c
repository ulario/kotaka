/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <status.h>
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

mapping facilities;
string timestamp;
mapping filebufs;
mapping buffers;
/* ([ filename: ({ first, last }) ]), node: ({ prev, text, next }) */
int callout;

/*
targets:

prefix:info

([ facilities: ([ target: mask ]) ])

Prefixes:

null: ignore the message
	Used as a placeholder for "explicit nop" that will suppress
	the default "shout to driver" done when no targets are present.
file: write to a file
channel: post to a channel
kuser: message a klib user

*/

static void create()
{
	facilities = ([ ]);
	filebufs = ([ ]);
	callout = 0;

	load_object("~/lwo/logbuf");
}

private void schedule()
{
	if (!callout) {
		callout = call_out("flush", 0);
	}
}

void clear_targets()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	facilities = ([ ]);
}

void set_target(string facility, int mask, string target_info)
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	CHECKARG(facility, 1, "set_target");
	CHECKARG(facility != "", 1, "set_target");
	CHECKARG(mask >= 0, 2, "set_target");
	CHECKARG(mask <= 255, 2, "set_target");
	CHECKARG(target_info, 3, "set_target");
	CHECKARG(target_info != "", 3, "set_target");

	if (mask) {
		mapping targets;

		targets = facilities[facility];

		if (!targets) {
			facilities[facility] = targets = ([ ]);
		}

		targets[target_info] = mask;
	} else {
		mapping targets;

		targets = facilities[facility];

		if (!targets) {
			return;
		}

		targets[target_info] = nil;

		if (!map_sizeof(targets)) {
			facilities[facility] = nil;
		}
	}
}

/*
000000000011111111112222
012345678901234567890123
WWW MMM DD HH:MM:SS YYYY

MMM DD HH:MM
MMM DD  YYYY
*/

private string timestamp()
{
	string c;

	c = ctime(time());

	return "[" + c + "]";
}

private void append_node(string file, string fragment)
{
	mixed *header;
	mixed *node;
	int max;
	int len;

	if (!buffers) {
		buffers = ([ ]);
	}

	header = buffers[file];

	if (!header) {
		node = ({ nil, "", nil });
		header = ({ node, node });
		buffers[file] = header;
	} else {
		node = header[1];
	}

	max = status(ST_STRSIZE);

	while (len = strlen(fragment)) {
		int spare;

		spare = max - strlen(node[1]);

		if (spare >= len) {
			node[1] += fragment;

			return;
		} else if (spare > 0) {
			mixed *newnode;

			node[1] += fragment[0 .. spare - 1];
			fragment = fragment[spare ..];

			newnode = ({ node, "", nil });
			node[2] = newnode;
			header[1] = newnode;
		}
	}
}

private void write_logfile(string file, string message)
{
	int i;
	int sz;
	string line;
	string *lines;
	object deque;

	lines = explode("\n" + message + "\n", "\n");

	sz = sizeof(lines);

	for (i = 0; i < sz; i++) {
		append_node(file, timestamp + " " + lines[i] + "\n");
	}

	schedule();
}

private void write_node(string file)
{
	mixed *header;
	mixed *node;

	header = buffers[file];
	node = header[0];

	write_file(file, node[1]);

	node[0] = nil;

	if (node[2]) {
		header[0] = node[2];
	} else {
		buffers[file] = nil;
	}
}

void flush()
{
	string *files;
	object *text_deques;
	int i;

	callout = 0;

	ACCESS_CHECK(SYSTEM() || KADMIN() || KERNEL());

	rlimits(0; -1) {
		files = map_indices(filebufs);
		text_deques = map_values(filebufs);

		for (i = sizeof(files) - 1; i >= 0; i--) {
			catch {
				string text;
				mixed buf;

				buf = filebufs[files[i]];
				text = buf->pop();

				if (!text) {
					filebufs[files[i]] = nil;
					continue;
				}

				if (!write_file(files[i], text)) {
					DRIVER->message("LogD: error writing to " + files[i] + "\n");
				}
			}
		}
	}

	if (map_sizeof(filebufs)) {
		schedule();
	}

	if (map_sizeof(buffers)) {
		string *files;
		int sz;

		files = map_indices(buffers);

		sz = sizeof(files);

		write_node(files[random(sz)]);

		call_out("flush", 0);
	}
}

private void send_to_target(string target, string message)
{
	string prefix, info;
	string *lines;
	int sz, usz, i, j;
	mixed users;

	sscanf(target, "%s:%s", prefix, info);

	if (!prefix) {
		ASSERT(!info);
		prefix = target;
	}

	lines = explode(message, "\n");
	sz = sizeof(lines);
	users = ([ ]);

	switch(prefix) {
	case "null":
		break;

	case "driver":
		for (i = 0; i < sz; i++) {
			DRIVER->message(lines[i] + "\n");
		}
		break;

	case "channel":
		ASSERT(info);

		if (find_object(CHANNELD)) {
			for (i = 0; i < sz; i++) {
				CHANNELD->post_message(info, nil, lines[i]);
			}
		}

		break;

	case "file":
		ASSERT(info);

		for (i = 0; i < sz; i++) {
			write_logfile(info, lines[i]);
		}

		break;

	case "user":
		if (this_user()) {
			users[this_user()] = 1;
		}
		break;

	case "kusers":
		{
			object *kusers;
			int i;
			int ksz;

			kusers = users();

			ksz = sizeof(users);

			for (i = 0; i < ksz; i++) {
				users[kusers[i]] = 1;
			}
		}
		break;

	case "kwizards":
		{
			string *kwizards;
			object *kusers;
			int i;
			int ksz;

			kwizards = KERNELD->query_users();
			kusers = users();

			ksz = sizeof(kusers);

			for (i = 0; i < ksz; i++) {
				if (
					sizeof(
						({ kusers[i]->query_name() })
						& kwizards
					)
				) {
					users[kusers[i]] = 1;
				}
			}
		}
		break;

	case "kadmins":
		{
			string *kwizards;
			object *kusers;
			int i;
			int ksz;

			kusers = users();
			ksz = sizeof(kusers);

			for (i = 0; i < ksz; i++) {
				string username;

				username = kusers[i]->query_name();

				if (KERNELD->access(username, "/", FULL_ACCESS)) {
					users[kusers[i]] = 1;
				}
			}
		}
		break;

	case "kadmin":
		{
			object kadmin;

			kadmin = KERNELD->find_user("admin");

			if (kadmin) {
				users[kadmin] = 1;
			}
		}
		break;

	default:
		DRIVER->message("Unparseable log target: " + target + "\n");
	}

	users = map_indices(users);
	usz = sizeof(users);

	for (i = 0; i < usz; i++) {
		object user;

		user = users[i];

		for (j = 0; j < sz; j++) {
			user->message(lines[j] + "\n");
		}
	}
}

void post_message(string facility, int priority, string message)
{
	string creator;
	mapping hits;

	CHECKARG(facility, 1, "post_message");
	CHECKARG(facility != "", 1, "post_message");
	CHECKARG(priority >= 0, 2, "post_message");
	CHECKARG(priority <= 7, 2, "post_message");
	CHECKARG(message, 3, "post_message");
	CHECKARG(message != "", 3, "post_message");

	catch {
		timestamp = timestamp();
		hits = ([ ]);

		if (facilities[facility]) {
			mapping submap;
			string *targets;
			int *masks;
			int index;
			int sz;

			submap = facilities[facility];
			targets = map_indices(submap);
			masks = map_values(submap);

			sz = map_sizeof(submap);

			for (index = 0; index < sz; index++) {
				if (masks[index] & (1 << priority)) {
					hits[targets[index]] = 1;
				}
			}
		}

		if (!hits["null"] && facilities["*"]) {
			mapping submap;
			string *targets;
			int *masks;
			int index;
			int sz;

			submap = facilities["*"];
			targets = map_indices(submap);
			masks = map_values(submap);

			sz = map_sizeof(submap);

			for (index = 0; index < sz; index++) {
				if (masks[index] & (1 << priority)) {
					hits[targets[index]] = 1;
				}
			}
		}

		creator = DRIVER->creator(object_name(previous_object()));

		if (map_sizeof(hits)) {
			string *targets;
			int sz;
			int index;

			targets = map_indices(hits);
			sz = sizeof(targets);

			for (index = 0; index < sz; index++) {
				send_to_target(targets[index], message);
			}
		} else {
			DRIVER->message(message + "\n");
		}
	} : {
		DRIVER->message("Error logging: " + creator + ": " + facility + ": " + message + "\n");
	}
}
