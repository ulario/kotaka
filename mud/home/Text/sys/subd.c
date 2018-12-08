/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kernel/access.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit "/lib/string/case";
inherit LIB_EMIT;

int query_user_class(string username);

string query_titled_name(string username)
{
	string name;

	int class;

	if (username) {
		name = to_title(username);
	} else {
		name = "guest";
	}

	class = query_user_class(username);

	switch(class) {
	case 0:
		break;

	case 1:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			break;

		case "male":
			name = "Mr. " + name;
			break;

		case "female":
			name = "Ms. " + name;
			break;
		}
		break;

	case 2:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			break;

		case "male":
			name = "Sir " + name;
			break;

		case "female":
			name = "Dame " + name;
			break;

		}
		break;

	case 3:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			break;

		case "male":
			name = "Lord " + name;
			break;

		case "female":
			name = "Lady " + name;
			break;

		}
		break;

	case 4:
		name = "The Ethereal Presence";
		break;

	}

	return name;
}

void send_to_all(string phrase)
{
	int sz;
	object *users;

	users = TEXT_USERD->query_users();
	users += TEXT_USERD->query_guests();

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

void send_to_all_except(string phrase, object *exceptions)
{
	int sz;
	object *users;

	users = TEXT_USERD->query_users();
	users += TEXT_USERD->query_guests();
	users -= exceptions;

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

int query_user_class(string username)
{
	if (!username) {
		return 0;
	}

	if (username == "admin") {
		return 4;
	}

	if (!ACCOUNTD->query_is_registered(username)) {
		return 0;
	}

	if (KERNELD->access(username, "/", FULL_ACCESS)) {
		return 3;
	}

	if (sizeof( KERNELD->query_users() & ({ username }) )) {
		return 2;
	}

	return 1;
}

private string primitive_brief(object thing)
{
	string brief;

	brief = thing->query_property("brief");

	if (brief) {
		return brief;
	}

	brief = thing->query_property("id");

	if (brief) {
		return brief;
	}

	return "nondescript object";
}

string generate_brief_proper(object thing)
{
	return primitive_brief(thing);
}

string generate_brief_definite(object thing)
{
	if (thing->query_property("is_proper")) {
		return generate_brief_proper(thing);
	}

	return "the " + primitive_brief(thing);
}

string generate_brief_indefinite(object thing)
{
	string brief;

	if (thing->query_property("is_proper")) {
		return generate_brief_proper(thing);
	}

	if (thing->query_property("is_definite")) {
		return generate_brief_definite(thing);
	}

	brief = primitive_brief(thing);

	if (strlen(brief) > 0) {
		switch(to_lower(brief)[0]) {
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
			return "an " + brief;
		}
	}

	return "a " + brief;
}

string generate_list(string *phrases)
{
	int sz;

	switch(sizeof(phrases)) {
	case 0:
		return nil;
	case 1:
		return phrases[0];
	case 2:
		return phrases[0] + " and " + phrases[1];
	default:
		sz = sizeof(phrases);
		return implode(phrases[0 .. sz - 2], ", ") + ", and " + phrases[sz - 1];
	}
}

string print_us(int cents)
{
	string centspart;
	int dollars;

	if (cents == 1) {
		return "one cent";
	}

	if (cents < 100) {
		return cents + " cents";
	}

	dollars = cents / 100;
	cents = cents % 100;

	centspart = "0" + cents;
	centspart = centspart[(strlen(centspart) - 2) ..];

	return "$" + dollars + "." + centspart;
}

string print_fantasy(int copper)
{
	int platinum;
	int gold;
	int silver;
	string *stack;

	platinum = copper / 1000;
	copper -= platinum * 1000;

	gold = copper / 100;
	copper -= gold * 100;

	silver = copper / 10;
	copper -= silver * 10;

	stack = ({ });

	if (platinum) {
		stack += ({ platinum + " Pp" });
	}

	if (gold) {
		stack += ({ gold + " Gp" });
	}

	if (silver) {
		stack += ({ silver + " Sp" });
	}

	if (copper) {
		stack += ({ copper + " Cp" });
	}

	return implode(stack, ", ");
}

string build_verb_report(object observer, object actor, string *vforms, object target, string preposition)
{
	string *message;

	if (observer == actor) {
		message = ({ "You", vforms[0] });
	} else {
		message = ({ generate_brief_definite(actor), vforms[1] });
	}

	if (preposition) {
		message += ({ preposition });
	}

	if (target) {
		if (observer == target) {
			if (observer == actor) {
				message += ({ "yourself" });
			} else {
				message += ({ "you" });
			}
		} else {
			if (target == actor) {
				message += ({ "himself" });
			} else {
				message += ({ generate_brief_definite(target) });
			}
		}
	}

	return implode(message, " ");
}

string pinkfish2ansi(string input)
{
	string output;

	output = "";

	while (strlen(input)) {
		string head;
		string fish;
		string tail;

		switch(sscanf(input, "%s%%^%s%%^%s", head, fish, tail)) {
		case 0:
			/* no more pinkfish codes */
		case 1:
			/* incomplete pinkfish code */
			output += input;
			return output;
		case 2:
			tail = "";
		case 3:
			output += head;
			input = tail;

			switch(fish) {
			case "BOLD":
				output += "\033[1m";
				break;
			case "RESET":
				output += "\033[0m";
				break;
			case "RED":
				output += "\033[31m";
				break;
			case "GREEN":
				output += "\033[32m";
				break;
			case "YELLOW":
				output += "\033[33m";
				break;
			case "BLUE":
				output += "\033[34m";
				break;
			case "CYAN":
				output += "\033[35m";
				break;
			case "MAGENTA":
				output += "\033[36m";
				break;
			case "WHITE":
				output += "\033[37m";
				break;
			default:
				LOGD->post_message("system", LOG_NOTICE, "Unknown pinkfish code " + fish);
			}
		}
	}

	return output;
}
