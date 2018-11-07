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
inherit "char";

string to_lower(string text)
{
	int ctr;
	int len;
	string newword;

	newword = text;
	len = strlen(newword);

	for (ctr = 0; ctr < len; ctr++) {
		newword[ctr] = char_to_lower(newword[ctr]);
	}

	return newword;
}

string to_upper(string text)
{
	int ctr;
	int len;
	string newword;

	newword = text;
	len = strlen(newword);

	for (ctr = 0; ctr < len; ctr++) {
		newword[ctr] = char_to_upper(newword[ctr]);
	}

	return newword;
}

string to_title(string text)
{
	string *words;
	int index;

	words = explode(text, " ") - ({ "" });

	for (index = 0; index < sizeof(words); index++) {
		string word;

		word = to_lower(words[index]);

		if (index == 0 || index == (sizeof(words) - 1)) {
			word[0] = char_to_upper(word[0]);
		} else {
			switch(word) {
			case "and":
			case "auf":
			case "de":
			case "of":
			case "or":
			case "the":
				break;
			default:
				word[0] = char_to_upper(word[0]);
			}
		}

		words[index] = word;
	}

	return implode(words, " ");
}
