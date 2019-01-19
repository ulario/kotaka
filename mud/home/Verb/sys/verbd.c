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
#include <config.h>
#include <kotaka/privilege.h>
#include <kotaka/help.h>
#include <kotaka/log.h>

inherit "/lib/string/char";
inherit "/lib/string/format";

private string *gather_dirs(string dir)
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

private void purge_help_dir(string dir)
{
	string *categories;
	string *topics;
	int sz;

	categories = HELPD->query_categories("verbs/" + dir);

	for (sz = sizeof(categories); --sz >= 0; ) {
		purge_help_dir(dir + "/" + categories[sz]);
	}

	topics = HELPD->query_topics("verbs/" + dir);

	for (sz = sizeof(topics); --sz >= 0; ) {
		HELPD->remove_topic(dir + "/" + topics[sz]);
	}
}

private string process_help_contents(string *paragraphs)
{
	string help;
	int sz, i;

	if (!paragraphs) {
		return "";
	}

	help = "";
	sz = sizeof(paragraphs);

	for (i = 0; i < sz; i++) {
		help += wordwrap(paragraphs[i], 70) + "\n";

		if (i < sz - 1) {
			help += "\n";
		}
	}

	return help;
}

static void sync_help_dir(string dir)
{
	string *names;
	int *sizes;
	int *times;
	object *objs;
	int sz;

	string *categories;
	string *topics;

	if (dir) {
		({ names, sizes, times, objs }) = get_dir("~/sys/verb/" + dir + "/*");
	} else {
		({ names, sizes, times, objs }) = get_dir("~/sys/verb/*");
	}

	if (dir) {
		categories = HELPD->query_categories("verbs/" + dir);
		topics = HELPD->query_topics("verbs/" + dir);
	} else {
		categories = HELPD->query_categories("verbs");
		topics = HELPD->query_topics("verbs");
	}

	if (!categories) {
		categories = ({ });
	}

	if (!topics) {
		topics = ({ });
	}

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;
		string fullname;

		name = names[sz];

		if (dir) {
			fullname = dir + "/" + name;
		} else {
			fullname = name;
		}

		if (sizes[sz] == -2) {
			categories -= ({ name });
			call_out("sync_help_dir", 0, fullname);
			continue;
		}

		if (sscanf(name, "%s.c", name) == 1 && objs[sz]) {
			string help;
			object verb;

			topics -= ({ name });

			verb = objs[sz];

			help = verb->query_help_title();
			if (!help) {
				help = "ERROR";
			}
			help += "\n" + chars('-', strlen(help)) + "\n\n";
			help += process_help_contents(verb->query_help_contents()) + "\n";

			HELPD->add_topic("verbs/" + fullname[0 .. strlen(fullname) - 3], help);
		}
	}

	for (sz = sizeof(categories); --sz >= 0; ) {
		if (dir) {
			purge_help_dir(dir + "/" + categories[sz]);
		} else {
			purge_help_dir(categories[sz]);
		}
	}

	for (sz = sizeof(topics); --sz >= 0; ) {
		if (dir) {
			HELPD->remove_topic("verbs/" + dir + "/" + topics[sz]);
		} else {
			HELPD->remove_topic("verbs/" + topics[sz]);
		}
	}
}

object find_verb(string command)
{
	object verb;
	string *dirs;
	int i, sz;

	dirs = gather_dirs("verb");

	sz = sizeof(dirs);

	for (i = 0; i < sz; i++) {
		if (verb = find_object(dirs[i] + "/" + command)) {
			return verb;
		}
	}
}

void sync_help()
{
	ACCESS_CHECK(GAME() || VERB());

	HELPD->add_topic("verbs/fudge", "Chocolatey goodness");

	call_out("sync_help_dir", 0, nil);
}
