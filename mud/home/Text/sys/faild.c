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
#include <kotaka/paths/system.h>
#include <kotaka/paths/account.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

mapping times; /* ([ IP : ({ strikes, time }) ]) */
mapping strikes; /* ([ IP : strikes ]) */

static void create()
{
	strikes = ([ ]);
	times = ([ ]);
}

static void ban_site(string ip)
{
	LOGD->post_message("system", LOG_NOTICE, "Too many authentication strikes, sitebanning " + ip + " for 90 days");

	BAND->ban_site(ip,
		([
			"expire" : time() + 86400 * 90,
			"message" : "Too many authentication failures",
			"issuer" : "Text"
		])
	);
}

void reset()
{
	ACCESS_CHECK(TEXT() || PRIVILEGED());

	strikes = ([ ]);
}

void strike(string ip)
{
	int time;
	int *strike;

	ACCESS_CHECK(TEXT());

	time = time();

	strike = strikes[ip];

	if (!strike || strike[1] + 24 * 60 + 60 < time) {
		strike = ({ 1, time });
		strikes[ip] = strike;
	} else {
		strike[0]++;
		strike[1] = time;
	}

	LOGD->post_message("system", LOG_NOTICE, "Issuing login strike " + strike[0] + " against " + ip);

	if (strike[0] >= 3) {
		ban_site(ip);
	}
}
