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
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <text/paths.h>
#include <game/paths.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	object thing;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	send_out("You conjure up a thing.\n");

	thing = GAME_INITD->create_object();
	thing->move(actor);
	thing->set_property("id", "thing");
	thing->set_property("local_nouns", ({ "thing" }) );
	thing->set_property("local_adjectives", ({ "thingy" }) );
}
