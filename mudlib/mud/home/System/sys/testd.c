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
#include <kernel/tls.h>

#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

#include <kotaka/bigstruct.h>

#include <type.h>
#include <status.h>

inherit SECOND_AUTO;

private inherit tls API_TLS;

static void create()
{
	tls::create();
}

static void tls_test(varargs mixed args...)
{
	set_tlvar(0, "foobar");

	ASSERT(get_tlvar(0) == "foobar");

	LOGD->post_message("test", LOG_DEBUG, "TLS on callout test passed");
}

void test()
{
	string err;

	ACCESS_CHECK(SYSTEM());
}
