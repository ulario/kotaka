/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2001, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/checkarg.h>
#include <kotaka/privilege.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <type.h>

private mixed *filter_noun(object *candidates, string noun)
{
	object *scont;
	object *pcont;
	int sz;
	int i;

	sz = sizeof(candidates);

	noun = STRINGD->to_lower(noun);

	/* singular nouns must match one and only one object */
	/* plural nouns can match multiples */

	scont = ({ });
	pcont = ({ });

	for (i = 0; i < sz; i++) {
		string *snouns;
		string *pnouns;

		snouns = candidates[i]->query_property("snouns");
		pnouns = candidates[i]->query_property("pnouns");

		if (sizeof(({ noun }) & snouns)) {
			scont += ({ candidates[i] });
		}

		if (sizeof(({ noun }) & pnouns)) {
			pcont += ({ candidates[i] });
		}
	}

	return ({ 3, scont | pcont, !sizeof(pcont) && sizeof(scont) });
}

private mixed *filter_adjectives(object *candidates, string *adjectives)
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

	return ({ 3, contenders });
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
	int number;

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
	default:
		if (sscanf(ordinal, "%dst", number)) {
			return number;
		}
		if (sscanf(ordinal, "%dnd", number)) {
			return number;
		}
		if (sscanf(ordinal, "%drd", number)) {
			return number;
		}
		if (sscanf(ordinal, "%dth", number)) {
			return number;
		}
	}
}

private mixed *filter_ordinals(object *candidates, string *ordinals)
{
	int number;

	if (sizeof(ordinals) > 1) {
		return ({ 0, "Too many ordinals" });
	}

	if (sizeof(ordinals) == 0) {
		return ({ 3, candidates });
	}

	number = resolve_ordinal_number(ordinals[0]);

	if (number > sizeof(candidates)) {
		return ({ 2, "There aren't that many to pick from." });
	}

	return ({ 3, ({ candidates[number - 1] }) });
}

private string bind_raw(mixed **phrases)
{
	string *build;
	int sz;
	int i;

	build = ({ });

	sz = sizeof(phrases);

	for (i = 0; i < sz; i++) {
		mixed *phrase;
		mixed *np;

		phrase = phrase[i];

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

private mixed *bind_english(mixed **phrases, object *initial)
{
	int sz, i;
	object *candidates;
	string *adj;
	string noun;

	sz = sizeof(phrases);
	candidates = initial;

	for (i = sz - 1; i >= 0; i--) {
		mixed *phrase;
		string *np;
		string *ordinals;
		string pre;
		mixed *result;
		int exact;
		/* 1.  find np in candidates */
		/* 2.  build new candidates using the preposition */

		phrase = phrases[i];

		if (!phrase[1]) {
			return ({ 0, "Bad grammar" });
		}

		np = phrase[1][1];

		noun = np[sizeof(np) - 1];
		adj = np[0 .. sizeof(np) - 2];

		ordinals = select_ordinals(adj);
		adj -= ordinals;

		result = filter_adjectives(candidates, adj);

		if (result[0] != 3) {
			return result;
		}

		result = filter_noun(result[1], noun);
		exact = result[2];

		if (result[0] != 3) {
			switch(result[1]) {
			case "NOMATCH":
				return ({ 2, "There is no " + implode(adj + ({ noun }), " ") });

			case "MULTIPLE":
				return ({ 2, "Be more specific, there is more than one " + implode(adj + ({ noun }), " ") });

			}
			return result;
		}

		result = filter_ordinals(result[1], ordinals);

		if (result[0] != 3) {
			return result;
		}

		candidates = result[1];

		{
			int sz;

			for (sz = sizeof(candidates); --sz >= 0; ) {
				if (candidates[sz]->query_property("is_invisible") && this_user()->query_class() < 2) {
					candidates[sz] = nil;
				}
			}

			candidates -= ({ nil });
		}

		if (sizeof(candidates) == 0) {
			return ({ 2, "There is no " + implode(adj + ({ noun }), " ") });
		}

		if (exact || i > 0) {
			if (sizeof(candidates) > 1) {
				return ({ 2, "Be more specific, there is more than one " + implode(adj + ({ noun }), " ") });
			}
		}

		/* todo:  allow multiple matches for the last part */
		if (i > 0) {
			switch(phrase[0]) {
			case "from":
			case "in":
				candidates = candidates[0]->query_inventory();
				break;

			default:
				return ({ 0, "Cannot handle relation: " + phrase[0] });
			}
		}
	}

	if (sizeof(candidates) > 1) {
		return ({ 3, phrases[0][0], candidates });
	} else {
		return ({ 3, phrases[0][0], candidates[0] });
	}
}

private mixed *english_process(string command, object ustate, object actor, object verb, string args)
{
	mixed *parse;
	mapping raw;
	mapping prepkey;
	mapping roles;
	string evoke;
	/* 0 = parse failure */
	/* 1 = map failure */
	/* 2 = bind failure */
	/* 3 = success */

	/* phase 1: parse */
	{
		string statement;

		statement = command + " " + args;

		parse = PARSER_ENGLISH->parse(statement);

		if (!parse) {
			return ({ 0, "Could not parse." });
		}
	}
	/* phase 2: map */

	/* stage 2-1: read roles and build prepkey */
	{
		mixed **rules;
		int i, sz;

		raw = ([ ]);
		prepkey = ([ ]);

		rules = verb->query_roles();

		sz = sizeof(rules);

		if (!sz) {
			rules = ({ ({ "dob", ({ nil }) }) });
			sz = 1;
		}

		for (i = 0; i < sz; i++) {
			mixed *rule;
			string *preps;
			int sz2, j;
			string role;

			rule = rules[i];

			role = rule[0];
			preps = rule[1];
			sz2 = sizeof(preps);

			if (sizeof(rule) > 2 && rule[2]) {
				raw[role] = 1;
			}

			for (j = 0; j < sz2; j++) {
				string prep;

				prep = preps[j];

				if (!prepkey[prep]) {
					prepkey[prep] = ({ });
				}

				prepkey[prep] += ({ rule[0] });
			}
		}
	}

	/* stage 2-2: assign phrases to roles */
	{
		int i, sz;
		string crole;

		roles = ([ ]);

		sz = sizeof(parse);

		for (i = 0; i < sz; i++) {
			mixed *phrase;
			mixed *np;
			string prep;
			string *rcand;
			int j, sz2;

			phrase = parse[i];

			switch(phrase[0]) {
			case "V":
				prep = nil;
				np = phrase[2];
				break;

			case "P":
				prep = phrase[1];
				np = phrase[2];
				break;

			case "E":
				if (evoke) {
					return ({ 1, "Multiple evokes" });
				}

				evoke = phrase[1];
				continue;
			}

			rcand = prepkey[prep];

			if (!rcand) {
				rcand = ({ });
			}

			sz2 = sizeof(rcand);

			for (j = 0; j < sz2; j++) {
				string role;

				role = rcand[j];

				if (!roles[role]) {
					crole = role;
					break;
				}
			}

			if (crole) {
				if (prep || np) {
					if (!roles[crole]) {
						roles[crole] = ({ });
					}

					roles[crole] += ({ ({ prep, np }) });
				}
			} else {
				if (prep) {
					return ({ 1, "No role for preposition \"" + prep + "\"" });
				} else if (np) {
					return ({ 1, "No direct role" });
				} else {
					/* harmless */
				}
			}
		}
	}

	/* phase 3: bind */
	{
		string *rlist;
		object env;
		object *icand;
		int i, sz;

		rlist = map_indices(roles);
		sz = sizeof(rlist);

		if (actor) {
			env = actor->query_environment();

			if (env) {
				icand = actor->query_inventory() + env->query_inventory();
			} else {
				icand = ({ actor }) + actor->query_inventory();
			}
		}

		if (icand) {
			int sz;

			for (sz = sizeof(icand); --sz >= 0; ) {
				if (icand[sz]->query_property("is_invisible") && this_user()->query_class() < 2) {
					icand[sz] = nil;
				}
			}

			icand -= ({ nil });
		}

		for (i = 0; i < sz; i++) {
			string role;

			role = rlist[i];

			if (raw[role]) {
				roles[role] = bind_raw(roles[role]);
			} else {
				mixed *result;

				if (!actor) {
					return ({ 2, "You must be in character to use this command." });
				}

				result = bind_english(roles[role], icand);

				switch(result[0]) {
				case 0 .. 2:
					return result;

				case 3:
					roles[role] = result[1 ..];
				}
			}
		}
	}

	roles["evoke"] = evoke;

	return ({ 3, roles });
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
	CHECKARG(verb, 1, "do_verb");
	CHECKARG(command && command != "", 2, "do_verb");
	CHECKARG(args, 3, "do_verb");

	actor = ustate->query_user()->query_body();

	TLSD->set_tls_value("Text", "ustate", ustate);

	methods = verb->query_parse_methods();

	sz = sizeof(methods);

	for (i = 0; i < sz && !roles; i++) {
		int leave;

		switch(methods[i]) {
		case "raw":
			err = nil;
			roles = ([ "raw" : args, "verb": command ]);
			verb->main(actor, roles);
			break;

		case "english":
			{
				mixed *result;

				result = english_process(command, ustate, actor, verb, args);

				switch(result[0]) {
				case 0: /* parse failure */
					err = result[1];
					continue;

				case 1: /* map failure */
					err = result[1];
					continue;

				case 2: /* bind failure */
					err = result[1];
					roles = ([ ]);
					continue;

				case 3: /* success */
					roles = result[1];
					roles["verb"] = command;
					verb->main(actor, roles);
					continue;

				default:
					break;
				}
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
