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

static void on_quit(Rid button, void* user_data) {
    (void)button;
    (void)user_data;
    Rquit(0);
}

int main(void) {
    if (!Rinit(R_BACKEND_VULKAN)) return 1;

    Rid window = Rwindow_create("Hello", 420, 220, R_WINDOW_DEFAULT);
    Rid box = Rwidget_create(R_WIDGET_VBOX);
    Rpanel_set_spacing(box, 12);
    Rslot_set_halign(box, R_HALIGN_CENTER);
    Rslot_set_valign(box, R_VALIGN_CENTER);
    Rwindow_set_content(window, box);

    Rwidget_add_child(box, Rlabel_create("Hello from Raven"));
    Rid quit = Rwidget_add_child(box, Rbutton_create("Quit"));
    Rbutton_set_clicked_callback(quit, on_quit, NULL);

    while (Rwait_frame(-1.0)) {}
    Rshutdown();
    return Rget_exit_code();
}

