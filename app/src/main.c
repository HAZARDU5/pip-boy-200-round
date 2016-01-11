#include <pebble.h>
#include <stdio.h>
#include <ctype.h>

//NOTE: make sure the values of these keys match the equivalents in appinfo.json or their values will be NULL!
#define CONFIG_KEY_BACKGROUND_COLOR (2)
#define CONFIG_KEY_ANIMATE_SECONDS false
  
static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *current_date_layer;
//static TextLayer *timephase_layer;

static GFont s_time_font;
//static GFont s_timephase_font;
static GFont s_date_font;

static BitmapLayer *clock_layer;
#if defined(PBL_ROUND)
    static Layer *s_canvas_layer;
#endif

/*static BitmapLayer *batt_layer;
static BitmapLayer *bt_layer;*/

static GBitmap *clock_bitmap;
static GBitmap *s_bitmap;
/*static GBitmap *batt_bitmap;
static GBitmap *bt_bitmap;*/

static bool animate_seconds = false;

// Create a long-lived buffer
static char buffer[] = "0000";
static char current_date_buffer[] = "00.00 000";
static char timephase_buffer[] = "00";

static char timeOfDay;

static int lastSecond = 0;

static void set_clock_bitmap_bw(char timeOfDay){
    if(clock_is_24h_style() == true){
        clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_bw);
    }else if(timeOfDay == 'A'){
        clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_bw_am);
    }else if(timeOfDay == 'P'){
        clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_bw_pm);
    }
}

static void set_clock_bitmap_rect(){
    if(clock_is_24h_style() == true){
        clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image);
    }else if(timeOfDay == 'A'){
        clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_am);
    }else if(timeOfDay == 'P'){
        clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_pm);
    }
}

static void set_clock_bitmap_round(){

    int background_color = persist_read_int(CONFIG_KEY_BACKGROUND_COLOR);

    if(clock_is_24h_style() == true){
        if(background_color == 1){
            clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_round_red);
        }else{
            clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_round);
        }
    }else if(timeOfDay == 'A'){
        if(background_color == 1){
            clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_round_am_red);
        }else{
            clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_round_am);
        }
    }else if(timeOfDay == 'P'){
        if(background_color == 1){
            clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_round_pm_red);
        }else{
            clock_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg_image_round_pm);
        }
    }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("0000"), "%H%M", tick_time);
    strftime(timephase_buffer, sizeof("00"), "  ", tick_time);
      
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("0000"), "%I%M", tick_time);
    strftime(timephase_buffer, sizeof("00"), "%p", tick_time);

  }

  //store last second
  lastSecond = tick_time->tm_sec;

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Second: %d", lastSecond);

  char newTimeOfDay = timephase_buffer[0];

  if(newTimeOfDay != timeOfDay){
    //time of day has changed, reload view
    timeOfDay = timephase_buffer[0];
    #if defined(PBL_BW)
        set_clock_bitmap_bw(timeOfDay);
    #elif defined(PBL_RECT)
        set_clock_bitmap_rect(timeOfDay);
    #elif defined(PBL_ROUND)
        set_clock_bitmap_round(timeOfDay);
    #endif

      //update clock layer with latest changes to clock bitmap so they are visible on screen
      bitmap_layer_set_bitmap(clock_layer, clock_bitmap);
  }else{
    timeOfDay = timephase_buffer[0];
  }

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "c2: %d", timeOfDay);

  //tod = strftime(timephase_buffer, sizeof("0"), "%p", tick_time);



  strftime(current_date_buffer, sizeof("000 00.00"), "%a %d.%m", tick_time);

  int i=0;
  char c;
  while (current_date_buffer[i])
  {
    c = current_date_buffer[i];
    current_date_buffer[i] = toupper((unsigned char)c);
    i++;
  }  //write to all text layers

  //gbitmap_destroy(bt_bitmap);

  /*if (bluetooth_connection_service_peek()) {
     bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bt_on);
  } else {
     bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bt_off);  
  }*/
  //bitmap_layer_set_bitmap(bt_layer, bt_bitmap);

  //BatteryChargeState state = battery_state_service_peek();
  //gbitmap_destroy(batt_bitmap);
  
  /*switch (state.charge_percent) {
    case 90: 
       batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt100);
       break;
    case 80: 
       batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt80);
       break;
    case 70: 
       batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt70);
       break;
    case 60: 
       batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt60);
       break;
    case 50: 
       batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt50);
       break;
    case 40: 
       batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt40);
       break;
    case 30: 
       batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt30);
       break;
    case 20: 
       batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt20);
       break;
    case 10: 
       if ( tick_time->tm_sec % 2 == 0)
          batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt00);
       else
          batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt10);       
       break;
    case 00: 
       if ( tick_time->tm_sec % 2 == 0)
          batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt00);
       else
          batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt10);       
       break;
    default: 
       batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt100);
       break;
  }*/
  
  //bitmap_layer_set_bitmap(batt_layer, batt_bitmap);

  text_layer_set_text(s_time_layer, buffer);
  //text_layer_set_text(timephase_layer, timephase_buffer);
  text_layer_set_text(current_date_layer, current_date_buffer);

  #if defined(PBL_ROUND)
    layer_mark_dirty(s_canvas_layer);
  #endif

}

static void update_text_color() {

    int background_color = persist_read_int(CONFIG_KEY_BACKGROUND_COLOR);

    #if defined(PBL_BW)
       text_layer_set_text_color(s_time_layer, GColorWhite);
       text_layer_set_text_color(current_date_layer, GColorWhite);
      #else
        if(background_color == 1){
            text_layer_set_text_color(s_time_layer, GColorRed);
            text_layer_set_text_color(current_date_layer, GColorRed);
        }else{
            text_layer_set_text_color(s_time_layer, GColorGreen);
            text_layer_set_text_color(current_date_layer, GColorGreen);
        }
      #endif
}

static void set_text_to_window() {

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating text bitmaps");

  //Time TextLayer 
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBASNEUE_60));

  int background_color = persist_read_int(CONFIG_KEY_BACKGROUND_COLOR);

  #if defined(PBL_RECT)
   s_time_layer = text_layer_create(GRect(18, 46, 108, 63));
  #elif defined(PBL_ROUND)
   s_time_layer = text_layer_create(GRect(36, 50, 108, 63));
  #endif


  text_layer_set_background_color(s_time_layer, GColorClear);



  text_layer_set_text(s_time_layer, "0000");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  //Time TextLayer 
  /*s_timephase_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBASNEUE_60));
  timephase_layer = text_layer_create(GRect(38, 50, 108, 63));
  text_layer_set_background_color(timephase_layer, GColorClear);
  text_layer_set_text_color(timephase_layer, GColorGreen);
  text_layer_set_text(timephase_layer, "00");
  text_layer_set_font(timephase_layer, s_timephase_font);
  text_layer_set_text_alignment(timephase_layer, GTextAlignmentCenter);*/
  
  // Create current date TextLayer 
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEBASNEUE_18));

  #if defined(PBL_RECT)
   current_date_layer = text_layer_create(GRect(12, 34, 120, 30));
  #elif defined(PBL_ROUND)
   current_date_layer = text_layer_create(GRect(29, 39, 120, 30));
  #endif


  text_layer_set_background_color(current_date_layer, GColorClear);




  text_layer_set_text(current_date_layer, "00.00 000");
  text_layer_set_font(current_date_layer, s_date_font);
  text_layer_set_text_alignment(current_date_layer, GTextAlignmentCenter);
}

static char* floatToString(char* buffer, int bufferSize, double number)
{
    char decimalBuffer[5];

    snprintf(buffer, bufferSize, "%d", (int)number);
    strcat(buffer, ".");

    snprintf(decimalBuffer, 5, "%02d", (int)((double)(number - (int)number) * (double)100));
    strcat(buffer, decimalBuffer);

    return buffer;
}

static void canvas_update_proc(Layer *this_layer, GContext *ctx) {

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "c2: %d", animate_seconds);

    if(animate_seconds == true){
        int background_color = persist_read_int(CONFIG_KEY_BACKGROUND_COLOR);

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating canvas");

        // Get a tm structure
          //time_t temp = time(NULL);
          //struct tm *tick_time = localtime(&temp);

          //strftime(buffer, sizeof("0000"), "%H%M", tick_time);
          //strftime(timephase_buffer, sizeof("00"), "  ", tick_time);

          GRect bounds = layer_get_bounds(this_layer);

          // Get the center of the screen (non full-screen)
          //GPoint center = GPoint(bounds.size.w / 2, (bounds.size.h / 2));

          //GOvalScaleModeFitCircle - Places the largest possible fully visible circle in the center of a rectangle.
          //GOvalScaleModeFillCircle - Places the smallest possible circle in the center of a rectangle so that the rectangle is fully inside the circle.

          //APP_LOG(APP_LOG_LEVEL_DEBUG, floatToString("00",sizeof("00"),(lastSecond/60.0)*360));
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "Percentage: %d", ceil((lastSecond/60.0)*360));

          graphics_context_set_stroke_width(ctx, 4);
          if(background_color == 1){
            graphics_context_set_stroke_color(ctx, GColorBulgarianRose); //GColorIslamicGreen
          }else{
            graphics_context_set_stroke_color(ctx, GColorDarkGreen); //GColorIslamicGreen
          }

          graphics_draw_arc(ctx, bounds, GOvalScaleModeFillCircle, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE((lastSecond/60.0)*360));



          // Draw the 'loop' of the 'P'
          //graphics_context_set_fill_color(ctx, GColorBlack);
          //graphics_fill_circle(ctx, center, 40);
          //graphics_context_set_fill_color(ctx, GColorWhite);
          //graphics_fill_circle(ctx, center, 35);

          // Draw the 'stalk'
          //graphics_context_set_fill_color(ctx, GColorBlack);
          //graphics_fill_rect(ctx, GRect(32, 40, 5, 100), 0, GCornerNone);

          //doSecondIndicatorUpdate = false;
    }else{
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "Skipping canvas update");
    }
}

static void update_background(){
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    strftime(timephase_buffer, sizeof("00"), "%p", tick_time);

    timeOfDay = timephase_buffer[0];
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "c2: %d", timeOfDay);

    #if defined(PBL_BW)

        set_clock_bitmap_bw(timeOfDay);

    #elif defined(PBL_RECT)

        set_clock_bitmap_rect(timeOfDay);

    #elif defined(PBL_ROUND)

        set_clock_bitmap_round(timeOfDay);

    #endif

    bitmap_layer_set_bitmap(clock_layer, clock_bitmap);
}

static void set_background(){
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    strftime(timephase_buffer, sizeof("00"), "%p", tick_time);

    timeOfDay = timephase_buffer[0];
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "c2: %d", timeOfDay);

    #if defined(PBL_BW)

        set_clock_bitmap_bw(timeOfDay);
        clock_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
        //s_canvas_layer = layer_create(GRect(0, 0, 144, 168));

    #elif defined(PBL_RECT)

        set_clock_bitmap_rect(timeOfDay);
        clock_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
        //s_canvas_layer = layer_create(GRect(0, 0, 144, 168));

    #elif defined(PBL_ROUND)

        set_clock_bitmap_round(timeOfDay);
        clock_layer = bitmap_layer_create(GRect(0, 0, 180, 180));
        s_canvas_layer = layer_create(GRect(26, 25, 130, 130));

        // Set LayerUpdateProc to draw bitmap
        layer_set_update_proc(s_canvas_layer, canvas_update_proc);

    #endif

    bitmap_layer_set_bitmap(clock_layer, clock_bitmap);
}

static void main_window_load(Window *window) {
  //ACTION: Create GBitmap, then set to created BitmapLayer

  if (persist_read_bool(CONFIG_KEY_ANIMATE_SECONDS)) {
    animate_seconds = persist_read_bool(CONFIG_KEY_ANIMATE_SECONDS);
  }

  set_background();



  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(clock_layer));

  #if defined(PBL_ROUND)
    //cast s_canvas_layer to BitmapLayer so we can use it with bitmap_layer_get_layer() as s_canvas_layer is a Layer primitive
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer((BitmapLayer *) s_canvas_layer));
  #endif



  //BATTERY: Create GBitmap, then set to created BitmapLayer
  /*batt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_batt100);
  batt_layer = bitmap_layer_create(GRect(0 ,0, 44, 168));
  bitmap_layer_set_bitmap(batt_layer, batt_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(batt_layer));*/
  
    //BATTERY: Create GBitmap, then set to created BitmapLayer
  /*bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bt_off);
  bt_layer = bitmap_layer_create(GRect( 98, 0, 46, 168));
  bitmap_layer_set_bitmap(bt_layer, bt_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bt_layer));*/
  
  set_text_to_window();

  update_text_color();
    
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(current_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(timephase_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}





static void main_window_unload(Window *window) {
    
  fonts_unload_custom_font(s_time_font);
  //fonts_unload_custom_font(s_timephase_font);
  fonts_unload_custom_font(s_date_font);

  //Destroy GBitmap
  gbitmap_destroy(clock_bitmap);
  /*gbitmap_destroy(batt_bitmap);
  gbitmap_destroy(bt_bitmap);*/

  //Destroy BitmapLayer
  bitmap_layer_destroy(clock_layer);
  /*bitmap_layer_destroy(batt_layer);
  bitmap_layer_destroy(bt_layer);*/

  #if defined(PBL_ROUND)
    // Destroy Layer
    layer_destroy(s_canvas_layer);
  #endif


  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  //text_layer_destroy(timephase_layer);
  text_layer_destroy(current_date_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
  
static void bt_handler(bool connected) {
  
}


static void in_received_handler(DictionaryIterator *received, void *context) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got data from config page");


  Tuple *background_color_t = dict_find(received, CONFIG_KEY_BACKGROUND_COLOR);
  Tuple *animate_seconds_t = dict_find(received, CONFIG_KEY_ANIMATE_SECONDS);

  if (background_color_t) {
      int background_color = background_color_t->value->int8;

      APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting background value %d",background_color);

      persist_write_int(CONFIG_KEY_BACKGROUND_COLOR, background_color_t->value->int8);

      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Got new background actual: %d",persist_read_int(CONFIG_KEY_BACKGROUND_COLOR));

      update_background();

      update_text_color();

      //set_text_to_window();
  }

//APP_LOG(APP_LOG_LEVEL_DEBUG, "Will set animate seconds? %d",animate_seconds_t->length);

if(animate_seconds_t == NULL){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Animate seconds value is null");
}


//animation_seconds_t Tuple seems to be empty every time
  if (animate_seconds_t) {

        APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting animate seconds value %s",animate_seconds_t->value->cstring);

        if (strcmp(animate_seconds_t->value->cstring, "T") == 0){
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting animate seconds value to true");
            persist_write_bool(CONFIG_KEY_ANIMATE_SECONDS, true);
            animate_seconds = true;
        }else{
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting animate seconds value to false");
            persist_write_bool(CONFIG_KEY_ANIMATE_SECONDS, false);
            animate_seconds = false;
        }

        APP_LOG(APP_LOG_LEVEL_DEBUG, "Got new animate seconds actual: %d",animate_seconds);
  }
}

void in_dropped_handler(AppMessageResult reason, void *ctx)
{
    app_log(APP_LOG_LEVEL_WARNING,
            __FILE__,
            __LINE__,
            "Message dropped, reason code %d",
            reason);
}

static void out_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Problem sending data: %d",reason);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  bluetooth_connection_service_subscribe(bt_handler);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_inbox_received(in_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

