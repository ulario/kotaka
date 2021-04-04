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
