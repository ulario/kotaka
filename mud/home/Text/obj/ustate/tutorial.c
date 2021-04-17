/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021  Raymond Jennings
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
#include <kotaka/paths/account.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

inherit TEXT_LIB_USTATE;

int pending;

private void do_help()
{
	send_out("To use the tutorial, please choose one of the following topics:\n");
	send_out("characters - How characters work on this MUD\n");
	send_out("communication - How to communicate, both IC and OOC\n");
}

private void do_characters()
{
	send_out("Characters vs Players\n");
	send_out("---------------------\n\n");
	send_out("Unlike on other muds, characters and players are separate concepts on\n");
	send_out("Ulario, as are IC and OOC.\n\n");
	send_out("You are the player as soon as you registered your account.\n");
	send_out("By contrast, your character, or in the future, characters,\n");
	send_out("are separate from you and don't exist at first when you first register.\n\n");
	send_out("To create your character, type \"chargen\".\n");
	send_out("To log in to your character, type \"play\".\n");
}

private void do_communication()
{
	send_out("Communication\n");
	send_out("-------------\n\n");
	send_out("Communicating with other players, or characters, can be done as follows:\n\n");
	send_out("OOC:\n");
	send_out("Players can use channels to talk to other subscribed users.\n");
	send_out("chlist - List available channels");
	send_out("chon - Turn a channel on");
	send_out("choff - Turn a channel off");
	send_out("chpost <channel name> <message> - Post to a channel.\n\n");
	send_out("The \"chat\" channel is for player use.\n\n");
}

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	send_out("Welcome to the Ulario MUD tutorial.\n\n");
	send_out("Available tutorial topics:\n\n");
	send_out("characters, communication, help.\n");

	prompt();
}

void end()
{
	destruct_object(this_object());
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());
}
