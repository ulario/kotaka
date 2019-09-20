/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2019  Raymond Jennings
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
private object character_lwo;

object query_living_lwo();
void initialize_living();
void clear_living();

void initialize_character(int atk, int def, int hp)
{
	if (character_lwo) {
		error("Character already initialized");
	}

	character_lwo = new_object("~/lwo/character");

	character_lwo->set_attack(atk);
	character_lwo->set_defense(def);
	character_lwo->set_max_hp(hp);

	initialize_living();
}

object query_character_lwo()
{
	return character_lwo;
}

void clear_character()
{
	if (query_living_lwo()) {
		clear_living();
	}

	character_lwo = nil;
}
