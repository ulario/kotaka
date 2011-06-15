#include <status.h>

#include <kernel/access.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/checkarg.h>
#include <kotaka/assert.h>
#include <kotaka/bigstruct.h>

inherit SECOND_AUTO;

mapping facilities;
string timestamp;
mapping filebufs;
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
	callout = -1;
}

private void schedule()
{
	if (callout != -1) {
		remove_call_out(callout);
	}

	callout = call_out("flush", 0);
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

private string timestamp()
{
	int sec;
	int msec;

	string str_sec;
	string str_msec;
	string str_date;
	mixed *mtime;
	string timestamp;

	mtime = millitime();
	sec = mtime[0];
	msec = (int)(mtime[1] * 1000.0);
	str_sec = "          " + sec;
	str_sec = str_sec[strlen(str_sec) - 10 ..];
	str_msec = "00" + msec;
	str_msec = str_msec[strlen(str_msec) - 3 ..];

	return "[" + str_sec + "." + str_msec + "]";
}

private void write_logfile(string file, string message)
{
	int i;
	int sz;
	string *lines;
	object deque;

	lines = explode("\n" + message + "\n", "\n");

	sz = sizeof(lines);

	if (!filebufs) {
		filebufs = ([ ]);
	}

	deque = filebufs[file];

	if (!deque) {
		deque = new_object(BIGSTRUCT_DEQUE_LWO);
		filebufs[file] = deque;
	}

	for (i = 0; i < sz; i++) {
		deque->push_back(timestamp + " " + lines[i] + "\n");
	}

	schedule();
}

void flush()
{
	string *files;
	object *text_deques;
	int i;

	callout = -1;

	ACCESS_CHECK(SYSTEM() || KADMIN() || KERNEL());

	rlimits(0; -1) {
		files = map_indices(filebufs);
		text_deques = map_values(filebufs);

		for (i = sizeof(files) - 1; i >= 0; i--) {
			catch {
				object deque;

				deque = text_deques[i];

				while (!deque->empty()) {
					string text;
					text = deque->get_front();
					deque->pop_front();
					if (!write_file(files[i], text)) {
						DRIVER->message("LogD: error writing to " + files[i] + "\n");
					}
				}
			}
		}
	}

	filebufs = ([ ]);
}

private void send_to_target(string target, string header, string message)
{
	string prefix, info;
	string *lines;

	sscanf(target, "%s:%s", prefix, info);

	if (!prefix) {
		ASSERT(!info);
		prefix = target;
	}

	lines = explode(message, "\n");
	message = header + ": " + implode(lines, "\n" + header + ": ");

	switch(prefix) {
	case "null":
		break;

	case "driver":
		DRIVER->message(message + "\n");
		break;

	case "channel":
		ASSERT(info);
		CHANNELD->post_message(info, "Logger", message);
		break;

	case "file":
		ASSERT(info);
		write_logfile(info, message);
		break;

	case "kusers":
		{
			object *users;
			int i;
			int sz;
			
			users = users();
			
			sz = sizeof(users);
			
			for (i = 0; i < sz; i++) {
				users[i]->message(message + "\n");
			}
		}
		break;

	case "kwizards":
		{
			string *kwizards;
			object *users;
			int i;
			int sz;

			kwizards = KERNELD->query_users();
			users = users();

			sz = sizeof(users);

			for (i = 0; i < sz; i++) {
				if (
					sizeof(
						({ users[i]->query_name() })
						& kwizards
					)
				) {
					users[i]->message(message + "\n");
				}
			}
		}
		break;

	case "kadmins":
		{
			string *kwizards;
			object *users;
			int i;
			int sz;
			
			users = users();
			sz = sizeof(users);
			
			for (i = 0; i < sz; i++) {
				string username;
				
				username = users[i]->query_name();
				
				if (KERNELD->access(username, "/", FULL_ACCESS)) {
					users[i]->message(message + "\n");
				}
			}
		}
		break;

	case "kadmin":
		{
			object kadmin;
			
			kadmin = KERNELD->find_user("admin");

			if (kadmin) {
				kadmin->message(message + "\n");
			}
		}
		break;

	default:
		DRIVER->message("Unparseable log target: " + target + "\n");
	}
}

void post_message(string facility, int priority, string message)
{
	string creator;
	mapping hits;

	ACCESS_CHECK(PRIVILEGED());

	CHECKARG(facility, 1, "post_message");
	CHECKARG(facility != "", 1, "post_message");
	CHECKARG(priority >= 0, 2, "post_message");
	CHECKARG(priority <= 7, 2, "post_message");
	CHECKARG(message, 3, "post_message");
	CHECKARG(message != "", 3, "post_message");

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
	
	if (facilities["*"]) {
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
			send_to_target(targets[index], facility, message);
		}
	} else {
		DRIVER->message(creator + ": " + facility + ": " + message + "\n");
	}
}
