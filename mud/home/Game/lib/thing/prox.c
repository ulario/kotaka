/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020, 2022  Raymond Jennings
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
string environment_detail;
object prox;
string prep;

void set_environment_detail(string new_environment_detail)
{
	environment_detail = new_environment_detail;
}

string query_environment_detail()
{
	return environment_detail;
}

void set_prox(object new_prox)
{
	prox = new_prox;
}

object query_prox()
{
	return prox;
}

void set_prep(string new_prep)
{
	prep = new_prep;
}

string query_prep()
{
	return prep;
}

mapping prox_save()
{
	return ([
		"environment_detail": environment_detail,
		"prox": prox,
		"prep": prep
	]);
}

void prox_load(mapping data)
{
	if (data) {
		environment_detail = data["environment_detail"];
		prox = data["prox"];
		prep = data["prep"];
	} else {
		environment_detail = nil;
		prox = nil;
		prep = nil;
	}
}
