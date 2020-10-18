static string style()
{
	return "<style>\n"
		+ read_file("http.css")
		+ "</style>\n";
}
