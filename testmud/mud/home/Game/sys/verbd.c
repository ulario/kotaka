#include <kotaka/paths.h>
#include <kotaka/privilege.h>

int do_action(string cmd, object actor, string args)
{
	object ustate;
	object bin;

	ACCESS_CHECK((ustate = previous_object())<-LIB_USTATE);
	bin = find_object("~/sys/verb/" + cmd);

	if (!bin) {
		return FALSE;
	}

	TLSD->set_tls_value("Game", "ustate", ustate);

	bin->main(actor, args);

	TLSD->set_tls_value("Game", "ustate", nil);

	return TRUE;
}