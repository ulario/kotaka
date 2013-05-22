/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <text/paths.h>
#include <kotaka/privilege.h>

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

private object find_verb(string command)
{
	object verb;
	string *dirs;
	int i, sz;

	dirs = gather_dirs("~/sys/verb");
	sz = sizeof(dirs);

	for (i = 0; i < sz; i++) {
		if (verb = find_object(dirs[i] + "/" + command)) {
			return verb;
		}
	}
}

private string raw_convert(mixed **role)
{
	string *build;
	int sz;
	int i;

	build = ({ });

	sz = sizeof(role);

	for (i = 0; i < sz; i++) {
		mixed *phrase;
		mixed *np;

		phrase = role[i];

		switch(phrase[0]) {
		case "V":
			np = phrase[2];

			if (np) {
				build += np[1];
			}

			break;

		case "P":
			build += ({ phrase[1] });
			np = phrase[2];

			if (np) {
				build += np[1];
			}

			break;
		}
	}

	return implode(build, " ");
}

private mapping raw_bind(mapping raw)
{
	mapping bind;
	string *rlist;
	int sz;
	int i;

	bind = ([ ]);
	rlist = map_indices(raw);
	sz = sizeof(rlist);

	for (i = 0; i < sz; i++) {
		bind[rlist[i]] = raw_convert(raw[rlist[i]]);
	}

	return bind;
}

private mixed role_convert(mixed **role, object *candidates)
{
}

private mapping role_bind(mapping roles, object *candidates)
{
	mapping bind;
	string *rlist;
	int sz;
	int i;

	bind = ([ ]);
	rlist = map_indices(roles);
	sz = sizeof(rlist);

	for (i = 0; i < sz; i++) {
		bind[rlist[i]] = role_convert(roles[rlist[i]], candidates);
	}

	return bind;
}

int do_action(object actor, string command, string args)
{
	object ustate;
	object verb;
	object *candidates;
	string statement;
	string crole;
	mapping roles;
	mapping raw;
	mapping prepkey;
	mixed **rules;
	mixed *parse;
	string evoke;
	string *rlist;
	int sz;
	int i;

	ACCESS_CHECK((ustate = previous_object())<-TEXT_LIB_USTATE);

	verb = find_verb(command);

	if (!verb) {
		return FALSE;
	}

	if (sscanf(object_name(verb), "%*s/ic/%*s")) {
		if (!actor) {
			ustate->send_out("You must be in character to use that verb.\n");
			return TRUE;
		}
	}

	statement = command + " " + args;

	parse = ENGLISHD->parse(statement);

	if (!parse) {
		/* choked on bad grammar */
		ustate->send_out("Your grammar stinks.\n");
		return TRUE;
	}

	rules = verb->query_roles();

	sz = sizeof(rules);

	/* ({ role, prepositions, raw }) */
	raw = ([ ]);
	prepkey = ([ ]);
	roles = ([ ]);

	for (i = 0; i < sz; i++) {
		int j;
		int sz2;

		string role;
		string *preps;
		mixed *rule;

		rule = rules[i];

		role = rule[0];
		preps = rule[1];

		if (rule[2]) {
			raw[role] = 1;
		}

		sz2 = sizeof(preps);

		for (j = 0; j < sz2; j++) {
			if (!prepkey[preps[j]]) {
				prepkey[preps[j]] = ({ });
			}

			prepkey[preps[j]] += ({ role });
		}
	}

	sz = sizeof(parse);

	for (i = 0; i < sz; i++) {
		string prep;
		string *np;
		string *rcand;

		switch(parse[i][0]) {
		case "V":
			np = parse[i][2];
			break;

		case "P":
			prep = parse[i][1];
			np = parse[i][2];
			break;

		case "E":
			if (evoke) {
				ustate->send_out("Too many evokes!\n");
				return TRUE;
			}

			evoke = parse[i][1];
			continue;
		default:
			error("Unrecognized parse element");
		}

		if (!crole) {
			crole = "dob";
		}

		if (!raw[crole]) {
			rcand = prepkey[prep];

			if (rcand) {
				int j;
				int sz2;

				sz2 = sizeof(rcand);

				for (j = 0; j < sz2; j++) {
					if (!roles[rcand[j]]) {
						crole = rcand[j];
						break;
					}
				}
			}
		}

		if (!roles[crole]) {
			roles[crole] = ({ });
		}

		roles[crole] += ({ parse[i] });
	}

	rlist = map_indices(raw);
	raw = roles & rlist;
	roles = roles - rlist;
	raw = raw_bind(raw);

	if (actor) {
		object environment;

		candidates = actor->query_inventory();
		environment = actor->query_environment();

		if (environment) {
			candidates += environment->query_inventory();
		}
	} else {
		candidates = ({ });
	}

	roles = role_bind(roles, candidates);

	TLSD->set_tls_value("Text", "ustate", ustate);

	verb->main(actor, roles + raw);

	if (this_object()) {
		TLSD->set_tls_value("Text", "ustate", nil);
	}

	return TRUE;
}
