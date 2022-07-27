/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2019, 2022  Raymond Jennings
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
private int attack;
private int defense;

private int max_hp;

int query_attack()
{
	return attack;
}

void set_attack(int new_attack)
{
	attack = new_attack;
}

int query_defense()
{
	return defense;
}

void set_defense(int new_defense)
{
	defense = new_defense;
}

int query_max_hp()
{
	return max_hp;
}

void set_max_hp(int new_max_hp)
{
	max_hp = new_max_hp;
}
