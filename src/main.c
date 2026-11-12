#include <Raven.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// NOTE: This part here is windows bullshit, this makes sure that in release mode, it compile fine, normally, u don't need this in command line apps
#ifdef _WIN32
#include <windows.h>

#ifdef NDEBUG
int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE previous_instance,
    LPSTR command_line,
    int show_command)
{
    return main();
}
#endif
#endif

static Rid windowID;

static void on_quit(Rid button, void* user_data) {
    (void)button;
    (void)user_data;
    Rquit(0);
}


static void toggle_chapter_menu(Rid button, void* user_data) {
    (void)button;
	(void)user_data;
	Rid menu = Rwidget_find(windowID, "chapter_menu");
    //assert(Rwidget_get_visibility(menu), -1);
	if (Rwidget_get_visibility(menu) == R_VISIBLE) {
		Rwidget_set_visibility(menu, R_COLLAPSED);
	}
	else {
		Rwidget_set_visibility(menu, R_VISIBLE);
	}
}
static void populate_chapters(Rid chapter_menu) {
	// Populate the chapter menu with buttons for each chapter
	for (int i = 1; i <= 5; ++i) {
		char button_name[20];
		snprintf(button_name, sizeof(button_name), "chapter_%d", i);
		printf("Creating button: %s\n", button_name);   
		Rid button = Rbutton_create(button_name);
		Rmenu_add_item(chapter_menu, button);
	}
}
int main(void) {
    if (!Rinit(R_BACKEND_VULKAN)) return 1;
    Raction_bind("app.quit", on_quit, NULL);
    Raction_bind("chapter_menu.toggle", toggle_chapter_menu, NULL);
    windowID = Rwindow_create_from_xml_file("./assets/window.xml");
    if (windowID == R_INVALID_ID)  return 1; 

    //populate_chapters(Rwidget_find(windowID, "chapter_menu"));
	populate_chapters(Rwidget_find(windowID, "chapter_menu"));
	

    while (Rwait_frame(-1.0)) {}
    Rshutdown();
    return 0;
}