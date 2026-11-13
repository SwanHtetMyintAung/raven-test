#include <Raven.h>

// NOTE: This is the json library from: "https://github.com/DaveGamble/cJSON". Clanker for doc

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "jsonreader.h"


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
static SouMatome root = {0};
static cJSON* json = NULL;

static void initsoumatome() {
	if (!parse_soumatome(json, &root))
	{
		printf("Failed to populate SouMatome!\n");

		cJSON_Delete(json);
		exit(1);
	}
}

// NOTE: void name(Rid, const char*, void*). sender is the widget that triggered this callback. char* action is the action name like click, change etc(u may have to consult offical doc fAll button callbacks have to be this signatureor this), void* userdata is custom data registered with callback

static void on_quit(Rid sender, const char* action, void* user_data) {
    (void)sender;
    (void)user_data;
    Rquit(0);
}


static void toggle_chapter_menu(Rid sender, const char* action, void* user_data) {
    (void)sender;
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
	for (int i = 1; i <= 20; ++i) {
		char button_name[40];
		snprintf(button_name, sizeof(button_name), "備chapter_%d", i);
        // NOTE: this function takes widget id and char*, not another widget
		Rid menu = Rmenu_add_item(chapter_menu, button_name);
		Rmenu_item_set_clicked_callback(menu, NULL, NULL);
	}
}

static void switcher_index_to_0(Rid sender, const char* action, void* user_data) {
	(void)sender;
	(void)user_data;
	Rid switcher = Rwidget_find(windowID, "button_switcher");
	Rswitcher_set_index(switcher, 0);
}
static void switcher_index_to_1(Rid sender, const char* action, void* user_data) {
	(void)sender;
	(void)user_data;
	Rid switcher = Rwidget_find(windowID, "button_switcher");
	Rswitcher_set_index(switcher, 1);
}


int main(void) {
	json = cJSON_Parse(parseJson("assets/data/soumatome_n3_kanji_with_examples.json"));
	if(!json) {
		printf("Failed to parse json\n");
		return 1;
	}
	initsoumatome();

    if (!Rinit(R_BACKEND_VULKAN)) return 1;
    Raction_bind("app.quit", on_quit, NULL);
    Raction_bind("chapter_menu.toggle", toggle_chapter_menu, NULL);
	Raction_bind("switcher.index.0", switcher_index_to_0, NULL);
	Raction_bind("switcher.index.1", switcher_index_to_1, NULL);
    windowID = Rwindow_create_from_xml_file("./assets/window.xml");
    if (windowID == R_INVALID_ID)  return 1; 

    //populate_chapters(Rwidget_find(windowID, "chapter_menu"));
	populate_chapters(Rwidget_find(windowID, "chapter_menu"));
	

    while (Rwait_frame(-1.0)) {}
    Rshutdown();
    return 0;
}
