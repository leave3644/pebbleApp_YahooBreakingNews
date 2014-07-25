#include <pebble.h>

static Window *window;
static MenuLayer *menu_layer;

enum {
  KEY_HEADLINE = 0x0,
  KEY_UUID = 0x1,
  KEY_PUBLISHED_DATE = 0x2,
  KEY_SUMMARY = 0x3,
  KEY_GET = 0x4,
  KEY_FETCH = 0x5,
  KEY_INDEX = 0x6,
  KEY_START = 0x8,
  KEY_END = 0x9
};

static ScrollLayer *scroll_layer;
static TextLayer *text_layer;
static TextLayer *title_layer;
static GBitmap *yahoo_logo;

#define NUM_ITEMS 30

typedef struct Post {
    int index;
    char uuid[50];
    char headline[128];
    char publishedDate[20];
} Post;

Post posts[NUM_ITEMS];
int i=0;

static char publishedDate[] = "";

int selected = 0;
int selected_index = 0;
char summary[1024];
#define spacing 10

static void long_click_handler (ClickRecognizerRef recognizer, void *context){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Long Click Registered!");
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
    text_layer_set_text(title_layer, posts[selected].headline);
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
    Tuple *title = dict_find(iter, KEY_HEADLINE);
    Tuple *published = dict_find(iter, KEY_PUBLISHED_DATE);
    if (title) {
        Post p;
        p.index = dict_find(iter, KEY_INDEX)->value->int8;
        strncpy(p.headline, title->value->cstring, 128);
        snprintf(p.publishedDate, 128, "%s", dict_find(iter, KEY_PUBLISHED_DATE)->value->cstring);
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
        //GRect bounds = yahoo_logo->bounds;
        //graphics_draw_bitmap_in_rect(ctx, yahoo_logo, (GRect) { .origin = { 5, 5 }, .size = bounds.size });
    } else if (i>0) {
        menu_cell_basic_draw(ctx, cell_layer, posts[cell_index->row].headline, posts[cell_index->row].publishedDate, NULL);
    }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {

  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  menu_cell_basic_header_draw(ctx, cell_layer, "Yahoo! Headline News");
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

void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    menu_layer = menu_layer_create(layer_get_bounds(window_layer));

    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_rows = menu_get_num_rows_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback
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

    //yahoo_logo = gbitmap_create_with_resource(RESOURCE_ID_YAHOO_LOGO);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
    app_event_loop();

    //gbitmap_destroy(yahoo_logo);
    window_destroy(window);
}