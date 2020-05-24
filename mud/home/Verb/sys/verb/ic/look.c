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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <type.h>
#include <kotaka/assert.h>

inherit "/lib/string/format";
inherit LIB_EMIT;
inherit LIB_VERB;

private string inventory_list(object *inv)
{
	string *desc;
	int i, sz;

	desc = ({ });
	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		desc += ({ TEXT_SUBD->generate_brief_indefinite(inv[i]) });
	}

	return TEXT_SUBD->generate_list(desc);
}

private string contents(object env, object viewer)
{
	string prox;
	object *inv;
	int sz;

	prox = viewer->query_prox();

	inv = env->query_inventory() - ({ viewer });

	for (sz = sizeof(inv); --sz >= 0; ) {
		if (inv[sz]->query_property("is_invisible") && this_user()->query_class() < 2) {
			inv[sz] = nil;
			continue;
		}

		if (inv[sz]->query_prox() != prox) {
			inv[sz] = nil;
			continue;
		}
	}

	inv -= ({ nil });

	if (sizeof(inv)) {
		return wordwrap("You see " + inventory_list(inv) + ".", 55) + "\n";
	} else {
		return "";
	}
}

private string brief_of(object env, string prox)
{
	string brief;

	if (env->has_detail(prox)) {
		brief = env->query_description(prox, "brief");
	}

	if (!brief) {
		if (!prox) {
			brief = env->query_property("brief");
		}
	}

	if (!brief) {
		brief = (prox ? prox + " of " : "") + env->query_object_name();
	}

	if (!brief) {
		brief = (prox ? prox + " of " : "") + env->query_id();
	}

	if (!brief) {
		brief = "no brief";
	}

	return brief;
}

string *query_parse_methods()
{
	return ({ "english" });
}

/* ({ role, prepositions, raw }) */
mixed **query_roles()
{
	return ({
		({ "dob", ({ nil, "at" }) })
	});
}

string query_help_title()
{
}

string *query_help_contents()
{
}

void main(object actor, mapping roles)
{
	mixed dob;
	mixed obj;
	string prep;
	string look;

	dob = roles["dob"];

	if (!dob) {
		string prox;

		if (actor && (prox = actor->query_prox())) {
			if (actor->query_prep() == "in") {
				object env;

				env = actor->query_environment();

				ASSERT(env);

				send_out(brief_of(env, prox) + "\n");

				if (env->has_detail(prox)) {
					send_out(wordwrap(env->query_description(prox, "look"), 60) + "\n");
					send_out(contents(env, actor) + "\n");
					return;
				} else {
					send_out("...that doesn't actually exist...\n");
				}
			}
		}

		/* it's ok to look around while dead, RenderD checks for this itself */
		send_out(RENDERD->look(actor));
		return;
	}

	prep = dob[0];
	obj = dob[1];

	switch(prep) {
	case nil:
	case "at":
		if (typeof(obj) == T_OBJECT) {
			emit_from(actor, actor, " ", ({ "look", "looks" }), " at ", obj, ".");
			look = obj->query_property("look");
			send_out(look ? wordwrap(look, 60) : "a bland object");
			send_out("\n");
		} else {
			int sz, i;
			string *briefs;

			emit_from(actor, actor, " ", ({ "look", "looks" }), " at a bunch of stuff.");

			sz = sizeof(obj);
			briefs = ({ });

			for (i = 0; i < sz; i++) {
				briefs += ({ TEXT_SUBD->generate_brief_indefinite(obj[i]) });
			}

			briefs[sz - 1] = "and " + briefs[sz - 1];

			send_out(implode(briefs, ", ") + "\n");
		}

		break;

	case "in":
		if (typeof(obj) == T_OBJECT) {
		} else {
			send_out("You have to be more specific to look inside something.\n");
			return;
		}

		emit_from(actor, actor, " ", ({ "look", "looks" }), " in ", obj, ".");
		{
			object *inv;
			int sz;

			inv = obj->query_inventory();
			sz = sizeof(inv);

			if (sz) {
				int i;

				send_out("Contents:\n\n");

				for (i = 0; i < sz; i++) {
					send_out(TEXT_SUBD->generate_brief_indefinite(inv[i]) + "\n");
				}

				send_out("\n");
			} else {
				send_out("There is nothing inside.\n");
			}
		}
		break;

	default:
		send_out("todo: handle preposition " + prep + "\n");
	}
}
