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
#include <kotaka/paths/system.h>
#include <kotaka/log.h>

inherit "~Text/lib/sub";

static void create()
{
	/*
	 * pinkfish: normal text
	 * YELLOW: pinkfish code
	 * GREEN: normal text
	 * OCTARINE: bogus pinkfish code
	 * yellow: normal text
	 * RED: incomplete pinkfish code
	 */
	LOGD->post_message("debug", LOG_DEBUG,
		pinkfish2ansi("pinkfish%^YELLOW%^GREEN%^OCTARINE%^yellow%^RED")
	);
}
