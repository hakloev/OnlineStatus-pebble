#include <pebble.h>

#define KEY_BUSSTOP 0
#define KEY_BUSID 1
#define KEY_STOPTIME1 2
#define KEY_STOPTIME2 3


static Window *window;
static TextLayer *stopname_layer;
static TextLayer *stoptime_layer;
static TextLayer *stoptime1_layer;
static TextLayer *time_layer;
static TextLayer *date_layer;
static Layer *line_layer;

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
    // check accel here
    APP_LOG(APP_LOG_LEVEL_INFO, "Pebble was shaked");
    
    text_layer_set_text(stopname_layer, "");
    text_layer_set_text(stoptime_layer, "Laster info...");
    text_layer_set_text(stoptime1_layer, "");

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
}

static void line_layer_update(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void update_time(struct tm *tick_time) {
    if (!tick_time) {
        time_t temp = time(NULL);
        tick_time = localtime(&temp);
    }
    
    static char time_buffer[] = "00:00";
    static char date_buffer[] = "00.00.0000";

    if (clock_is_24h_style() == true) {
        strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
    } else {
        strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
    }

    text_layer_set_text(time_layer, time_buffer);

    // Date
    // TODO: Change once a day
    strftime(date_buffer, sizeof(date_buffer), "%d.%m.%Y", tick_time);
    text_layer_set_text(date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Tick happened!");
    if (units_changed & MINUTE_UNIT) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Minute ticked away!");
        update_time(tick_time);
    }

    if (tick_time->tm_min % 30 == 0) {
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);
        dict_write_uint8(iter, 0, 0);
        app_message_outbox_send();
    }
}

static void main_window_load(Window *window) {
    window_set_background_color(window, GColorBlack);

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Origin = coordinate of upper-lefthand corner of rectangle
    // Size = size of rectangle
    time_layer = text_layer_create((GRect) { .origin = { 0, -2 }, .size = { bounds.size.w, 49 } });
    text_layer_set_background_color(time_layer, GColorClear);
    text_layer_set_text_color(time_layer, GColorWhite);
    text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(time_layer));
    text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    
    date_layer = text_layer_create((GRect) { .origin = { 0, 53 }, .size = { bounds.size.w, 19 } });
    text_layer_set_background_color(date_layer, GColorClear);
    text_layer_set_text_color(date_layer, GColorWhite);
    text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
    text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(date_layer));

    update_time(NULL);

    GRect line = GRect(0, 83, 144, 2);
    line_layer = layer_create(line);
    layer_set_update_proc(line_layer, line_layer_update);
    layer_add_child(window_layer, line_layer);

    stopname_layer = text_layer_create((GRect) { .origin = { 0, 87 }, .size = { bounds.size.w, 42 } });
    text_layer_set_background_color(stopname_layer, GColorClear);
    text_layer_set_text_color(stopname_layer, GColorWhite);
    text_layer_set_text(stopname_layer, "");
    text_layer_set_text_alignment(stopname_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(stopname_layer));

    stoptime_layer = text_layer_create((GRect) { .origin = { 0, 110 }, .size = { bounds.size.w, 42 } });
    text_layer_set_background_color(stoptime_layer, GColorClear);
    text_layer_set_text_color(stoptime_layer, GColorWhite);
    text_layer_set_text(stoptime_layer, "Laster info...");
    text_layer_set_text_alignment(stoptime_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(stoptime_layer));

     stoptime1_layer = text_layer_create((GRect) { .origin = { 0, 130 }, .size = { bounds.size.w, 42 } });
    text_layer_set_background_color(stoptime1_layer, GColorClear);
    text_layer_set_text_color(stoptime1_layer, GColorWhite);
    text_layer_set_text(stoptime1_layer, "");
    text_layer_set_text_alignment(stoptime1_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(stoptime1_layer));

}

static void main_window_unload(Window *window) {
    text_layer_destroy(time_layer);
    text_layer_destroy(stopname_layer);
    text_layer_destroy(stoptime_layer);
    text_layer_destroy(stoptime1_layer);
    text_layer_destroy(date_layer);
    layer_destroy(line_layer);
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Message recieved from JS");
    
    static char stop_buffer[16];
    static char time_buffer[40];
    static char time_buffer1[40];
    
    Tuple *t = dict_read_first(iter);

    while (t != NULL) {
        switch (t->key) {
        case KEY_BUSSTOP:
            snprintf(stop_buffer, sizeof(stop_buffer), "%s", t->value->cstring);
            break;
        case KEY_BUSID:
            //snprintf(time_buffer, sizeof(time_buffer), "%s", t->value->cstring);
            break;
        case KEY_STOPTIME1:
            snprintf(time_buffer, sizeof(time_buffer), "%s", t->value->cstring);
            break;
        case KEY_STOPTIME2:
            snprintf(time_buffer1, sizeof(time_buffer1), "%s", t->value->cstring);
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
            break;
        }
        t = dict_read_next(iter);
    }
    text_layer_set_text(stopname_layer, stop_buffer);
    text_layer_set_font(stopname_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

    text_layer_set_text(stoptime_layer, time_buffer);
    text_layer_set_font(stoptime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

    text_layer_set_text(stoptime1_layer, time_buffer1);
    text_layer_set_font(stoptime1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox message dropped");
}

static void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason,void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox could not send");
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox sent successfully to JS");
}

static void init(void) {
    window = window_create();

    window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
    });
    const bool animated = true;
    window_stack_push(window, animated);

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

    accel_tap_service_subscribe(&accel_tap_handler);
}

static void deinit(void) {
    window_destroy(window);
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

    app_event_loop();
    deinit();
}
