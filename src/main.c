#include <Raven.h>

// NOTE: This is the json library from: "https://github.com/DaveGamble/cJSON". Clanker for doc

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "cvector.h"
#include "jsonreader.h"
#include "raven/raven_core.h"
#include <time.h>

#ifdef APP_EMBED_XML
#include "window_embedded.h"
#endif
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

typedef struct
{
    size_t week_index;
    size_t day_index;
} ChapterContext;

typedef struct
{
    ChapterContext* items;
    size_t count;
} ChapterListData;

static ChapterListData chapter_data = {0};

static Rid windowID;
static SouMatome root = {0};
static cJSON* json = NULL;
static ChapterContext* chapter_contexts = NULL;
static size_t selected_week_index = 0;
static size_t selected_day_index = 0;
static size_t selected_kanji_index = 0;
static Rid chapter_menu = R_INVALID_ID;
static Rid chapter_list = R_INVALID_ID;

static void initsoumatome() {
	if (!parse_soumatome(json, &root))
	{
		printf("Failed to populate SouMatome!\n");

		cJSON_Delete(json);
		exit(1);
	}
}

static void refresh_kanji(
    Rid sender,
    const char* action,
    void* user_data
);

static int on_window_event(
    Rid window,
    const REvent* event,
    void* user_data)
{
    (void)window;
    (void)user_data;

    if (Rwidget_get_visibility(chapter_list) != R_VISIBLE)
        return R_FALSE;

    if (event->type != R_EVENT_MOUSE_DOWN ||
        event->button != R_MOUSE_LEFT)
        return R_FALSE;

    float x, y, width, height;

    Rwidget_get_rect(
        chapter_list,
        &x, &y, &width, &height
    );

    // Check whether the click was outside the ListView.
    if (event->x < x || event->x > x + width ||
        event->y < y || event->y > y + height)
    {
        Rwidget_set_visibility(chapter_list, R_COLLAPSED);
    }

    return R_FALSE;
}

static void next_kanji(
    Rid sender,
    const char* action,
    void* user_data)
{
    (void)sender;
    (void)action;
    (void)user_data;

    Week* week = &root.weeks[selected_week_index];
    Day* day = &week->days[selected_day_index];

    size_t count = cvector_size(day->kanjis);

    if (selected_kanji_index + 1 >= count) {//final kanji
        if (selected_day_index + 1 >= cvector_size(week->days)) {//final day
            if (selected_week_index + 1 >= cvector_size(week)) {//final week
                return;
            }
            else {
                selected_week_index++;
                selected_day_index = 0;
                selected_kanji_index = 0;
            }
        }
        else {
			selected_day_index++;
            selected_kanji_index = 0;
        }
        
    }

    selected_kanji_index++;

    refresh_kanji(R_INVALID_ID, NULL, NULL);

    // Return to the question side.
    Rid switcher = Rwidget_find(windowID, "button_switcher");
    Rid meaning = Rwidget_find(windowID, "meaning");

    Rwidget_set_visibility(meaning, R_HIDDEN);
    Rswitcher_set_index(switcher, 0);
}

static void previous_kanji(
    Rid sender,
    const char* action,
    void* user_data)
{
    (void)sender;
    (void)action;
    (void)user_data;

    if (selected_kanji_index == 0) {

        // First kanji of the first day
        if (selected_day_index == 0) {

            // First kanji of the first week
            if (selected_week_index == 0) {
                return;
            }

            // Move to previous week
            selected_week_index--;

            selected_day_index =
                cvector_size(root.weeks[selected_week_index].days) - 1;
        }
        else {
            // Move to previous day
            selected_day_index--;
        }

        // Select the last kanji of the previous day
        selected_kanji_index = 0;
    }
    else {
        selected_kanji_index--;
    }

    refresh_kanji(R_INVALID_ID, NULL, NULL);

    Rid switcher = Rwidget_find(windowID, "button_switcher");
    Rid meaning = Rwidget_find(windowID, "meaning");

    Rwidget_set_visibility(meaning, R_HIDDEN);
    Rswitcher_set_index(switcher, 0);
}

static void on_chapter_selected(
    Rid list_view,
    int index,
    void* user_data)
{
    (void)list_view;

    ChapterListData* data = user_data;

    if (!data || index < 0 ||
        (size_t)index >= data->count)
        return;

    ChapterContext* ctx = &data->items[index];

    Week* week = &root.weeks[ctx->week_index];
    Day* day = &week->days[ctx->day_index];

    selected_week_index = ctx->week_index;
    selected_day_index = ctx->day_index;
	selected_kanji_index = 0;

    refresh_kanji(R_INVALID_ID, NULL, NULL);
	// Clear selection so the same chapter can be selected again.
    Rlist_view_set_selected(list_view, -1);

    // Close the chapter list.
    Rwidget_set_visibility(list_view, R_COLLAPSED);
}

static void on_chapter_clicked(Rid item, void* user_data)
{
    (void)item;

    ChapterContext* ctx = user_data;

    if (!ctx)
        return;

	if (ctx->week_index >= cvector_size(root.weeks))
        return;

    Week* week = &root.weeks[ctx->week_index];

	if (ctx->day_index >= cvector_size(week->days))
        return;

    selected_week_index = ctx->week_index;
    selected_day_index = ctx->day_index;

    // Display a random kanji from the selected day.
    refresh_kanji(R_INVALID_ID, NULL, NULL);
    printf("Selected Week: %zu\n", selected_week_index);
    printf("Selected Day: %zu\n", selected_day_index);

    // NOTE: Now you can display the kanjis of this day.
}

// NOTE: void name(Rid, const char*, void*). sender is the widget that triggered this callback. char* action is the action name like click, change etc(u may have to consult offical doc fAll button callbacks have to be this signatureor this), void* userdata is custom data registered with callback

static void on_quit(Rid sender, const char* action, void* user_data) {
    (void)sender;
    (void)user_data;
    Rquit(0);
}

static void refresh_kanji(
    Rid sender,
    const char* action,
    void* user_data)
{
    (void)sender;
    (void)action;
    (void)user_data;

    if (selected_week_index >= cvector_size(root.weeks))
        return;

    Week* week = &root.weeks[selected_week_index];

    if (selected_day_index >= cvector_size(week->days))
        return;

    Day* day = &week->days[selected_day_index];

    size_t kanji_count = cvector_size(day->kanjis);

    if (kanji_count == 0 ||
        selected_kanji_index >= kanji_count)
        return;

    const Kanji* kanji =
        &day->kanjis[selected_kanji_index];

    Rid kanji_widget = Rwidget_find(windowID, "kanji");
    Rid meaning_widget = Rwidget_find(windowID, "meaning");

    Rlabel_set_text(kanji_widget, kanji->kanji);

    char answer_text[512];

    snprintf(
        answer_text,
        sizeof(answer_text),
        "%s, %s",
        kanji->hiragana,
        kanji->meaning
    );

    Rlabel_set_text(meaning_widget, answer_text);


    Rid week_day_label = Rwidget_find(windowID, "week_day");
    char text[40];
    snprintf(
        text,
        40,
        "Week %zu Day %zu",
        selected_week_index + 1,
        selected_day_index + 1
    );
    Rlabel_set_text(week_day_label, text);
}

static void toggle_chapter_menu(
    Rid sender,
    const char* action,
    void* user_data)
{
    (void)sender;
    (void)action;
    (void)user_data;


    Rid list = Rwidget_find(windowID, "chapter_list");

    if (Rwidget_get_visibility(list) == R_VISIBLE)
        Rwidget_set_visibility(list, R_COLLAPSED);
    else
        Rwidget_set_visibility(list, R_VISIBLE);
}

static void populate_chapters(Rid chapter_list)
{
    size_t total_items = 0;

    Rlist_view_clear_items(chapter_list);

    free(chapter_data.items);
    chapter_data = (ChapterListData){0};

    // Count all days.
    for (size_t i = 0; i < cvector_size(root.weeks); i++)
    {
        total_items += cvector_size(root.weeks[i].days);
    }

    chapter_data.items = calloc(
        total_items,
        sizeof(ChapterContext)
    );

    if (total_items > 0 && !chapter_data.items)
    {
        printf("Failed to allocate chapter contexts!\n");
        return;
    }

    for (size_t i = 0; i < cvector_size(root.weeks); i++)
    {
        Week* week = &root.weeks[i];

        for (size_t j = 0; j < cvector_size(week->days); j++)
        {
            Day* day = &week->days[j];

            char item_name[40];

            snprintf(
                item_name,
                sizeof(item_name),
                "Week %d Day %d",
                week->week,
                day->day
            );

            int index = Rlist_view_add_item(
                chapter_list,
                item_name
            );

            if (index < 0 ||
                (size_t)index >= total_items)
            {
                printf("Failed to add chapter item!\n");
                Rlist_view_clear_items(chapter_list);
                free(chapter_data.items);
                chapter_data = (ChapterListData){0};
                return;
            }

            chapter_data.items[index] = (ChapterContext)
            {
                .week_index = i,
                .day_index = j
            };
        }
    }

    chapter_data.count = (size_t)Rlist_view_get_item_count(
        chapter_list
    );

    Rlist_view_set_selection_changed_callback(
        chapter_list,
        on_chapter_selected,
        &chapter_data
    );
}

static void switcher_index_to_0(Rid sender, const char* action, void* user_data) {
	(void)sender;
	(void)user_data;
	refresh_kanji(R_INVALID_ID, NULL, NULL);
	Rid switcher = Rwidget_find(windowID, "button_switcher");
	Rid meaning_widget = Rwidget_find(windowID, "meaning");
	Rwidget_set_visibility(meaning_widget, R_HIDDEN);
	Rswitcher_set_index(switcher, 0);
}
static void switcher_index_to_1(Rid sender, const char* action, void* user_data) {
	(void)sender;
	(void)user_data;
	Rid switcher = Rwidget_find(windowID, "button_switcher");
	Rid meaning_widget = Rwidget_find(windowID, "meaning");
	Rwidget_set_visibility(meaning_widget, R_VISIBLE);
	Rswitcher_set_index(switcher, 1);
}


int main(void) {
	srand(time(NULL));
	json = cJSON_Parse(parseJson("assets/data/soumatome_n3_kanji_with_examples.json"));
	if(!json) {
		printf("Failed to parse json\n");
		return 1;
	}
	initsoumatome();
    if (!Rinit(R_BACKEND_VULKAN)) return 1;
	//events here
    Raction_bind("app.quit", on_quit, NULL);
    Raction_bind("chapter_menu.toggle", toggle_chapter_menu, NULL);
	Raction_bind("switcher.index.1", switcher_index_to_1, NULL);
	Raction_bind("kanji.next", next_kanji, NULL);
	Raction_bind("kanji.previous", previous_kanji, NULL);
#ifdef APP_EMBED_XML
    windowID = Rwindow_create_from_xml(
        hello_xml,
        hello_xml_size
    );
#else
    windowID = Rwindow_create_from_xml_file(
        "./assets/window.xml"
    );
#endif
	if (windowID == R_INVALID_ID)  return 1; 

	chapter_list = Rwidget_find(windowID, "chapter_list");
	Rwindow_set_event_hook(
		windowID,
		on_window_event,
		NULL
	);

	populate_chapters(chapter_list);	

    while (Rwait_frame(-1.0)) {}
    Rshutdown();
	free(chapter_data.items);
	chapter_data = (ChapterListData){0};
    return 0;
}
