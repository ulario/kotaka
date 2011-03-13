#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit "object";

float xpos, ypos, zpos;

/*********************/
/* Position handling */
/*********************/

static void move_notify(object old_env)
{
	object common;
	object new_env;

	new_env = query_environment();

	if (!old_env || !new_env) {
		xpos = 0.0;
		ypos = 0.0;
		zpos = 0.0;

		return;
	}

	common = SUBD->query_common_container(old_env, new_env);

	if (!common) {
		xpos = 0.0;
		ypos = 0.0;
		zpos = 0.0;

		return;
	}

	for (; old_env != common; old_env = old_env->query_environment()) {
		xpos += old_env->query_x_position();
		ypos += old_env->query_y_position();
		zpos += old_env->query_z_position();
	}

	for (; new_env != common; new_env = new_env->query_environment()) {
		xpos -= old_env->query_x_position();
		ypos -= old_env->query_y_position();
		zpos -= old_env->query_z_position();
	}
}

void set_x_position(float new_xpos)
{
	xpos = new_xpos;
}

void set_y_position(float new_ypos)
{
	ypos = new_ypos;
}

void set_z_position(float new_zpos)
{
	zpos = new_zpos;
}

float query_x_position()
{
	return xpos;
}

float query_y_position()
{
	return ypos;
}

float query_z_position()
{
	return zpos;
}
