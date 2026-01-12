/* © 2020 David Given.
 * WordGrinder is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include <string.h>
#include <curses.h>
#include <wctype.h>
#include <sys/time.h>
#include <time.h>

#define KEY_TIMEOUT (KEY_MAX + 1)
#define FIRST_COLOUR_ID 1
#define FIRST_PAIR_ID 1

#if defined WA_ITALIC
static bool use_italics = false;
#endif

static bool enable_colours = true;
static bool use_colours = false;
static int currentAttr = 0;
static short currentPair = 0;

typedef struct
{
    uint8_t fg;
    uint8_t bg;
} pair_t;

static std::vector<colour_t> colours;
static std::vector<pair_t> colourPairs;

void dpy_init(const char* argv[])
{
    // ESCDELAY defaults to 1000 (ms) in ncurses. This is why the menu requires a 1 second delay to appear after hitting escape.
    // Setting it too low might interfere with other control key inputs.. 30 doesn't seem to cause problems, and the menu
    // appears quickly. 
    ESCDELAY = 30;

    while (*argv)
    {
        if (strcmp(*argv, "--no-ncurses-colour") == 0)
            enable_colours = false;
        if (strcmp(*argv, "--") == 0)
            break;
        argv++;
    }
}

void dpy_start(void)
{
    initscr();

    use_colours = enable_colours && has_colors() && can_change_color();
    if (use_colours)
        start_color();

    raw();
    noecho();
    meta(NULL, TRUE);
    nonl();
    idlok(stdscr, TRUE);
    idcok(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    intrflush(stdscr, FALSE);
    // notimeout(stdscr, TRUE);
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);

#if defined A_ITALIC
    use_italics = !!tigetstr((char*) "sitm");
#endif
}

void dpy_shutdown(void)
{
    colours.clear();
    colourPairs.clear();
    endwin();
}

void dpy_clearscreen(void)
{
    int w, h;
    getmaxyx(stdscr, h, w);
    dpy_cleararea(0, 0, w - 1, h - 1);
}

void dpy_getscreensize(int* x, int* y)
{
    getmaxyx(stdscr, *y, *x);
}

void dpy_getmouse(uni_t key, int* x, int* y, bool* p)
{
    x = y = 0;
    *p = false;
}

void dpy_sync(void)
{
    refresh();
}

void dpy_setcursor(int x, int y, bool shown)
{
    move(y, x);
}

static void update_attrs()
{
    attr_t cattr = 0;
    if (currentAttr & DPY_ITALIC)
    {
#if defined WA_ITALIC
        if (use_italics)
            cattr |= WA_ITALIC;
        else
            cattr |= WA_BOLD;
#else
        cattr |= WA_BOLD;
#endif
    }
    if (currentAttr & DPY_BOLD)
        cattr |= WA_BOLD;
    if (!use_colours && (currentAttr & DPY_BRIGHT))
        cattr |= WA_BOLD;
    if (!use_colours && (currentAttr & DPY_DIM))
        cattr |= WA_DIM;
    if (currentAttr & DPY_UNDERLINE)
        cattr |= WA_UNDERLINE;
    if (currentAttr & DPY_REVERSE)
        cattr |= WA_REVERSE;

    if (use_colours)
        attr_set(cattr, currentPair, NULL);
    else
        attr_set(cattr, 0, NULL);
}

void dpy_setattr(int andmask, int ormask)
{
    currentAttr &= andmask;
    currentAttr |= ormask;

    update_attrs();
}

static uint8_t lookup_colour(const colour_t* colour)
{
    for (uint8_t i = 0; i < colours.size(); i++)
    {
        if ((colours[i].r == colour->r) && (colours[i].g == colour->g) &&
            (colours[i].b == colour->b))
            return i + FIRST_COLOUR_ID;
    }

    uint8_t id = colours.size() + FIRST_COLOUR_ID;
    colours.emplace_back(*colour);
    init_color(id, colour->r * 1000.0, colour->g * 1000.0, colour->b * 1000.0);
    return id;
}

void dpy_setcolour(const colour_t* fg, const colour_t* bg)
{
    if (!use_colours)
        return;

    uint8_t fgc = lookup_colour(fg);
    uint8_t bgc = lookup_colour(bg);

    for (int i = 0; i < colourPairs.size(); i++)
    {
        pair_t* p = &colourPairs[i];
        if ((p->fg == fgc) && (p->bg == bgc))
        {
            currentPair = FIRST_PAIR_ID + i;
            update_attrs();
            return;
        }
    }

    currentPair = colourPairs.size() + FIRST_PAIR_ID;
    colourPairs.emplace_back(pair_t{fgc, bgc});

    init_pair(currentPair, fgc, bgc);
    update_attrs();
}

void dpy_writechar(int x, int y, uni_t c)
{
    char buffer[8];
    char* p = buffer;
    writeu8(&p, c);
    *p = '\0';

    mvaddstr(y, x, buffer);
}

void dpy_cleararea(int x1, int y1, int x2, int y2)
{
    char cc = ' ';

    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++)
            mvaddnstr(y, x, &cc, 1);
}

static int handle_mouse(void)
{
    static int mx = -1;
    static int my = -1;
    static bool p = false;

    MEVENT event;
    getmouse(&event);
    switch (event.bstate)
    {
        case REPORT_MOUSE_POSITION:
            mx = event.x;
            my = event.y;
            if (!p)
                return 0;
            break;

        case BUTTON1_PRESSED:
            mx = event.x;
            my = event.y;
            p = true;
            break;

        case BUTTON1_RELEASED:
            mx = event.x;
            my = event.y;
            p = false;
            break;

        case BUTTON2_PRESSED:
        case BUTTON2_CLICKED:
        case BUTTON3_PRESSED:
        case BUTTON3_CLICKED:
            return KEY_MENU;

#if NCURSES_MOUSE_VERSION > 1
        case BUTTON4_PRESSED:
        case BUTTON4_CLICKED:
            return KEY_SCROLLUP;

        case BUTTON5_PRESSED:
        case BUTTON5_CLICKED:
            return KEY_SCROLLDOWN;
#endif
    }

    return encode_mouse_event(mx, my, p);
}

uni_t dpy_getchar(double timeout)
{
    struct timeval then;
    gettimeofday(&then, NULL);
    uint64_t thenms = (then.tv_usec / 1000) + ((uint64_t)then.tv_sec * 1000);

    for (;;)
    {

        if (timeout != -1)
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            uint64_t nowms =
                (now.tv_usec / 1000) + ((uint64_t)now.tv_sec * 1000);

            int delay = ((uint64_t)(timeout * 1000)) + nowms - thenms;
            if (delay <= 0)
                return -KEY_TIMEOUT;

            timeout(delay);
        }
        else
            timeout(-1);

        wint_t c;
        int r = get_wch(&c);

        if (r == ERR) /* timeout */
            return -KEY_TIMEOUT;

        if (c == KEY_MOUSE)
            return -handle_mouse();

        if ((r == KEY_CODE_YES) || !iswprint(c)) /* function key */
            return -c;

        if (emu_wcwidth(c) > 0)
            return c;
    }
}

static const char* ncurses_prefix_to_name(const char* s)
{
    if (strcmp(s, "KDC") == 0)
        return "DELETE";
    if (strcmp(s, "kDN") == 0)
        return "DOWN";
    if (strcmp(s, "kEND") == 0)
        return "END";
    if (strcmp(s, "kHOM") == 0)
        return "HOME";
    if (strcmp(s, "kIC") == 0)
        return "INSERT";
    if (strcmp(s, "kLFT") == 0)
        return "LEFT";
    if (strcmp(s, "kNXT") == 0)
        return "PGDN";
    if (strcmp(s, "kPRV") == 0)
        return "PGUP";
    if (strcmp(s, "kRIT") == 0)
        return "RIGHT";
    if (strcmp(s, "kUP") == 0)
        return "UP";

    return s;
}

static const char* ncurses_suffix_to_name(int suffix)
{
    switch (suffix)
    {
        case 3:
            return "A";
        case 4:
            return "SA";
        case 5:
            return "^";
        case 6:
            return "S^";
        case 7:
            return "A^";
    }

    return NULL;
}

std::string dpy_getkeyname(uni_t k)
{
    k = -k;

    switch (k)
    {
        case 127: /* Some misconfigured terminals produce this */
        case KEY_BACKSPACE:
            return "KEY_BACKSPACE";

        case KEY_SCROLLUP:
            return "KEY_SCROLLUP";
        case KEY_SCROLLDOWN:
            return "KEY_SCROLLDOWN";
        case KEY_MENU:
            return "KEY_MENU";

        case KEY_TIMEOUT:
            return "KEY_TIMEOUT";
        case KEY_DOWN:
            return "KEY_DOWN";
        case KEY_UP:
            return "KEY_UP";
        case KEY_LEFT:
            return "KEY_LEFT";
        case KEY_RIGHT:
            return "KEY_RIGHT";
        case KEY_HOME:
            return "KEY_HOME";
        case KEY_END:
            return "KEY_END";
        case KEY_DC:
            return "KEY_DELETE";
        case KEY_IC:
            return "KEY_INSERT";
        case KEY_NPAGE:
            return "KEY_PGDN";
        case KEY_PPAGE:
            return "KEY_PGUP";
        case KEY_STAB:
            return "KEY_STAB";
        case KEY_CTAB:
            return "KEY_^TAB";
        case KEY_CATAB:
            return "KEY_^ATAB";
        case KEY_ENTER:
            return "KEY_RETURN";
        case KEY_SIC:
            return "KEY_SINSERT";
        case KEY_SDC:
            return "KEY_SDELETE";
        case KEY_SHOME:
            return "KEY_SHOME";
        case KEY_SEND:
            return "KEY_SEND";
        case KEY_SR:
            return "KEY_SUP";
        case KEY_SF:
            return "KEY_SDOWN";
        case KEY_SLEFT:
            return "KEY_SLEFT";
        case KEY_SRIGHT:
            return "KEY_SRIGHT";
        case KEY_MOUSE:
            return "KEY_MOUSE";
        case KEY_RESIZE:
            return "KEY_RESIZE";
        case 13:
            return "KEY_RETURN";
        case 27:
            return "KEY_ESCAPE";
    }

    if (k < 32) {
        std::string Result = "KEY_^";
        Result.push_back(k + 'A' - 1);
        return Result;
    }

    if ((k >= KEY_F0) && (k < (KEY_F0 + 64))) {
        std::string Result = "KEY_F{}";
        Result += std::to_string(k - KEY_F0);
        return Result;
    }

    const char* name = keyname(k);
    if (name)
    {
        char buf[strlen(name) + 1];
        strcpy(buf, name);

        int prefix = strcspn(buf, "0123456789");
        int suffix = buf[prefix] - '0';
        buf[prefix] = '\0';

        if ((suffix >= 0) && (suffix <= 9))
        {
            const char* ps = ncurses_prefix_to_name(buf);
            const char* ss = ncurses_suffix_to_name(suffix);
            if (ss) {
                std::string Result = "KEY_";
                Result += ss;
                Result += ps;
                return Result;
            }
        }
    }

    std::string Result = "KEY_UNKNOWN_";
    Result += k;
    Result += " (";
    Result += name ? name : "???";
    Result += ")";
    return Result;
}

static bool running = false;
static int cursorx = 0;
static int cursory = 0;
static bool cursorshown = true;

void screen_deinit(void)
{
    if (running)
    {
        dpy_shutdown();
        running = false;
    }
}

static int initscreen_cb(lua_State* L)
{
    dpy_start();

    running = true;
    atexit(screen_deinit);
    return 0;
}

static int deinitscreen_cb(lua_State* L)
{
    screen_deinit();
    return 0;
}

static int clearscreen_cb(lua_State* L)
{
    dpy_clearscreen();
    return 0;
}

static int sync_cb(lua_State* L)
{
    dpy_setcursor(cursorx, cursory, cursorshown);
    dpy_sync();
    return 0;
}

static int setbold_cb(lua_State* L)
{
    dpy_setattr(-1, DPY_BOLD);
    return 0;
}

static int setunderline_cb(lua_State* L)
{
    dpy_setattr(-1, DPY_UNDERLINE);
    return 0;
}

static int setreverse_cb(lua_State* L)
{
    dpy_setattr(-1, DPY_REVERSE);
    return 0;
}

static int setdim_cb(lua_State* L)
{
    dpy_setattr(-1, DPY_DIM);
    return 0;
}

static int setbright_cb(lua_State* L)
{
    dpy_setattr(-1, DPY_BRIGHT);
    return 0;
}

static int setitalic_cb(lua_State* L)
{
    dpy_setattr(-1, DPY_ITALIC);
    return 0;
}

static float getnumber(lua_State* L, int table, int index)
{
    lua_rawgeti(L, table, index);
    float value = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return value;
}

static int setcolour_cb(lua_State* L)
{
    colour_t fg = {
        getnumber(L, 1, 1),
        getnumber(L, 1, 2),
        getnumber(L, 1, 3),
    };

    colour_t bg = {
        getnumber(L, 2, 1),
        getnumber(L, 2, 2),
        getnumber(L, 2, 3),
    };

    dpy_setcolour(&fg, &bg);
    return 0;
}

static int setnormal_cb(lua_State* L)
{
    dpy_setattr(0, 0);
    return 0;
}

void dpy_writeunichar(int x, int y, uni_t c)
{
    if (!enable_unicode && (c > 0xff))
        c = '?';
    dpy_writechar(x, y, c);
}

static int write_cb(lua_State* L)
{
    int x = forceinteger(L, 1);
    int y = forceinteger(L, 2);
    size_t size;
    const char* s = luaL_checklstring(L, 3, &size);
    const char* send = s + size;

    while (s < send)
    {
        uni_t c = readu8(&s);
        dpy_writeunichar(x, y, c);

        if (!iswcntrl(c))
            x += emu_wcwidth(c);
    }

    return 0;
}

static int cleararea_cb(lua_State* L)
{
    int x1 = forceinteger(L, 1);
    int y1 = forceinteger(L, 2);
    int x2 = forceinteger(L, 3);
    int y2 = forceinteger(L, 4);
    dpy_cleararea(x1, y1, x2, y2);
    return 0;
}

static int gotoxy_cb(lua_State* L)
{
    cursorx = forceinteger(L, 1);
    cursory = forceinteger(L, 2);
    return 0;
}

static int showcursor_cb(lua_State* L)
{
    cursorshown = true;
    return 0;
}

static int hidecursor_cb(lua_State* L)
{
    cursorshown = false;
    return 0;
}

static int getscreensize_cb(lua_State* L)
{
    int x, y;
    dpy_getscreensize(&x, &y);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

static int getstringwidth_cb(lua_State* L)
{
    size_t size;
    const char* s = luaL_checklstring(L, 1, &size);
    const char* send = s + size;

    int width = 0;
    while (s < send)
    {
        uni_t c = readu8(&s);
        if (!iswcntrl(c))
            width += emu_wcwidth(c);
    }

    lua_pushnumber(L, width);
    return 1;
}

static int getboundedstring_cb(lua_State* L)
{
    size_t size;
    const char* start = luaL_checklstring(L, 1, &size);
    const char* send = start + size;
    int width = forceinteger(L, 2);

    const char* s = start;
    while (s < send)
    {
        const char* p = s;
        uni_t c = readu8(&s);
        if (!iswcntrl(c))
        {
            width -= emu_wcwidth(c);
            if (width < 0)
            {
                send = p;
                break;
            }
        }
    }

    lua_pushlstring(L, start, send - start);
    return 1;
}

static int getbytesofcharacter_cb(lua_State* L)
{
    int c = forceinteger(L, 1);

    lua_pushnumber(L, getu8bytes(c));
    return 1;
}

uni_t encode_mouse_event(int x, int y, bool b)
{
    uni_t key = b ? KEY_MOUSEDOWN : KEY_MOUSEUP;
    key |= (x & 0xff);
    key |= (y & 0xff) << 8;
    return key;
}

void decode_mouse_event(uni_t key, int* x, int* y, bool* b)
{
    *b = (key & KEYM_MOUSE) == KEY_MOUSEDOWN;
    *x = key & 0xff;
    *y = (key >> 8) & 0xff;
}

static int getchar_cb(lua_State* L)
{
    double t = -1.0;
    if (!lua_isnone(L, 1))
        t = forcedouble(L, 1);

    dpy_setcursor(cursorx, cursory, cursorshown);
    dpy_sync();

    for (;;)
    {
        uni_t c = dpy_getchar(t);
        if (c <= 0)
        {
            switch (-c & KEYM_MOUSE)
            {
                case KEY_MOUSEDOWN:
                case KEY_MOUSEUP:
                {
                    int x, y;
                    bool b;
                    static bool oldb;
                    decode_mouse_event(-c, &x, &y, &b);

                    lua_newtable(L);
                    lua_pushnumber(L, x);
                    lua_setfield(L, -2, "x");
                    lua_pushnumber(L, y);
                    lua_setfield(L, -2, "y");
                    lua_pushboolean(L, b);
                    lua_setfield(L, -2, "b");
                    lua_pushboolean(L, b && !oldb);
                    lua_setfield(L, -2, "clicked");
                    oldb = b;
                    return 1;
                }

                default:
                {
                    std::string s = dpy_getkeyname(c);
					lua_pushstring(L, s.c_str());
					return 1;
                }
            }
        }

        if (emu_wcwidth(c) > 0)
        {
            static char buffer[8];
            char* p = buffer;

            writeu8(&p, c);
            *p = '\0';

            lua_pushstring(L, buffer);
            break;
        }
    }

    return 1;
}

static int useunicode_cb(lua_State* L)
{
    lua_pushboolean(L, enable_unicode);
    return 1;
}

static int setunicode_cb(lua_State* L)
{
    enable_unicode = lua_toboolean(L, 1);
    return 0;
}

void screen_init(const char* argv[])
{
    dpy_init(argv);

    const static luaL_Reg funcs[] = {
        {"initscreen",          initscreen_cb         },
        {"deinitscreen",        deinitscreen_cb       },
        {"clearscreen",         clearscreen_cb        },
        {"sync",                sync_cb               },
        {"setbold",             setbold_cb            },
        {"setunderline",        setunderline_cb       },
        {"setreverse",          setreverse_cb         },
        {"setbright",           setbright_cb          },
        {"setdim",              setdim_cb             },
        {"setitalic",           setitalic_cb          },
        {"setnormal",           setnormal_cb          },
        {"setcolour",           setcolour_cb          },
        {"write",               write_cb              },
        {"cleararea",           cleararea_cb          },
        {"gotoxy",              gotoxy_cb             },
        {"showcursor",          showcursor_cb         },
        {"hidecursor",          hidecursor_cb         },
        {"getscreensize",       getscreensize_cb      },
        {"getstringwidth",      getstringwidth_cb     },
        {"getboundedstring",    getboundedstring_cb   },
        {"getbytesofcharacter", getbytesofcharacter_cb},
        {"getchar",             getchar_cb            },
        {"useunicode",          useunicode_cb         },
        {"setunicode",          setunicode_cb         },
        {NULL,                  NULL                  }
    };

    luaL_register(L, "wg", funcs);
}
