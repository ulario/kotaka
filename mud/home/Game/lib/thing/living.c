/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2019, 2021  Raymond Jennings
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

inherit LIB_EMIT;

private object living_lwo;

object *query_inventory();
object query_character_lwo();

object query_living_lwo()
{
	return living_lwo;
}

void initialize_living()
{
	object char_lwo;

	char_lwo = query_character_lwo();

	if (!char_lwo) {
		error("Not a character");
	}

	living_lwo = new_object("~/lwo/living");
	living_lwo->set_hp(char_lwo->query_max_hp());
}

void clear_living()
{
	living_lwo = nil;
}

void recalculate_combat()
{
	int current;
	int total;
	int sz;
	int base;

	object *inv;
	object ch, this;

	if (!living_lwo) {
		return;
	}

	ch = query_character_lwo();
	current = living_lwo->query_attack_bonus();

	inv = query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		object obj;

		obj = inv[sz];

		if (obj->query_property("is_wielded")) {
			total += obj->query_property("attack_value");
		}
	}

	base = ch->query_attack();

	this = this_object();

	if (total > current) {
		emit_to(this, this, "Your attack increased to " + (base + total) + ".");
	} else if (total < current) {
		emit_to(this, this, "Your attack decreased to " + (base + total) + ".");
	}

	living_lwo->set_attack_bonus(total);
}
