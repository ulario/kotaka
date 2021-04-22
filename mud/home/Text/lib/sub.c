/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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
#include <kotaka/checkarg.h>

inherit "/lib/string/case";
inherit "emit";
inherit "class";
inherit "generate";

string query_titled_name(string username)
{
	string name;
	int class;

	CHECKARG(username, 1, "query_titled_name");

	name = to_title(username);
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
	string *slices;
	int sz, i;

	slices = explode("%^" + input + "%^", "%^");
	sz = sizeof(slices);

	if (sz == 1) {
		return input;
	}

	for (i = 1; i < sz - 1; i++) {
		string fish;

		fish = slices[i];

		switch(fish) {
		case "BOLD":
			fish = "\033[1m";
			break;
		case "RESET":
			fish = "\033[0m";
			break;
		case "RED":
			fish = "\033[31m";
			break;
		case "GREEN":
			fish = "\033[32m";
			break;
		case "ORANGE":
		case "YELLOW":
			fish = "\033[33m";
			break;
		case "BLUE":
			fish = "\033[34m";
			break;
		case "CYAN":
			fish = "\033[35m";
			break;
		case "MAGENTA":
			fish = "\033[36m";
			break;
		case "B_RED":
			fish = "\033[1;31m";
			break;
		case "B_GREEN":
			fish = "\033[1;32m";
			break;
		case "B_YELLOW":
			fish = "\033[1;33m";
			break;
		case "B_BLUE":
			fish = "\033[1;34m";
			break;
		case "B_CYAN":
			fish = "\033[1;35m";
			break;
		case "B_MAGENTA":
			fish = "\033[1;36m";
			break;
		case "WHITE":
			fish = "\033[37m";
			break;
		default:
			/* if all caps, it's probably a pinkfish code */
			if (fish == to_upper(fish)) {
				LOGD->post_message("system", LOG_NOTICE, "Potential untranslated pinkfish code " + fish);
			}
		}

		slices[i] = fish;
	}

	return implode(slices, "");
}
