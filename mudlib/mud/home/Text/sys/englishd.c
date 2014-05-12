/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <type.h>

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
		string *match;

		match = candidates[i]->query_property("snouns")
			| candidates[i]->query_property("pnouns");

		if (sizeof(({ noun }) & match)) {
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
			switch(phrase[0]) {
			case "from":
			case "in":
				candidates = candidates[0]->query_inventory();
				break;
			default:
				TLSD->set_tls_value("Text", "parse_error", "Cannot handle relation: " + phrase[0]);
				return nil;
			}
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

private mapping english_process(string command, object ustate, object actor, object verb, string args)
{
	string statement;
	mixed *parse;
	string **rules;
	mapping roles;
	int i, sz;
	mapping raw;
	mapping prepkey;
	string evoke;
	string crole;
	string *rlist;
	object *def_candidates;
	mapping candidates;

	statement = command + " " + args;
	parse = PARSER_ENGLISH->parse(statement);

	if (!parse) {
		/* choked on bad grammar */
		return ([ "error": "Parsing error" ]);
	}

	rules = verb->query_roles();

	sz = sizeof(rules);

	/* ({ role, prepositions, raw }) */
	raw = ([ ]);
	prepkey = ([ ]);
	roles = ([ ]);

	/* build the prepkey and mark raw roles */
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

	/* assign roles */
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

	/* bind roles */
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
		return ([ "error" : TLSD->query_tls_value("Text", "parse_error") ]);
	}

	if (evoke) {
		roles["evoke"] = evoke;
	}

	roles += raw;

	return roles;
}

int do_verb(object verb, string command, string args)
{
	object ustate;
	object actor;
	mixed roles;
	int i, sz;
	string *methods;
	string err;

	ACCESS_CHECK((ustate = previous_object())<-TEXT_LIB_USTATE);

	actor = ustate->query_user()->query_body();

	TLSD->set_tls_value("Text", "ustate", ustate);

	methods = verb->query_parse_methods();

	sz = sizeof(methods);

	for (i = 0; i < sz && !roles; i++) {
		int leave;

		switch(methods[i]) {
		case "raw":
			roles = ([ "raw" : args ]);
			verb->main(actor, roles);
			break;

		case "english":
			roles = english_process(command, ustate, actor, verb, args);

			if (typeof(roles) == T_STRING) {
				err = roles;
			} else if (roles["error"]) {
				err = roles["error"];
				roles = nil;
			} else if (typeof(roles) == T_MAPPING) {
				verb->main(actor, roles);
			}
		}
	}

	if (err) {
		ustate->send_out(err + "\n");
	}

	if (this_object()) {
		TLSD->set_tls_value("Text", "ustate", nil);
	}

	return TRUE;
}
