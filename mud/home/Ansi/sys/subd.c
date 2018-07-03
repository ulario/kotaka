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

private int target_bits(int chr)
{
	switch (chr) {
	case 'k': return 0;
	case 'r': return 1;
	case 'g': return 2;
	case 'y': return 3;
	case 'b': return 4;
	case 'm': return 5;
	case 'c': return 6;
	case 'w': return 7;
	case 'K': return 8;
	case 'R': return 9;
	case 'G': return 10;
	case 'Y': return 11;
	case 'B': return 12;
	case 'M': return 13;
	case 'C': return 14;
	case 'W': return 15;
	default: return -1;
	}
}

/* Simple compiler for ansi-escaped ascii graphics */

/* Colors: */
/* K = black, R = red, G = green, Y = yellow */
/* B = blue, M = magenta, C = cyan, W = white */

/* case determines intensity and blink */
string simple_ansify(string chars, varargs string fore, string back)
{
	string buf;
	int i;
	int sz;
	int forecolor;
	int backcolor;
	buf = "";

	/* 1 = blue bit */
	/* 2 = green bit */
	/* 4 = red bit */
	/* 8 = intense/blink bit */
	forecolor = 7;
	backcolor = 0;

	sz = strlen(chars);

	for (i = 0; i < sz; i++) {
		int target_bits;
		int forecolor_change;
		int backcolor_change;
		int bold_change;
		int blink_change;
		string *codes;
		string escape;

		codes = ({ });

		target_bits = -1;

		if (fore) {
			target_bits = target_bits(fore[i]);
		}

		if (target_bits == -1) {
			target_bits = 7;
		}

		if ((target_bits & 7) != (forecolor & 7)) {
			forecolor_change = 1;
		}
		if ((target_bits & 8) != (forecolor & 8)) {
			bold_change = 1;
		}

		forecolor = target_bits;

		target_bits = -1;

		if (back) {
			target_bits = target_bits(back[i]);
		}

		if (target_bits == -1) {
			target_bits = 0;
		}

		if ((target_bits & 7) != (backcolor & 7)) {
			backcolor_change = 1;
		}
		if ((target_bits & 8) != (backcolor & 8)) {
			blink_change = 1;
		}

		backcolor = target_bits;

		if (forecolor == 7 && backcolor == 0 &&
			(bold_change
				|| blink_change
				|| forecolor_change
				|| backcolor_change
			)
		) {
			codes = ({ "0" });
			bold_change = 0;
			blink_change = 0;
			forecolor_change = 0;
			backcolor_change = 0;
		}

		if (bold_change) {
			if (forecolor & 8) {
				codes += ({ "1" });
			} else {
				codes += ({ "22" });
			}

			bold_change = 0;
		}

		if (forecolor_change) {
			codes += ({ "3" + (forecolor & 7) });

			forecolor_change = 0;
		}

		if (blink_change) {
			if (backcolor & 8) {
				codes += ({ "5" });
			} else {
				codes += ({ "25" });
			}

			blink_change = 0;
		}

		if (backcolor_change) {
			codes += ({ "4" + (backcolor & 7) });

			backcolor_change = 0;
		}

		if (sizeof(codes)) {
			escape = "\033[" + implode(codes, ";") + "m";
			buf += escape;
		}

		buf += chars[i .. i];
	}

	return buf;
}
