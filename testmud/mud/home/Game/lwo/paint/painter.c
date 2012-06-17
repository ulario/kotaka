mapping layers;

int forecolor;
int backcolor;

static void create(int clone)
{
	if (clone) {
		forecolor = backcolor = -1;
	}
}

void start()
{
	forecolor = -1;
	backcolor = -1;
	layers = ([ ]);
}

void set_forecolor(int new_fore)
{
}
