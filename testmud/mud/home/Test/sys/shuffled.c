/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/bigstruct.h>
#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <status.h>

object rooms;
object mobs;
int handle;
float power;
float delay;

int room_goal;
int mob_goal;

static void create()
{
	rooms = clone_object(BIGSTRUCT_ARRAY_OBJ);
	mobs = clone_object(BIGSTRUCT_ARRAY_OBJ);

	power = 1.0;

	room_goal = 1000;
	mob_goal = 500;

	delay = 0.001 * (float)room_goal;

	handle = call_out("process", 0);
}

static void destruct()
{
	destruct_object(rooms);
	destruct_object(mobs);
}

void set_room_goal(int new_count)
{
	ACCESS_CHECK(TEXT());

	if (mob_goal > 0 && new_count == 0) {
		error("Cannot have mobs without rooms");
	}

	room_goal = new_count;

	if (!handle) {
		handle = call_out("process", 0);
	}
}

void set_mob_goal(int new_count)
{
	ACCESS_CHECK(TEXT());

	mob_goal = new_count;

	if (!handle) {
		handle = call_out("process", 0);
	}
}

void set_power(float new_power)
{
	ACCESS_CHECK(TEXT());

	if (new_power <= 0.0) {
		error("Invalid power");
	}

	power = new_power;
}

void set_jump_delay(float new_delay)
{
	ACCESS_CHECK(TEXT());

	if (new_delay <= 0.0) {
		error("Invalid delay");
	}

	delay = new_delay;
}

int query_room_count()
{
	return rooms->get_size();
}

int query_mob_count()
{
	return mobs->get_size();
}

int query_room_goal()
{
	return room_goal;
}

int query_mob_goal()
{
	return mob_goal;
}

static void process()
{
	handle = 0;

	if (mobs->get_size() > mob_goal) {
		object turkey;

		turkey = mobs->get_back();
		mobs->pop_back();

		destruct_object(turkey);
	} else if (rooms->get_size() < room_goal) {
		rooms->push_back(clone_object("~/obj/shuffleroom"));
	} else if (mobs->get_size() < mob_goal) {
		mobs->push_back(clone_object("~/obj/shufflemob"));
	} else if (rooms->get_size() > room_goal) {
		object turkey;

		turkey = rooms->get_back();
		rooms->pop_back();

		destruct_object(turkey);
	}
	if (rooms->get_size() != room_goal || mobs->get_size() != mob_goal) {
		handle = call_out("process", 0);
	}
}

object pick_room()
{
	float room;

	room = pow(SUBD->rnd(), power) * (float)rooms->get_size();

	return rooms->get_element((int)floor(room));
}

float query_jump_delay()
{
	return (SUBD->rnd() + SUBD->rnd()) * delay;
}
