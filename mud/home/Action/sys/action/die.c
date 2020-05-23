/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/paths/action.h>
#include <kotaka/log.h>

inherit "/lib/string/case";

inherit LIB_USERIO;
inherit LIB_EMIT;
inherit LIB_ACTION;

void action(mapping roles)
{
	object body;
	object ghost;
	string name;

	body = roles["actor"];

	if (!body->query_character_lwo()) {
		send_out("Wait what?  You're not made of meat.\n");
		return;
	}

	LOGD->post_message("debug", LOG_DEBUG, object_name(body) + " dies");

	emit_from(body, body, " ", ({ "die", "dies" }), ".");

	name = body->query_object_name();

	body->clear_living();

	if (name && sscanf(name, "players:%s", name)) {
		body->set_local_property("brief", to_title(name) + "'s corpse");
	}

	ghost = body->query_possessor();

	if (ghost) {
		ghost->possess(nil);

		if (ghost->query_environment() == body) {
			ghost->move(body->query_environment());
			emit_from(ghost, ghost, " ", ({ "emerge", "emerges" }), " from ", body, ".");
		}
	}
}
