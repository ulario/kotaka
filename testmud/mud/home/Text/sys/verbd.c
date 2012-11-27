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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

object find_verb(string command)
{
	object verb;

	if (verb = find_object("~/sys/verb/ooc/adm/" + command)) {
		return verb;
	}
	if (verb = find_object("~/sys/verb/ooc/wiz/debug/" + command)) {
		return verb;
	}
	if (verb = find_object("~/sys/verb/ooc/wiz/" + command)) {
		return verb;
	}
	if (verb = find_object("~/sys/verb/ooc/test/" + command)) {
		return verb;
	}
	if (verb = find_object("~/sys/verb/ooc/movie/" + command)) {
		return verb;
	}
	if (verb = find_object("~/sys/verb/ic/direction/" + command)) {
		return verb;
	}
	if (verb = find_object("~/sys/verb/ooc/" + command)) {
		return verb;
	}
	if (verb = find_object("~/sys/verb/ic/" + command)) {
		return verb;
	}
	if (verb = find_object("~/sys/verb/" + command)) {
		return verb;
	}
}

int do_action(object actor, string command, string args)
{
	object ustate;
	object verb;

	ACCESS_CHECK((ustate = previous_object())<-LIB_USTATE);

	verb = find_verb(command);

	if (!verb) {
		return FALSE;
	}

	TLSD->set_tls_value("Text", "ustate", ustate);

	verb->main(actor, args);

	TLSD->set_tls_value("Text", "ustate", nil);

	return TRUE;
}
