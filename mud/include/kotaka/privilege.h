/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2003, 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <config.h>
#include <kernel/kernel.h>
#include <kernel/user.h>

#define ANSI()		(sscanf(previous_program(), USR_DIR + "/Ansi/%*s"))
#define ACCOUNT()	(sscanf(previous_program(), USR_DIR + "/Account/%*s"))
#define BIGSTRUCT()	(sscanf(previous_program(), USR_DIR + "/Bigstruct/%*s"))
#define CATALOG()	(sscanf(previous_program(), USR_DIR + "/Catalog/%*s"))
#define CHANNEL()	(sscanf(previous_program(), USR_DIR + "/Channel/%*s"))
#define GAME()		(sscanf(previous_program(), USR_DIR + "/Game/%*s"))
#define GEOMETRY()	(sscanf(previous_program(), USR_DIR + "/Geometry/%*s"))
#define HELP()		(sscanf(previous_program(), USR_DIR + "/Help/%*s"))
#define HTTP()		(sscanf(previous_program(), USR_DIR + "/Http/%*s"))
#define INTERMUD()	(sscanf(previous_program(), USR_DIR + "/Intermud/%*s"))
#define KOTAKA()	(sscanf(previous_program(), USR_DIR + "/Kotaka/%*s"))
#define TEST()		(sscanf(previous_program(), USR_DIR + "/Test/%*s"))
#define TEXT()		(sscanf(previous_program(), USR_DIR + "/Text/%*s"))
#define THING()		(sscanf(previous_program(), USR_DIR + "/Thing/%*s"))
#define VERB()		(sscanf(previous_program(), USR_DIR + "/Verb/%*s"))

#define KADMIN()	(previous_program(1) == LIB_WIZTOOL && \
			previous_program() == USR_DIR + "/admin/_code")
#define CODE()		(previous_program(1) == LIB_WIZTOOL && \
			sscanf(previous_program(), USR_DIR + "/%*s/_code"))
#define LOCAL()		(calling_object() == this_object())

#define INTERFACE()	(TEXT() || SYSTEM() || GAME() || INTERMUD() || VERB() || HTTP())
#define PRIVILEGED()	(KOTAKA() || GAME() || SYSTEM() || KADMIN() || TEST())
#define ACCESS_CHECK(x)	do { if (!(x)) error("Access denied"); } while (0)
#define PERMISSION_CHECK(x)	do { if (!(x)) error("Permission denied"); } while (0)
