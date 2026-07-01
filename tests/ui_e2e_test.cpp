#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <atomic>

#define main mdp_gui_app_main
#include "../mdp_gui.cpp"
#undef main

using namespace std;

namespace {

atomic<bool> fake_capture_started{false};
atomic<bool> fake_capture_can_finish{false};

void fail(const string& message) {
    cerr << "TEST FAILURE: " << message << '\n';
    exit(1);
}

void expect(bool condition, const string& message) {
    if (!condition) {
        fail(message);
    }
}

void expect_equal(const string& actual, const string& expected, const string& message) {
    if (actual != expected) {
        fail(message + " | actual=" + actual + " expected=" + expected);
    }
}

void pump_events() {
    while (g_main_context_iteration(nullptr, FALSE)) {
    }
}

bool wait_for(const function<bool()>& predicate, int timeout_ms = 3000) {
    const gint64 deadline = g_get_monotonic_time() + static_cast<gint64>(timeout_ms) * 1000;

    while (g_get_monotonic_time() < deadline) {
        pump_events();
        if (predicate()) {
            return true;
        }
        g_usleep(10000);
    }

    pump_events();
    return predicate();
}

void wait_until_capture_can_finish() {
    const gint64 deadline = g_get_monotonic_time()+ 3000 * 1000;

    while (!fake_capture_can_finish.load() && g_get_monotonic_time() < deadline) {
        g_usleep(10000);
    }
}

string label_text(GtkWidget* widget) {
    return gtk_label_get_text(GTK_LABEL(widget));
}

string entry_text(GtkWidget* widget) {
    return gtk_entry_get_text(GTK_ENTRY(widget));
}

string button_label(GtkWidget* widget) {
    return gtk_button_get_label(GTK_BUTTON(widget));
}

void click(GtkWidget* widget) {
    g_signal_emit_by_name(widget, "clicked");
    pump_events();
}

vector<unsigned char> fake_capture_success(unsigned int seconds) {
    expect(seconds == 2, "microphone capture duration must stay at 2 seconds");

    fake_capture_started.store(true);
    wait_until_capture_can_finish();

    return vector<unsigned char>(512, 42);
}

vector<unsigned char> fake_capture_failure(unsigned int seconds) {
    expect(seconds == 2, "microphone capture duration must stay at 2 seconds");

    fake_capture_started.store(true);
    wait_until_capture_can_finish();

    throw runtime_error(g_text.microphone_capture_error);
}

void create_ui(bool french = false) {
    fake_capture_started.store(false);
    fake_capture_can_finish.store(false);

    g_is_french = french;
    g_text = french ? french_text() : english_text();
    build_application_ui();
    gtk_widget_show_all(window_widget);
    gtk_widget_hide(recording_box);
    pump_events();
}

void destroy_ui() {
    reset_capture_microphone_noise_for_tests();
    destroy_application_ui();
    pump_events();
}

void test_initial_english_ui_state() {
    create_ui(false);

    expect_equal(label_text(title_label), "Password Generator", "title label must use English text");
    expect_equal(button_label(generate_button), "Generate", "generate button must use English label");
    expect_equal(button_label(copy_button), "Copy", "copy button must use English label");
    expect_equal(entry_text(length_entry), "32", "default password length must be 32");
    expect(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lower_check)) != 0, "lowercase must be enabled by default");
    expect(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(upper_check)) != 0, "uppercase must be enabled by default");
    expect(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(digits_check)) != 0, "digits must be enabled by default");
    expect(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(symbols_check)) != 0, "symbols must be enabled by default");
    expect(!gtk_widget_get_visible(recording_box), "recording box must stay hidden before generation");

    destroy_ui();
}

void test_language_switch_updates_ui_text() {
    create_ui(false);

    gtk_combo_box_set_active(GTK_COMBO_BOX(language_combo), 0);
    pump_events();
    expect_equal(label_text(title_label), "Generateur de mots de passe", "title label must switch to French");
    expect_equal(button_label(generate_button), "Generer", "generate button must switch to French");

    gtk_combo_box_set_active(GTK_COMBO_BOX(language_combo), 1);
    pump_events();
    expect_equal(label_text(title_label), "Password Generator", "title label must switch back to English");
    expect_equal(button_label(generate_button), "Generate", "generate button must switch back to English");

    destroy_ui();
}

void test_invalid_length_shows_bilingual_error() {
    create_ui(false);

    gtk_entry_set_text(GTK_ENTRY(length_entry), "0");
    click(generate_button);
    expect_equal(label_text(status_label), g_text.length_positive_error, "non-positive length must show the UI error");

    destroy_ui();
}

void test_copy_without_password_shows_status() {
    create_ui(false);

    click(copy_button);
    expect_equal(label_text(status_label), g_text.nothing_to_copy_label, "copy without password must update the status");

    destroy_ui();
}

void test_successful_generation_updates_ui_end_to_end() {
    create_ui(false);
    set_capture_microphone_noise_for_tests(fake_capture_success);

    gtk_entry_set_text(GTK_ENTRY(length_entry), "24");
    click(generate_button);

    expect(wait_for([] {
        return fake_capture_started.load()
            && !gtk_widget_get_sensitive(generate_button)
            && !gtk_widget_get_sensitive(copy_button)
            && gtk_widget_get_visible(recording_box);
    }, 1000), "recording UI must be visible while capture is running");

    expect_equal(label_text(status_label), g_text.capturing_label, "status must show the capture message during capture");

    fake_capture_can_finish.store(true);

    expect(wait_for([] {
        return gtk_widget_get_sensitive(generate_button) && entry_text(result_entry).size() == 24;
    }), "generation must finish and re-enable the UI");

    expect_equal(label_text(status_label), g_text.generated_label, "successful generation must update the status");
    expect(!entry_text(result_entry).empty(), "successful generation must fill the result entry");
    expect(!gtk_widget_get_visible(recording_box), "recording box must hide after generation");

    click(copy_button);
    expect_equal(label_text(status_label), g_text.copied_label, "copy after generation must update the status");

    destroy_ui();
}

void test_generation_failure_restores_ui_state() {
    create_ui(false);
    set_capture_microphone_noise_for_tests(fake_capture_failure);

    gtk_entry_set_text(GTK_ENTRY(length_entry), "16");
    click(generate_button);

    expect(wait_for([] {
        return fake_capture_started.load()
            && !gtk_widget_get_sensitive(generate_button)
            && gtk_widget_get_visible(recording_box);
    }, 1000), "recording UI must be visible while failing capture is running");

    fake_capture_can_finish.store(true);

    expect(wait_for([] {
        return gtk_widget_get_sensitive(generate_button);
    }), "failing generation must re-enable the UI");

    expect_equal(label_text(status_label), g_text.microphone_capture_error, "capture failure must surface the UI error");
    expect(entry_text(result_entry).empty(), "failed generation must not leave a password behind");
    expect(!gtk_widget_get_visible(recording_box), "recording box must hide after a failure");

    destroy_ui();
}

}  // namespace

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    test_initial_english_ui_state();
    test_language_switch_updates_ui_text();
    test_invalid_length_shows_bilingual_error();
    test_copy_without_password_shows_status();
    test_successful_generation_updates_ui_end_to_end();
    test_generation_failure_restores_ui_state();

    cout << "All UI end-to-end tests passed." << '\n';
    return 0;
}
