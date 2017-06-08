/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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

private string wordwrap_line(string line, int width)
{
	string *words;
	string buffer;
	int tail;
	int length;
	int i;
	int sz;

	buffer = "";
	words = explode(line, " ") - ({ nil, "" });
	sz = sizeof(words);

	for (i = 0; i < sz; i++) {
		string word;
		int wlen;

		word = words[i];
		wlen = strlen(word);

		if (length + tail + wlen > width) {
			length = 0;
			buffer += "\n";
		} else {
			buffer += spaces(tail);
			length += tail;
		}

		buffer += word;
		length += wlen;

		switch(word[wlen - 1]) {
		case '.':
		case '?':
		case '!':
			tail = 2;
			break;
		default:
			tail = 1;
		}
	}

	return buffer;
}

string wordwrap(string text, int width)
{
	string *lines;
	int i, sz;

	lines = explode(text, "\n\n");
	sz = sizeof(lines);

	for (i = 0; i < sz; i++) {
		lines[i] = wordwrap_line(lines[i], width);
	}

	return implode(lines, "\n\n");
}

string render_table(string **table, int colgap)
{
	int irow;
	int icol;
	int cols;

	int tsz;

	string *rows;

	int *widths;
	widths = ({ });
	tsz = sizeof(table);
	rows = allocate(tsz);

	for (irow = 0; irow < tsz; irow++) {
		string *row;
		int rsz;

		row = table[irow];

		if (!row) {
			continue;
		}

		rsz = sizeof(row);

		if (cols < rsz) {
			cols = rsz;
		}

		while (sizeof(widths) < cols) {
			widths += ({ 0 });
		}

		for (icol = 0; icol < rsz; icol++) {
			int sz;

			sz = strlen(row[icol]);

			if (widths[icol] < sz) {
				widths[icol] = sz;
			}
		}
	}

	for (irow = 0; irow < tsz; irow++) {
		string *row;
		int rsz;

		row = table[irow];

		if (!row) {
			row = ({ });
		}

		rsz = sizeof(row);

		for (icol = 0; icol < rsz; icol++) {
			string cell;
			string piece;
			string spaces;

			piece = row[icol];
			spaces = spaces(widths[icol] - strlen(piece));

			cell = piece + spaces;

			row[icol] = cell;
		}

		rows[irow] = implode(row, spaces(colgap));
	}

	return implode(rows, "\n") + "\n";
}
