/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <text/parse.h>
#include <text/paths.h>

string *verbdirs;

static void create()
{
	verbdirs = ({ USR_DIR + "/Text/sys/verb", USR_DIR + "/Game/sys/verb" });
}

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

	sz = sizeof(verbdirs);
	dirs = ({ });

	for (i = 0; i < sz; i++) {
		dirs += gather_dirs(verbdirs[i]);
	}

	sz = sizeof(dirs);

	for (i = 0; i < sz; i++) {
		if (verb = find_object(dirs[i] + "/" + command)) {
			return verb;
		}
	}
}

void add_verb_directory(string directory)
{
	verbdirs |= ({ directory });
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

private object *filter_noun(object *candidates, string noun)
{
	object *contenders;
	int sz;
	int i;

	sz = sizeof(candidates);
	contenders = ({ });

	noun = STRINGD->to_lower(noun);

	for (i = 0; i < sz; i++) {
		if (sizeof(({ noun }) & candidates[i]->query_property("snouns"))) {
			contenders += ({ candidates[i] });
		}
	}

	return contenders;
}

private object *filter_adjectives(object *candidates, string *adjectives)
{
	object *contenders;
	int sz;
	int i;

	sz = sizeof(candidates);
	contenders = ({ });

	for (i = 0; i < sz; i++) {
		if (!sizeof(adjectives - candidates[i]->query_property("adjectives"))) {
			contenders += ({ candidates[i] });
		}
	}

	return contenders;
}

private object *gather_relation(string prep, object source)
{
	switch(prep) {
	case "from":
	case "in":
		return source->query_inventory();
	default:
		TLSD->set_tls_value("Text", "parse_error", "Cannot handle relation: " + prep);
		return nil;
	}
}

private string *select_ordinals(string *adjectives)
{
	int sz;
	int i;
	string *ordinals;

	sz = sizeof(adjectives);
	ordinals = ({ });

	for (i = 0; i < sz; i++) {
		string adjective;

		adjective = adjectives[i];

		switch(adjective) {
		case "first":
		case "second":
		case "third":
		case "fourth":
		case "fifth":
		case "sixth":
		case "seventh":
		case "eighth":
		case "ninth":
		case "tenth":
			ordinals += ({ adjective });
			break;
		default:
			if (sscanf(adjective, "%*dst")) {
				ordinals += ({ adjective });
				break;
			}
			if (sscanf(adjective, "%*dnd")) {
				ordinals += ({ adjective });
				break;
			}
			if (sscanf(adjective, "%*drd")) {
				ordinals += ({ adjective });
				break;
			}
			if (sscanf(adjective, "%*dth")) {
				ordinals += ({ adjective });
				break;
			}
		}
	}

	return ordinals;
}

private int resolve_ordinal_number(string ordinal)
{
	switch(ordinal) {
	case "first":
		return 1;
	case "second":
		return 2;
	case "third":
		return 3;
	case "fourth":
		return 4;
	case "fifth":
		return 5;
	case "sixth":
		return 6;
	case "seventh":
		return 7;
	case "eighth":
		return 8;
	case "ninth":
		return 9;
	case "tenth":
		return 10;
	}
}

private object *filter_ordinals(object *candidates, string *ordinals)
{
	int number;

	if (sizeof(ordinals) > 1) {
		TLSD->set_tls_value("Text", "parse_error", "Too many ordinals");
		return ({ });
	}

	if (sizeof(ordinals) == 0) {
		return candidates;
	}

	number = resolve_ordinal_number(ordinals[0]);

	if (number > sizeof(candidates)) {
		TLSD->set_tls_value("Text", "parse_error", "There aren't that many");
		return ({ });
	}

	return ({ candidates[number - 1] });
}

private mixed role_convert(mixed **role, object *initial)
{
	int sz, i;
	object *candidates;
	string *adj;
	string noun;

	sz = sizeof(role);
	candidates = initial;

	for (i = sz - 1; i >= 0; i--) {
		mixed *phrase;
		string *np;
		string *ordinals;
		/* 1.  find np in candidates */
		/* 2.  build new candidates using the preposition */

		phrase = role[i];

		if (!phrase[1]) {
			TLSD->set_tls_value("Text", "parse_error", "Bad grammar");
			return nil;
		}

		np = phrase[1][1];

		noun = np[sizeof(np) - 1];
		adj = np[0 .. sizeof(np) - 2];

		ordinals = select_ordinals(adj);
		adj -= ordinals;

		candidates = filter_noun(candidates, noun);
		candidates = filter_adjectives(candidates, adj);
		candidates = filter_ordinals(candidates, ordinals);

		if (sizeof(candidates) == 0) {
			TLSD->set_tls_value("Text", "parse_error", "There is no " + implode(adj + ({ noun }), " ") + ".");
			return nil;
		} else if (sizeof(candidates) > 1) {
			TLSD->set_tls_value("Text", "parse_error", "There is more than one " + implode(adj + ({ noun }), " ") + ".");
			return nil;
		}

		if (i > 0) {
			candidates = gather_relation(phrase[0], candidates[0]);
		}

		if (TLSD->query_tls_value("Text", "parse_error")) {
			return nil;
		}
	}

	return ({ role[0][0], candidates[0] });
}

private mapping role_bind(mapping roles, mapping initial)
{
	mapping bind;
	string *rlist;
	int sz;
	int i;

	bind = ([ ]);
	rlist = map_indices(roles);
	sz = sizeof(rlist);

	for (i = 0; i < sz; i++) {
		bind[rlist[i]] = role_convert(roles[rlist[i]], initial[rlist[i]]);

		if (TLSD->query_tls_value("Text", "parse_error")) {
			return ([ ]);
		}
	}

	return bind;
}

int do_verb(string command, string args)
{
	object verb, actor, ustate;
	object *def_candidates;
	string statement, crole, evoke;
	mixed **parse, **rules;
	int sz, i;
	mapping raw, roles, prepkey, candidates;
	string *rlist;

	ACCESS_CHECK((ustate = previous_object())<-TEXT_LIB_USTATE);

	verb = find_verb(command);

	if (!verb) {
		return FALSE;
	}

	actor = ustate->query_user()->query_body();

	if (verb->query_parse_method() == PARSE_RAW) {
		TLSD->set_tls_value("Text", "ustate", ustate);
		verb->main(actor, args);
		if (this_object()) {
			TLSD->set_tls_value("Text", "ustate", nil);
		}
		return TRUE;
	} else {
		if (!actor) {
			ustate->send_out("You must be in character to use this command.\n");
			return TRUE;
		}
	}

	TLSD->set_tls_value("Text", "ustate", ustate);

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

			if (!np) {
				continue;
			}

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

		if (!roles[crole]) {
			roles[crole] = ({ });
		}

		roles[crole] += ({ ({ prep, np }) });
	}

	rlist = map_indices(raw);
	raw = roles & rlist;
	roles = roles - rlist;
	raw = raw_bind(raw);

	if (actor) {
		object environment;

		def_candidates = actor->query_inventory();
		environment = actor->query_environment();

		if (environment) {
			def_candidates += environment->query_inventory();
		}
	} else {
		def_candidates = ({ });
	}

	rlist = map_indices(roles);
	candidates = ([ ]);
	sz = sizeof(rlist);

	/* todo: query verb for candidates for each role */
	for (i = 0; i < sz; i++) {
		candidates[rlist[i]] = def_candidates[..];
	}

	roles = role_bind(roles, candidates);

	if (TLSD->query_tls_value("Text", "parse_error")) {
		ustate->send_out(TLSD->query_tls_value("Text", "parse_error") + "\n");
		return TRUE;
	}

	verb->do_action(actor, roles + raw, evoke);

	if (this_object()) {
		TLSD->set_tls_value("Text", "ustate", nil);
	}

	return TRUE;
}
