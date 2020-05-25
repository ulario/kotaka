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

inherit LIB_EMIT;
inherit LIB_VERB;
inherit "~/lib/ic";

private string describe(object thing)
{
	string brief;
	string id;
	string prox;
	string buffer;

	if (thing->has_detail(nil)) {
		brief = thing->query_description(nil, "brief");
	}

	if (!brief) {
		brief = thing->query_property("brief");
	}

	buffer = "";

	if (brief) {
		buffer += brief + " (";
	}

	if (id = thing->query_object_name()) {
		buffer += id;
	} else if (id = thing->query_id()) {
		buffer += id;
	} else {
		buffer += object_name(thing);
	}

	if (brief) {
		buffer += ")";
	}

	if (prox = thing->query_prox()) {
		string prep;

		prep = thing->query_prep();

		if (prep) {
			buffer += ": " + prep + " " + prox;
		} else {
			buffer += ": " + prox;
		}
	}

	return buffer;
}

private void debug_inventory(object actor)
{
	object *inv;
	int sz;

	send_out("Debug inventory:\n");

	inv = actor->query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		send_out(describe(inv[sz]) + "\n");
	}
}

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
}

string *query_help_contents()
{
}

private string print_values(mapping values)
{
	string buf;
	string *bits;
	string *c;
	int *v;
	int sz, i;

	c = map_indices(values);
	v = map_values(values);
	sz = sizeof(c);

	buf = "Value held: ";
	bits = ({ });

	for (i = 0; i < sz; i++) {
		switch(c[i]) {
		case "us":
			bits += ({ TEXT_SUBD->print_us(v[i]) });
			break;

		case "fantasy":
			bits += ({ TEXT_SUBD->print_fantasy(v[i]) });
			break;
		}
	}

	return buf + implode(bits, ", ");
}

void main(object actor, mapping roles)
{
	object *inv;
	int sz;

	if (roles["raw"] == "-d") {
		debug_inventory(actor);
		return;
	}

	inv = actor->query_inventory();

	{
		int sz;

		for (sz = sizeof(inv); --sz >= 0; ) {
			if (inv[sz]->query_property("is_invisible") && this_user()->query_class() < 2) {
				inv[sz] = nil;
			}
		}

		inv -= ({ nil });
	}

	sz = sizeof(inv);

	if (sz) {
		int i;
		int value;
		string *sections;
		string section;
		string *pieces;
		string currency;
		mapping values;

		sections = ({ "Your inventory:" });
		pieces = ({ });
		values = ([ ]);

		for (i = 0; i < sz; i++) {
			pieces += ({ TEXT_SUBD->generate_brief_indefinite(inv[i]) });

			if (inv[i]->query_property("value")) {
				value = inv[i]->query_property("value");

				if (currency = inv[i]->query_property("currency")) {
					if (!values[currency]) {
						values[currency] = value;
					} else {
						values[currency] += value;
					}
				}
			}
		}

		section = implode(pieces, "\n");
		sections += ({ section });

		if (map_sizeof(values)) {
			sections += ({ print_values(values) });
		}

		send_out(implode(sections, "\n\n") + "\n\n");
	} else {
		send_out("You have nothing.\n");
	}
}
