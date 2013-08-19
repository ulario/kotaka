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
#include <kotaka/paths/verb.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	object body;
	object user;

	user = query_user();
	body = user->query_body();

	if (body) {
		object *mobiles;
		ASSERT(actor == body);

		mobiles = body->query_property("mobiles");
		body->set_property("mobiles", mobiles - ({ nil, user }));
		user->set_body(nil);
		send_out("You leave " + TEXT_SUBD->generate_brief_definite(body) + ".\n");
	} else {
		query_user()->quit();
	}
}
