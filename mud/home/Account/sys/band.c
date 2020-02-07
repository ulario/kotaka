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
#include <kotaka/log.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <type.h>

inherit "/lib/string/case";
inherit "/lib/string/sprint";

void save();
void restore();
void prune_bans();
void prune_sitebans();

mapping bans;
mapping sitebans;

private string cidr_mask(string site, int cidr)
{
	mixed o1, o2, o3, o4;

	if (sscanf(site, "%d.%d.%d.%d", o1, o2, o3, o4) < 4) {
		error("Malformed IP address");
	}

	if (o1 != o1 & 255) {
		error("Malformed IP address");
	}
	if (o2 != o2 & 255) {
		error("Malformed IP address");
	}
	if (o3 != o3 & 255) {
		error("Malformed IP address");
	}
	if (o4 != o4 & 255) {
		error("Malformed IP address");
	}

	switch(cidr) {
	case 0 .. 8:
		o1 &= ~((1 << (8 - cidr)) - 1);
		o2 = "0";
		o3 = "0";
		o4 = "0";
		break;

	case 9 .. 16:
		o2 &= ~((1 << (16 - cidr)) - 1);
		o3 = "0";
		o4 = "0";
		break;

	case 17 .. 24:
		o3 &= ~((1 << (24 - cidr)) - 1);
		o4 = "0";
		break;

	case 25 .. 32:
		o4 &= ~((1 << (32 - cidr)) - 1);
		break;

	default:
		error("Invalid CIDR");
	}

	return o1 + "." + o2 + "." + o3 + "." + o4;
}

private string canonicalize_mask(string mask)
{
	mixed o1, o2, o3, o4;
	int cidr;

	switch (sscanf(mask, "%d.%d.%d.%d/%d", o1, o2, o3, o4, cidr)) {
	case 5:
		break;

	default:
		switch(sscanf(mask, "%s.%s.%s.%s", o1, o2, o3, o4)) {
		case 4:
			cidr = 32;

			if (o1 == "*") {
				error("Malformed mask");
			} else if (o2 == "*") {
				cidr = 8;

				if (o3 != "*") {
					error("Malformed mask");
				}

				if (o4 != "*") {
					error("Malformed mask");
				}

				o1 = (int)o1;
				o2 = "0";
				o3 = "0";
				o4 = "0";
			} else if (o3 == "*") {
				cidr = 16;

				if (o4 != "*") {
					error("Malformed mask");
				}

				o1 = (int)o1;
				o2 = (int)o2;
				o3 = "0";
				o4 = "0";
			} else if (o4 == "*") {
				cidr = 24;

				o1 = (int)o1;
				o2 = (int)o2;
				o3 = (int)o3;
				o4 = "0";
			} else {
				cidr = 32;
			}
			break;

		default:
			error("Malformed mask");
		}
	}

	return cidr_mask(o1 + "." + o2 + "." + o3 + "." + o4, cidr) + "/" + cidr;
}

private void check_property(string name, mixed value)
{
	switch(name) {
	case "message":
	case "issuer":
	case "note":
		switch(typeof(value)) {
		case T_NIL:
		case T_STRING:
			break;
		default:
			error("Invalid type for " + name);
		}
		break;

	case "expire":
		if (typeof(value) != T_INT) {
			error("Invalid type for expire");
		}
		break;

	default:
		error("Unknown property " + name);
	}
}

static void create()
{
	restore();

	if (!bans) {
		bans = ([ ]);
	}

	if (!sitebans) {
		sitebans = ([ ]);
	}
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	call_out("save", 0);
}

void save()
{
	string buf;

	ACCESS_CHECK(KERNEL() || ACCOUNT() || GAME() || INTERFACE() || KADMIN() || VERB());

	prune_bans();
	prune_sitebans();

	buf = hybrid_sprint( ([
		"bans": map_sizeof(bans) ? bans : nil,
		"sitebans": map_sizeof(sitebans) ? sitebans : nil
	]) );

	SECRETD->make_dir(".");
	SECRETD->remove_file("bans-tmp");
	SECRETD->write_file("bans-tmp", buf + "\n");
	SECRETD->remove_file("bans");
	SECRETD->rename_file("bans-tmp", "bans");
}

void restore()
{
	string buf;

	ACCESS_CHECK(ACCOUNT() || GAME() || INTERFACE() || KADMIN() || VERB());

	buf = SECRETD->read_file("bans");

	if (!buf) {
		buf = CONFIGD->read_file("bans");
	}

	if (buf) {
		mapping save;

		save = PARSER_VALUE->parse(buf);

		bans = save["bans"];
		sitebans = save["sitebans"];
	}

	if (!bans) {
		bans = ([ ]);
	}

	if (!sitebans) {
		sitebans = ([ ]);
	}

	prune_bans();
	prune_sitebans();
}

void prune_bans()
{
	string *keys;
	int sz;

	keys = map_indices(bans);

	for (sz = sizeof(keys); --sz >= 0; ) {
		mixed expire;
		string key;
		mixed ban;

		key = keys[sz];

		ban = bans[key];
		bans[key] = nil;
		key = to_lower(key);

		bans[key] = ban;

		expire = ban["expire"];

		if (expire != nil && expire != -1) {
			if (time() >= expire) {
				bans[key] = nil;
			}
		}
	}
}

void prune_sitebans()
{
	string *keys;
	mixed ban;
	int sz;

	keys = map_indices(sitebans);

	for (sz = sizeof(keys); --sz >= 0; ) {
		mixed expire;
		string key;

		key = keys[sz];
		ban = sitebans[key];

		expire = ban["expire"];

		if (expire != nil && expire != -1) {
			if (time() >= expire) {
				sitebans[key] = nil;
			}
		}
	}
}

void ban_user(string username, varargs mixed message_arg, mixed expire_arg)
{
	string *pnames;
	mixed *pvalues;
	int sz;

	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	switch(typeof(message_arg)) {
	case T_NIL:
	case T_STRING:
		/* old style */
		bans[username] = ([ ]);

		if (message_arg) {
			bans[username]["message"] = message_arg;
		}
		if (expire_arg == nil || expire_arg == -1) {
			bans[username]["expire"] = -1;
		} else {
			bans[username]["expire"] = expire_arg;
		}
		break;

	case T_MAPPING:
		/* new style */
		pnames = map_indices(message_arg);
		pvalues = map_values(message_arg);

		for (sz = sizeof(pnames); --sz >= 0; ) {
			string pname;
			mixed pvalue;

			pname = pnames[sz];
			pvalue = pvalues[sz];

			check_property(pname, pvalue);
		}

		bans[username] = message_arg[..];
	}

	call_out("save", 0);
}

void unban_user(string username)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	bans[username] = nil;

	call_out("save", 0);
}

int query_is_user_banned(string username)
{
	mixed ban;
	mixed expire;

	ban = bans[username];

	if (ban == nil) {
		return 0;
	}

	expire = ban["expire"];

	if (expire != nil && expire != -1) {
		if (time() >= expire) {
			return 0;
		}
	}

	return 1;
}

string query_ban_message(string username)
{
	mixed ban;
	mixed expire;

	ban = bans[username];

	if (ban == nil) {
		return nil;
	}

	expire = ban["expire"];

	if (expire != nil && expire != -1) {
		if (time() >= expire) {
			return nil;
		}
	}

	return ban["message"];
}

string *query_bans()
{
	prune_bans();

	return map_indices(bans);
}

mapping query_ban(string username)
{
	mixed ban;
	mixed expire;

	ban = bans[username];

	if (ban == nil) {
		return nil;
	}

	expire = ban["expire"];

	if (expire != nil && expire != -1) {
		if (time() >= expire) {
			return nil;
		}
	}

	return ban[..];
}

void ban_site(string mask, varargs mixed message_arg, mixed expire_arg)
{
	string *pnames;
	mixed *pvalues;
	int sz;

	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	mask = canonicalize_mask(mask);

	switch(typeof(message_arg)) {
	case T_NIL:
	case T_STRING:
		/* old style */
		sitebans[mask] = ([ ]);

		if (message_arg) {
			sitebans[mask]["message"] = message_arg;
		}
		if (expire_arg == nil || expire_arg == -1) {
			sitebans[mask]["expire"] = -1;
		} else {
			sitebans[mask]["expire"] = expire_arg;
		}
		break;

	case T_MAPPING:
		/* new style */
		pnames = map_indices(message_arg);
		pvalues = map_values(message_arg);

		for (sz = sizeof(pnames); --sz >= 0; ) {
			string pname;
			mixed pvalue;

			pname = pnames[sz];
			pvalue = pvalues[sz];

			check_property(pname, pvalue);
		}

		sitebans[mask] = message_arg[..];
	}

	call_out("save", 0);

	SYSTEM_USERD->check_sitebans();
}

void unban_site(string mask)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	mask = canonicalize_mask(mask);

	sitebans[mask] = nil;

	call_out("save", 0);
}

int query_is_site_banned(string site)
{
	mixed ban;
	mixed expire;

	site = canonicalize_mask(site);
	ban = sitebans[site];

	if (ban == nil) {
		return 0;
	}

	expire = ban["expire"];

	if (expire != nil && expire != -1) {
		if (time() >= expire) {
			return 0;
		}
	}

	return 1;
}

string query_siteban_message(string mask)
{
	mixed ban;
	mixed expire;

	ban = sitebans[canonicalize_mask(mask)];

	if (ban == nil) {
		return nil;
	}

	expire = ban["expire"];

	if (expire != nil && expire != -1) {
		if (time() >= expire) {
			return nil;
		}
	}

	return ban["message"];
}

string *query_sitebans()
{
	prune_sitebans();

	return map_indices(sitebans);
}

mapping query_siteban(string mask)
{
	mixed ban;
	mixed expire;

	ban = sitebans[mask];

	if (ban == nil) {
		return nil;
	}

	expire = ban["expire"];

	if (expire != nil && expire != -1) {
		if (time() >= expire) {
			return nil;
		}
	}

	return ban[..];
}

int test_siteban(string ip)
{
	int cidr;
	string masked;

	for (cidr = 32; cidr >= 0; cidr--) {
		masked = cidr_mask(ip, cidr);

		if (query_is_site_banned(cidr_mask(ip, cidr) + "/" + cidr)) {
			return 1;
		}
	}

	return 0;
}

mapping check_siteban(string ip)
{
	int cidr;
	string masked;

	for (cidr = 32; cidr >= 0; cidr--) {
		mapping ban;

		masked = cidr_mask(ip, cidr);

		ban = query_siteban(masked + "/" + cidr);

		if (ban) {
			return ban;
		}
	}

	return nil;
}

string check_siteban_message(string ip)
{
	mapping ban;

	ban = check_siteban(ip);

	if (ban) {
		return ban["message"];
	}
}
