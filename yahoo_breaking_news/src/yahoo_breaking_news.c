#include "pebble.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 10

static Window *window;

// Menu Layers: for list stories view
static SimpleMenuLayer *simple_menu_layer;
static SimpleMenuSection menu_sections[NUM_MENU_SECTIONS];
static SimpleMenuItem menu_items[NUM_MENU_ITEMS];

// Scrolling & Text Layers: for story view
static ScrollLayer *scroll_layer;
static TextLayer *text_layer;
static InverterLayer *inverter_layer;
static char scroll_text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam quam tellus, fermentu  m quis vulputate quis, vestibulum interdum sapien. Vestibulum lobortis pellentesque pretium. Quisque ultricies purus e  u orci convallis lacinia. Cras a urna mi. Donec convallis ante id dui dapibus nec ullamcorper erat egestas. Aenean a m  auris a sapien commodo lacinia. Sed posuere mi vel risus congue ornare. Curabitur leo nisi, euismod ut pellentesque se  d, suscipit sit amet lorem. Aliquam eget sem vitae sem aliquam ornare. In sem sapien, imperdiet eget pharetra a, lacin  ia ac justo. Suspendisse at ante nec felis facilisis eleifend.";
static const int vert_scroll_text_padding = 4;


static bool special_flag = false;

static int hit_count = 0;

static void show_scrolling_detail_layer(){
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);
    GRect max_text_bounds = GRect(0, 0, bounds.size.w, 2000);
    // Initialize the simple menu layer
    scroll_layer = scroll_layer_create(bounds);
    scroll_layer_set_click_config_onto_window(scroll_layer, window);

    text_layer = text_layer_create(max_text_bounds);
    text_layer_set_text(text_layer, scroll_text);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

    GSize max_size = text_layer_get_content_size(text_layer);
    text_layer_set_size(text_layer, max_size);
    scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, max_size.h + vert_scroll_text_padding));

    scroll_layer_add_child(scroll_layer, text_layer_get_layer(text_layer));
    inverter_layer = inverter_layer_create(GRect(0, 28, bounds.size.w, 28));
    scroll_layer_add_child(scroll_layer, inverter_layer_get_layer(inverter_layer));

    layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));
}

// You can capture when the user selects a menu icon with a menu item select callback
static void menu_select_callback(int index, void *ctx) {
  // Here we just change the subtitle to a literal string
  menu_items[index].subtitle = "You've hit select here!";
  // Mark the layer to be updated
  layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
}
// You can specify special callbacks to differentiate functionality of a menu item
static void special_select_callback(int index, void *ctx) {
  // Of course, you can do more complicated things in a menu item select callback
  // Here, we have a simple toggle
  special_flag = !special_flag;

  SimpleMenuItem *menu_item = &menu_items[index];

  if (special_flag) {
    menu_item->subtitle = "Okay, it's not so special.";
  } else {
    menu_item->subtitle = "Well, maybe a little.";
  }

  if (++hit_count > 5) {
    menu_item->title = "Very Special Item";
  }

  // Mark the layer to be updated
  layer_mark_dirty(simple_menu_layer_get_layer(simple_menu_layer));
  show_scrolling_detail_layer();
}



// This initializes the menu upon window load
static void window_load(Window *window) {
  // We'll have to load the icon before we can use it
  //menu_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_1);

  // Although we already defined NUM_FIRST_MENU_ITEMS, you can define
  // an int as such to easily change the order of menu items later
  int num_a_items = 0;

  // This is an example of how you'd set a simple menu item
  menu_items[num_a_items++] = (SimpleMenuItem){
    // You should give each menu item a title and callback
    .title = "First Item",
    .callback = menu_select_callback,
  };
  // The menu items appear in the order saved in the menu items array
  menu_items[num_a_items++] = (SimpleMenuItem){
    .title = "Second Item",
    // You can also give menu items a subtitle
    .subtitle = "Here's a subtitle",
    .callback = menu_select_callback,
  };
  menu_items[num_a_items++] = (SimpleMenuItem){
    .title = "Third Item",
    .subtitle = "This has an icon",
    .callback = menu_select_callback
  };

  // This initializes the second section
  menu_items[0] = (SimpleMenuItem){
    .title = "Special Item",
    // You can use different callbacks for your menu items
    .callback = special_select_callback,
  };

  // Bind the menu items to the corresponding menu sections
  menu_sections[0] = (SimpleMenuSection){
    .num_items = NUM_MENU_ITEMS,
    .items = menu_items,
  };

  // Now we prepare to initialize the simple menu layer
  // We need the bounds to specify the simple menu layer's viewport size
  // In this case, it'll be the same as the window's
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Initialize the simple menu layer
  simple_menu_layer = simple_menu_layer_create(bounds, window, menu_sections, NUM_MENU_SECTIONS, NULL);

  // Add it to the window for display
  layer_add_child(window_layer, simple_menu_layer_get_layer(simple_menu_layer));
}

// Deinitialize resources on window unload that were initialized on window load
void window_unload(Window *window) {
  simple_menu_layer_destroy(simple_menu_layer);
}

int main(void) {
  window = window_create();

  // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true /* Animated */);

  app_event_loop();

  window_destroy(window);
}