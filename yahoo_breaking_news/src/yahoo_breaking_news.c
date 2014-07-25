#include "pebble.h"


#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 10


//
static int
  current_title_index = 0,
  current_summary_index = 0;

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

//this is intro windows display logo while loading the data
// static void intro_window_load(Window *window){

//   intro_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO);
//   intro_layer = bitmap_layer_create(CRect(10,0,200,60));
//   bitmap_layer_set_bitmap(intro_layer, intro_gbitmap);
//   layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(intro_layer));

//   progress_layer = help_text_layer_create(GRect(10,90,200,30), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentCenter);
//   text_layer_set_text(progress_layer, "Loading...");
//   layer_add_child(window_get_root_layer(window), text_layer_get_layer(progress_layer));
// }

// static void intro_window_unload(Window *window){
//   //Destroy layers
//   bitmap_layer_destroy(intro_layer);
//   gbitmap_destroy(intro_gbitmap);
//   text_layer_destroy(progress_layer);

//   window_destroy(window);

// }

// TextLayer* help_text_layer_create(GRect location, GColor color, GColor background, bool custom_font, GFont *g_font, const char *res_id, GTextAlignment alignment){

//   TextLayer *later = text_layer_create(location);
//   text_layer_set_text_color(layer,color);
//   text_layer_set_background_color(layer, background);
//   if(custom_font == true){
//     text_later_set_font(layer,g_font);
//   }
//   else{
//     text_layer_set_font(layer,fonts_get_system_font(res_id));
//   }
//   test_layer_set_text_alignment(layer, alignment);

//   return layer;
// }

/*static void help_app_message_open(int inbound_size, int outbound_size, AppMessageInboxReceived in_received_handler){
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  if(APP_MESSAGE_OPEN == false){
    app_message_open(inbound_size, outbound_size);
    APP_MESSAGE_OPEN = true;
    if(special_flag){
      help_applog("AppMessage opened.");
    }
  }

}

static void in_dropped_handler(AppMessageResult reason, void *context){
    if(special_flag == true){
      //output error message
        if(DEBUG == true)
        {
          help_interpret_message_result(reason);
        }

    }  
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context){
     if(special_flag == true){
      //output error message
      if(DEBUG == true)
        {
          help_interpret_message_result(reason);
        }
     }
}
*/

/*
 * Convenience app_log shortcut
 */
void help_applog(char* message)
{
  app_log(APP_LOG_LEVEL_INFO, "C", 0, message);
}

/*
 * Convert AppMessageResult to readable console output. Pebble SDK really needs this!
 */
void help_interpret_message_result(AppMessageResult app_message_error)
{
  if(app_message_error == APP_MSG_OK)
  {
    help_applog("APP_MSG_OK");
  } 

  else if(app_message_error == APP_MSG_SEND_TIMEOUT)
  {
    help_applog("APP_MSG_SEND_TIMEOUT");
  } 

  else if(app_message_error == APP_MSG_SEND_REJECTED)
  {
    help_applog("APP_MSG_SEND_REJECTED");
  }

  else if(app_message_error == APP_MSG_NOT_CONNECTED)
  {
    help_applog("APP_MSG_NOT_CONNECTED");
  }

  else if(app_message_error == APP_MSG_APP_NOT_RUNNING)
  {
    help_applog("APP_MSG_APP_NOT_RUNNING");
  }

  else if(app_message_error == APP_MSG_INVALID_ARGS)
  {
    help_applog("APP_MSG_INVALID_ARGS");
  }

  else if(app_message_error == APP_MSG_BUSY)
  {
    help_applog("APP_MSG_BUSY");
  }

  else if(app_message_error == APP_MSG_BUFFER_OVERFLOW)
  {
    help_applog("APP_MSG_BUFFER_OVERFLOW");
  }

  else if(app_message_error == APP_MSG_ALREADY_RELEASED)
  {
    help_applog("APP_MSG_ALREADY_RELEASED");
  }

  else if(app_message_error == APP_MSG_CALLBACK_ALREADY_REGISTERED)
  {
    help_applog("APP_MSG_CALLBACK_ALREADY_REGISTERED");
  }

  else if(app_message_error == APP_MSG_CALLBACK_NOT_REGISTERED)
  {
    help_applog("APP_MSG_CALLBACK_NOT_REGISTERED");
  }

  else if(app_message_error == APP_MSG_OUT_OF_MEMORY)
  {
    help_applog("APP_MSG_OUT_OF_MEMORY");
  }
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
  //window = window_create();
  
  // set the intro windows
  window = window_create();

   // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  //new intro page
  /*WindowHandlers handlers = {
        .load = intro_window_load,
        .unload = intro_window_unload
  };
  window_set_window_handlers(intro_window, (WindowHandlers) handlers);

  // increase the appMessage speed
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  help_app_message_open(
    app_message_inbox_size_maximum();
    app_message_outbox_size_maximum();
    (AppMessageInboxReceived)in_recv_handler
    );
*/
  window_stack_push(window, true /* Animated */);

  app_event_loop();

  window_destroy(window);
}