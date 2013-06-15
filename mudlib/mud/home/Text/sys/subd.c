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
#include <kernel/access.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/bigstruct.h>

#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

string titled_name(string name, int class)
{
	string username;

	username = name;

	if (name) {
		name = STRINGD->to_title(name);
	} else {
		name = "guest";
	}

	switch(class) {
	case 0:
		name = "\033[1;34m" + name + "\033[0m";
		break;
	case 1:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			name = "\033[1;32m" + name + "\033[0m";
			break;
		case "male":
			name = "\033[1;32mMr. " + name + "\033[0m";
			break;
		case "female":
			name = "\033[1;32mMs. " + name + "\033[0m";
			break;
		}
		break;
	case 2:
		switch(ACCOUNTD->query_account_property(username, "gender")) {
		case nil:
			name = "\033[1;33m" + name + "\033[0m";
			break;
		case "male":
			name = "\033[1;33mSir " + name + "\033[0m";
			break;
		case "female":
			name = "\033[1;33mDame " + name + "\033[0m";
			break;
		}
		break;
	case 3:
		if (name == "Admin") {
			name = "\033[1;37mThe Ethereal Presence\033[m";
		} else {
			switch(ACCOUNTD->query_account_property(username, "gender")) {
			case nil:
				name = "\033[1;31m" + name + "\033[0m";
				break;
			case "male":
				name = "\033[1;31mLord " + name + "\033[0m";
				break;
			case "female":
				name = "\033[1;31mLady " + name + "\033[0m";
				break;
			}
		}
		break;
	}

	return name;
}

void send_to_all(string phrase)
{
	int sz;
	object *users;

	ACCESS_CHECK(TEXT() || GAME());

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

	ACCESS_CHECK(TEXT() || GAME());

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

string generate_brief_definite(object thing)
{
	string brief;

	brief = thing->query_property("brief");

	if (thing->query_property("is_definite")) {
		return brief;
	}

	if (brief) {
		return "the " + brief;
	}

	brief = thing->query_property("id");

	if (brief) {
		return "the " + brief;
	}

	return "the nondescript thing";
}

string generate_brief_indefinite(object thing)
{
	string brief;

	brief = thing->query_property("brief");

	if (brief) {
		if (thing->query_property("is_definite")) {
			return brief;
		}

		if (strlen(brief) > 0) {
			switch(STRINGD->to_lower(brief)[0]) {
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

	brief = thing->query_property("id");

	if (brief) {
		return "a " + brief;
	}

	return "a nondescript thing";
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

string print_dollars(int cents)
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
