#include <kotaka/paths/account.h>

inherit "/lib/sort";
inherit "style";

int message(string str);

static int compare_sitebans(mixed *a, mixed *b)
{
	mixed ae, be;

	ae = a[1]["expire"];
	be = b[1]["expire"];

	if (ae) {
		if (be) {
			if (ae < be) {
				return -1;
			} else if (ae > be) {
				return 1;
			}
		} else {
			return -1;
		}
	} else if (be) {
		return 1;
	} else {
		return 0;
	}
}

static void handle_get_siteban()
{
	object header;
	int i, sz, time;

	mixed *sitebans;
	string *sites;

	header = new_object("~/lwo/http_response");
	header->set_status(200, "Siteban list");

	message(header->generate_header());
	message("<html>\n");
	message("<head>\n");
	message(style());
	message("<title>Sitebans</title>\n");
	message("</head>\n");
	message("<body>\n");
	message("<h1>Sitebans</h1>\n");
	message("<table>\n");
	message("<tr><th>Site</th><th>Issuer</th><th>Expire</th><th>Message</th></tr>\n");

	time = time();
	sites = BAND->query_sitebans();
	sz = sizeof(sites);
	sitebans = allocate(sz);

	for (i = 0; i < sz; i++) {
		mapping ban;
		string site;

		site = sites[i];

		ban = BAND->query_siteban(site);

		if (!ban) {
			continue;
		}

		sitebans[i] = ({ site, ban });
	}

	sitebans -= ({ nil });

	quicksort(sitebans, 0, sizeof(sitebans), "compare_sitebans");

	for (i = 0, sz = sizeof(sitebans); i < sz; i++) {
		string site;
		mapping siteban;
		mixed expire;
		mixed remaining;

		site = sitebans[i][0];
		siteban = sitebans[i][1];

		expire = siteban["expire"];

		if (expire == nil) {
			remaining = "Permanent";
		} else {
			remaining = expire - time;

			if (remaining < 60) {
				remaining += (remaining == 1 ? " second" : " seconds");
			} else if (remaining < 3600) {
				remaining += 59;
				remaining /= 60;
				remaining += (remaining == 1 ? " minute" : " minutes");
			} else if (remaining < 86400) {
				remaining += 3599;
				remaining /= 3600;
				remaining += (remaining == 1 ? " hour" : " hours");
			} else {
				remaining += 86399;
				remaining /= 86400;;
				remaining += (remaining == 1 ? " day" : " days");
			}
		}

		message("<tr><td>" + site + "</td><td>" + siteban["issuer"]
			+ "</td><td>" + remaining + "</td><td>" + siteban["message"] + "</td></tr>\n"
		);
	}

	message("</table>\n");
	message("</body>\n");
	message("</html>\n");
	message("</html>\n");
}
