#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit LIB_OBJECT;

/* for all derived calculations, object is presumed to be a cube */

/* required */
float density;		/* kg/m^3 */

/* pick one */
float mass;		/* kg, default = density / length ^ 3 */
float length;		/* m, default = (mass / density) ^ 1/3 */

/* optional */
float area;		/* m^2, default = volume / length */

/* forbidden */
float volume;		/* m^3, mass / density */

int flexible;
int container;

float capacity_volume;	/* m^3, optional, default = 0.9 * volume */
float capacity_mass;	/* kg, optional, default = cvolume * density */
float capacity_length;	/* m, optional, default = 0.9 * length */
float capacity_area;	/* m^2, optional, default = cvolume / clength */

static void create()
{
	mass = -1.0;
	density = -1.0;
	volume = -1.0;
	length = -1.0;
	area = -1.0;
}

void set_density(float new_density)
{
	density = new_density;
}

void set_mass(float new_mass)
{
	mass = new_mass;
}

void set_length(float new_length)
{
	length = new_length;
}
