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
#include <text/paths.h>
#include <thing/paths.h>
#include <type.h>

void emit_to(object actor, object viewer, string *verbs, mixed chain ...)
{
	string *buffer;
	mapping seen;
	string phrase;
	object *mobiles;

	int i, sz;

	buffer = ({ });

	seen = ([ ]);

	if (actor == viewer) {
		buffer += ({ "you", verbs[0] });
	} else {
		buffer = ({ TEXT_SUBD->generate_brief_definite(actor), verbs[1] });
	}

	seen[actor] = 1;

	sz = sizeof(chain);

	for (i = 0; i < sz; i++) {
		mixed item;

		item = chain[i];

		switch(typeof(item)) {
		case T_STRING:
			buffer += ({ item });
			break;

		case T_OBJECT:
			if (seen[item]) {
				if (item == actor) {
					buffer += ({ "yourself" });
				} else {
					buffer += ({ "itself" });
				}
			} else {
				if (item == actor) {
					buffer += ({ "you" });
				} else {
					buffer += ({ TEXT_SUBD->generate_brief_definite(item) });
				}
			}
		}
	}

	phrase = implode(buffer, " ") + ".";
	phrase = STRINGD->to_upper(phrase[0 .. 0]) + phrase[1 ..];

	mobiles = viewer->query_property("mobiles");

	sz = sizeof(mobiles -= ({ nil }) );

	for (i = 0; i < sz; i++) {
		mobiles[i]->message(phrase + "\n");
	}
}

void emit_from(object actor, string *verbs, mixed chain ...)
{
	object env;
	object *inv;
	int i, sz;

	env = actor->query_environment();

	if (!env) {
		return;
	}

	inv = env->query_inventory();

	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		emit_to(actor, inv[i], verbs, chain ...);
	}
}
