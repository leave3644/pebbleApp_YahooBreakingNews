#include <pebble.h>

static Window *window;
static MenuLayer *menu_layer;

enum {
  KEY_TITLE = 0x0,
  KEY_POINTS = 0x1,
  KEY_COMMENTS = 0x2,
  KEY_SUMMARY = 0x3,
  KEY_GET = 0x4,
  KEY_FETCH = 0x5,
  KEY_INDEX = 0x6,
  KEY_READLATER = 0x7,
  KEY_START = 0x8,
  KEY_END = 0x9
};

static ScrollLayer *scroll_layer;
static TextLayer *text_layer;
static TextLayer *title_layer;

// TODO
/*static ActionBarLayer *action_bar;
void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) my_next_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) my_previous_click_handler);
}
  ...
  // Initialize the action bar:
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
  // The loading the icons is omitted for brevity... See HeapBitmap.
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, &my_icon_previous);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, &my_icon_next);*/

#define NUM_ITEMS 30

typedef struct Post {
    int index;
    char title[128];
    char subtitle[50];
    /*int points;
    int comments;*/
} Post;

Post posts[NUM_ITEMS];
int i=0;

// SEE http://forums.getpebble.com/discussion/8582/bug-pebble-logs-fails-with-ascii-codec-can-t-encode-character-u-xfc-in-position

static char subtitle[] = "0 points · 0 comments";

int selected = 0;
int selected_index = 0;
char summary[1024];
#define spacing 10

void readlater(uint8_t i) {
    DictionaryIterator *it;
    app_message_outbox_begin(&it);
    Tuplet tuplet = TupletInteger(KEY_READLATER, i);
    dict_write_tuplet(it, &tuplet);
    dict_write_end(it);
    app_message_outbox_send();
}

static void long_click_handler (ClickRecognizerRef recognizer, void *context){

    readlater(selected_index);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Read Later do");
}

static void click_config_provider(void *context) {

    window_long_click_subscribe(BUTTON_ID_SELECT, 500, NULL, long_click_handler);
}
static void view_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    GRect max_text_bounds = GRect(0, 0, bounds.size.w, 2000);

    scroll_layer = scroll_layer_create(bounds);
    scroll_layer_set_click_config_onto_window(scroll_layer, window);

    title_layer = text_layer_create(max_text_bounds);
    text_layer_set_text(title_layer, posts[selected].title);
    text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

    GSize title_size = text_layer_get_content_size(title_layer);
    text_layer_set_size(title_layer, title_size);

    text_layer = text_layer_create(GRect(0, title_size.h+spacing, max_text_bounds.size.w, max_text_bounds.size.h-(title_size.h+spacing)));
    text_layer_set_text(text_layer, summary);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

    GSize max_size = text_layer_get_content_size(text_layer);
    text_layer_set_size(text_layer, GSize(max_size.w, max_size.h+spacing));

    scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, title_size.h+spacing+max_size.h));
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(title_layer));
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(text_layer));

    scroll_layer_set_click_config_onto_window(scroll_layer, window);


    scroll_layer_set_callbacks(scroll_layer, (ScrollLayerCallbacks){.click_config_provider=click_config_provider});


    layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));
}

static void view_unload(Window *window) {

    text_layer_destroy(text_layer);
    scroll_layer_destroy(scroll_layer);
    window_destroy(window);
}

void view() {
    Window *view = window_create();
    window_set_window_handlers(view, (WindowHandlers) {
        .load = view_load,
        .unload = view_unload,
    });
    window_stack_push(view, true);
}


static void msg_received(DictionaryIterator *iter, void *context) {
    Tuple *title = dict_find(iter, KEY_TITLE);
    if (title) {
        Post p;
        p.index = dict_find(iter, KEY_INDEX)->value->int8;
        strncpy(p.title, title->value->cstring, 128);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "dada %s", p.title);
        snprintf(p.subtitle, sizeof(subtitle), "%i points · %i comments", dict_find(iter, KEY_POINTS)->value->int16, dict_find(iter, KEY_COMMENTS)->value->int16);
        posts[i++] = p;
        menu_layer_reload_data(menu_layer);
        layer_mark_dirty(menu_layer_get_layer(menu_layer));
    } else {
        if (dict_find(iter, KEY_START)) strcpy(summary, "");

        strcat(summary, dict_find(iter, KEY_SUMMARY)->value->cstring);

        if (dict_find(iter, KEY_END)) view();
    }
    title = NULL;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    if (i>0)
        return i;
    else
        return 1;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    if (i==0 && cell_index->row==0) {
        menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
    } else if (i>0) {
        menu_cell_basic_draw(ctx, cell_layer, posts[cell_index->row].title, posts[cell_index->row].subtitle, NULL);
    }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  menu_cell_basic_header_draw(ctx, cell_layer, "Hacker News");
}

void get(uint8_t i) {
    DictionaryIterator *it;
    app_message_outbox_begin(&it);
    Tuplet tuplet = TupletInteger(KEY_GET, i);
    dict_write_tuplet(it, &tuplet);
    dict_write_end(it);
    app_message_outbox_send();
}

void fetch() {
    DictionaryIterator *it;
    app_message_outbox_begin(&it);
    Tuplet tuplet = TupletInteger(KEY_FETCH, 0);
    dict_write_tuplet(it, &tuplet);
    dict_write_end(it);
    app_message_outbox_send();
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    summary[0] = *"\0";
    selected = cell_index->row;
    selected_index = posts[cell_index->row].index;
    get(selected_index);
}
void menu_long_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    readlater(posts[cell_index->row].index);
}

void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    menu_layer = menu_layer_create(layer_get_bounds(window_layer));

    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_rows = menu_get_num_rows_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback,
        .select_long_click = menu_long_select_callback
    });

    menu_layer_set_click_config_onto_window(menu_layer, window);
    layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
    fetch();
}

void window_unload(Window *window) {
    menu_layer_destroy(menu_layer);
}

void init(void) {
    app_message_register_inbox_received(msg_received);
    app_message_open(app_message_inbox_size_maximum(), 512);

    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
    app_event_loop();

    window_destroy(window);
}