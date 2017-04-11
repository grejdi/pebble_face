#include <pebble.h>

static Window *s_main_window;

static TextLayer *s_step_layer;
static TextLayer *s_silent_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_bluetooth_layer;

static int s_battery_level;
static bool s_bluetooth_connected = true;
static int s_step_count = 0, s_step_average = 0;

// Is step data available?
bool step_data_is_available() {
    return HealthServiceAccessibilityMaskAvailable &
        health_service_metric_accessible(HealthMetricStepCount,
            time_start_of_today(), time(NULL));
}

// Todays current step count
static void get_step_count() {
    s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
}

// Average daily step count for this time of day
static void get_step_average() {
    const time_t start = time_start_of_today();
    const time_t end = time(NULL);
    s_step_average = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily);
}

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Create step Layer
    s_step_layer = text_layer_create(
        GRect(5, 0, bounds.size.w - 5 - 50, 25));
    // Style the text
    text_layer_set_background_color(s_step_layer, GColorWhite);
    text_layer_set_text_color(s_step_layer, GColorBlack);
    text_layer_set_text(s_step_layer, "Loading...");
    text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_step_layer));

    // Create silent Layer
    s_silent_layer = text_layer_create(
        GRect(bounds.size.w - 50, 0, 50, 25));
    // Style the text
    text_layer_set_background_color(s_silent_layer, GColorWhite);
    text_layer_set_text_color(s_silent_layer, GColorBlack);
    if (quiet_time_is_active()) {
        text_layer_set_text(s_silent_layer, "Silent");
        text_layer_set_background_color(s_silent_layer, GColorRed);
        text_layer_set_text_color(s_silent_layer, GColorWhite);
    }
    text_layer_set_font(s_silent_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_silent_layer, GTextAlignmentCenter);
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_silent_layer));

    // Create date layer
    s_date_layer = text_layer_create(
      GRect(0, 25, bounds.size.w, 25));
    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_date_layer, GColorWhite);
    text_layer_set_text_color(s_date_layer, GColorBlack);
    text_layer_set_text(s_date_layer, "00/00");
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Create time layer
    s_time_layer = text_layer_create(
      GRect(0, 50, bounds.size.w, 45));
    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer, GColorWhite);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_text(s_time_layer, "00:00");
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // Create weather Layer
    s_weather_layer = text_layer_create(
        GRect(0, 95, bounds.size.w, 28));
    // Style the text
    text_layer_set_background_color(s_weather_layer, GColorWhite);
    text_layer_set_text_color(s_weather_layer, GColorBlack);
    text_layer_set_text(s_weather_layer, "Loading...");
    text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));

    // Create battery Layer
    s_battery_layer = text_layer_create(
        GRect(0, 143, bounds.size.w / 3, 25));
    // Style the text
    text_layer_set_background_color(s_battery_layer, GColorWhite);
    text_layer_set_text_color(s_battery_layer, GColorBlack);
    text_layer_set_text(s_battery_layer, "Loading...");
    text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));

    // Create bluetooth Layer
    s_bluetooth_layer = text_layer_create(
        GRect(bounds.size.w / 3, 143, (bounds.size.w * 2) / 3, 25));
    // Style the text
    text_layer_set_background_color(s_bluetooth_layer, GColorWhite);
    text_layer_set_text_color(s_bluetooth_layer, GColorBlack);
    text_layer_set_text(s_bluetooth_layer, "Connected");
    text_layer_set_font(s_bluetooth_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_bluetooth_layer, GTextAlignmentCenter);
    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_bluetooth_layer));

}

static void main_window_unload(Window *window) {
  // Destroy layers layer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_bluetooth_layer);
}

// time handle
static void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

static void update_date() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), "%m/%d", tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_date_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();

    // Get weather update every 30 minutes
    if(tick_time->tm_min % 30 == 0) {
        // Begin dictionary
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);

        // Add a key-value pair
        dict_write_uint8(iter, 0, 0);

        // Send the message!
        app_message_outbox_send();
    }

    // get the date
    if (tick_time->tm_hour == 0 && tick_time->tm_min == 0) {
        update_date();
    }
}

static void health_handler(HealthEventType event, void *context) {
    if(event != HealthEventSleepUpdate) {
        get_step_count();
        get_step_average();

        static char s_step_buffer[16];
        snprintf(s_step_buffer, sizeof(s_step_buffer), "%d / %d", s_step_count, s_step_average);
        text_layer_set_text(s_step_layer, s_step_buffer);
    }
}

// send app messages
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    // Store incoming information
    static char temperature_buffer[8];
    static char conditions_buffer[32];
    static char weather_layer_buffer[32];

    // Read tuples for data
    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

    // If all data is available, use it
    if(temp_tuple && conditions_tuple) {
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%s", temp_tuple->value->cstring);
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

        // Assemble full string and display
        snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
        text_layer_set_text(s_weather_layer, weather_layer_buffer);
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void battery_callback(BatteryChargeState state) {
    s_battery_level = state.charge_percent;

    static char battery_buffer[8];
    snprintf(battery_buffer, sizeof(battery_buffer), "%d%%", state.charge_percent);
    text_layer_set_text(s_battery_layer, battery_buffer);
}

static void bluetooth_callback(bool connected) {
    if(s_bluetooth_connected && !connected) {
        text_layer_set_text(s_bluetooth_layer, "Disconnected");
        text_layer_set_background_color(s_bluetooth_layer, GColorRed);
        text_layer_set_text_color(s_bluetooth_layer, GColorWhite);
        if (!quiet_time_is_active()) {
            vibes_long_pulse();
        }

        s_bluetooth_connected = false;
    } else if (!s_bluetooth_connected && connected) {
        text_layer_set_text(s_bluetooth_layer, "Connected");
        text_layer_set_background_color(s_bluetooth_layer, GColorWhite);
        text_layer_set_text_color(s_bluetooth_layer, GColorBlack);
        if (!quiet_time_is_active()) {
            vibes_double_pulse();
        }

        s_bluetooth_connected = true;
    }
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

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    // Make sure the time is displayed from the start
    update_time();
    // Make sure the date is displayed from the start
    update_date();

    // Register callbacks for AppMessage
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    // Open AppMessage
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

    // Register for battery level updates
    battery_state_service_subscribe(battery_callback);
    // Ensure battery level is displayed from the start
    battery_callback(battery_state_service_peek());

    // Register for Bluetooth connection updates
    connection_service_subscribe((ConnectionHandlers) {
        .pebble_app_connection_handler = bluetooth_callback
    });
    // Show the correct state of the BT connection from the start
    bluetooth_callback(connection_service_peek_pebble_app_connection());

    if (step_data_is_available()) {
        health_service_events_subscribe(health_handler, NULL);
    }
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
