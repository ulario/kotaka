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
#include <kotaka/paths/account.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>
#include <type.h>

inherit "/lib/string/sprint";
inherit "/lib/string/case";

private string from_object(object obj, mapping seen, object witness)
{
	if (seen[obj]) {
		if (obj == witness) {
			return "yourself";
		} else {
			return "itself";
		}
	} else {
		if (obj == witness) {
			return "you";
		} else {
			return TEXT_SUBD->generate_brief_definite(obj);
		}
	}
}

void emit_to(object witness, object actor, mixed chain ...)
{
	string buffer;
	mapping seen;
	string phrase;
	object *mobiles;
	object possessor;

	int i, sz;

	buffer = "";

	seen = ([ ]);

	sz = sizeof(chain);

	for (i = 0; i < sz; i++) {
		mixed item;

		item = chain[i];

		switch(typeof(item)) {
		case T_STRING:
			buffer += item;
			break;

		case T_OBJECT:
			buffer += from_object(item, seen, witness);
			seen[item] = 1;
			break;

		case T_ARRAY:
			switch(typeof(item[0])) {
			case T_STRING:
				/* verb */
				{
					if (actor == witness) {
						buffer += item[0];
					} else {
						buffer += item[1];
					}
				}
				break;

			case T_OBJECT:
				/* list of objects */
				{
					int j, sz2;
					string *list;

					sz2 = sizeof(item);
					list = ({ });

					for (j = 0; j < sz2; j++) {
						list += ({ from_object(item[j], seen, witness) });
					}

					list[sz2 - 1] = "and " + list[sz2 - 1];

					buffer += implode(list, ", ");
				}
			}
			break;
		}
	}

	if (strlen(buffer) > 1) {
		buffer = to_upper(buffer[0 .. 0]) + buffer[1 ..];
	} else {
		buffer = to_upper(buffer);
	}

	while (possessor = witness->query_possessor()) {
		witness = possessor;
	}

	mobiles = witness->query_property("mobiles");

	sz = sizeof(mobiles -= ({ nil }) );

	for (i = 0; i < sz; i++) {
		mobiles[i]->message(buffer + "\n");
	}
}

void emit_from(object actor, mixed chain ...)
{
	object *cand;
	int sz;

	object env;

	cand = ({ actor });

	env = actor->query_environment();

	if (env) {
		cand |= ({ env });
		cand |= env->query_inventory();
	}

	cand |= actor->query_inventory();

	LOGD->post_message("debug", LOG_DEBUG, "Witnesses: " + mixed_sprint(cand));

	for (sz = sizeof(cand); --sz >= 0; ) {
		emit_to(cand[sz], actor, chain ...);
	}
}

void ooc_emit_to(string sender, string receiver, string message)
{
	int sinvis;
	int sclass;
	int rclass;

	sclass = TEXT_SUBD->query_user_class(sender);
	rclass = TEXT_SUBD->query_user_class(receiver);

	if (sender) {
		sinvis = !!ACCOUNTD->query_account_property(sender, "invisible");
	}

	if (sinvis && sclass >= rclass) {
		return;
	}

	TEXT_USERD->find_user(receiver)->message(message);
}
