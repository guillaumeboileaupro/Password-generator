#include <gtk/gtk.h>
#include <alsa/asoundlib.h>
#include "password_core.h"

#include <cerrno>
#include <cstdlib>
#include <glib/gstdio.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <thread>

using namespace std;

GtkWidget* length_entry;
GtkWidget* lower_check;
GtkWidget* upper_check;
GtkWidget* digits_check;
GtkWidget* symbols_check;
GtkWidget* result_entry;
GtkWidget* status_label;
GtkWidget* window_widget;
GtkWidget* title_label;
GtkWidget* language_label;
GtkWidget* language_combo;
GtkWidget* options_label;
GtkWidget* generate_button;
GtkWidget* copy_button;
GtkWidget* recording_box;
GtkWidget* recording_spinner;
GtkWidget* recording_label;

vector<unsigned char> capture_microphone_noise(unsigned int seconds = 2);

struct UiText {
    const char* app_name;
    const char* window_title;
    const char* title;
    const char* length_placeholder;
    const char* options_label;
    const char* lower_label;
    const char* upper_label;
    const char* digits_label;
    const char* symbols_label;
    const char* generate_label;
    const char* result_placeholder;
    const char* copy_label;
    const char* ready_label;
    const char* language_label;
    const char* language_french;
    const char* language_english;
    const char* capturing_label;
    const char* length_positive_error;
    const char* length_max_error;
    const char* generated_label;
    const char* nothing_to_copy_label;
    const char* copied_label;
    const char* microphone_open_error;
    const char* microphone_config_error;
    const char* microphone_capture_error;
};

const size_t kMaxPasswordLength = 1024;
const char* kLocalIconPath = "mdp-logo.png";
UiText g_text;
bool g_is_french = false;
using CaptureMicrophoneNoiseFn = vector<unsigned char> (*)(unsigned int);
CaptureMicrophoneNoiseFn g_capture_microphone_noise_fn = capture_microphone_noise;

void on_language_changed(GtkComboBox* combo, gpointer);

UiText english_text() {
    return {
        "Password Generator",
        "Password Generator",
        "Password Generator",
        "Password length",
        "Character types",
        "Lowercase a-z",
        "Uppercase A-Z",
        "Digits 0-9",
        "Symbols",
        "Generate",
        "Generated password",
        "Copy",
        "Ready",
        "Language",
        "French",
        "English",
        "Capturing microphone noise for 2 seconds...",
        "Password length must be positive.",
        "Maximum password length is 1024.",
        "Password generated from microphone noise.",
        "No password to copy.",
        "Password copied to clipboard.",
        "Unable to open the microphone.",
        "Unable to configure the microphone.",
        "Error while capturing microphone noise."
    };
}

UiText french_text() {
    return {
        "Generateur de mots de passe",
        "Generateur de mots de passe",
        "Generateur de mots de passe",
        "Longueur du mot de passe",
        "Types de caracteres",
        "Minuscules a-z",
        "Majuscules A-Z",
        "Chiffres 0-9",
        "Symboles",
        "Generer",
        "Le mot de passe apparaitra ici",
        "Copier",
        "Pret",
        "Langue",
        "Francais",
        "Anglais",
        "Capture du bruit du microphone pendant 2 secondes...",
        "La longueur doit etre positive.",
        "La longueur maximale est 1024.",
        "Mot de passe genere a partir du bruit du microphone.",
        "Aucun mot de passe a copier.",
        "Mot de passe copie dans le presse-papiers.",
        "Impossible d'ouvrir le microphone.",
        "Impossible de configurer le microphone.",
        "Erreur pendant la capture du microphone."
    };
}

UiText detect_ui_text() {
    const char* lang = getenv("LANG");
    if (lang != nullptr && lang[0] == 'f' && lang[1] == 'r') {
        g_is_french = true;
        return french_text();
    }
    g_is_french = false;
    return english_text();
}

void apply_ui_text() {
    static bool application_name_set = false;
    if (!application_name_set) {
        g_set_application_name(g_text.app_name);
        application_name_set = true;
    }
    gtk_window_set_title(GTK_WINDOW(window_widget), g_text.window_title);
    gtk_label_set_text(GTK_LABEL(title_label), g_text.title);
    gtk_label_set_text(GTK_LABEL(language_label), g_text.language_label);
    gtk_label_set_text(GTK_LABEL(options_label), g_text.options_label);
    gtk_button_set_label(GTK_BUTTON(lower_check), g_text.lower_label);
    gtk_button_set_label(GTK_BUTTON(upper_check), g_text.upper_label);
    gtk_button_set_label(GTK_BUTTON(digits_check), g_text.digits_label);
    gtk_button_set_label(GTK_BUTTON(symbols_check), g_text.symbols_label);
    gtk_button_set_label(GTK_BUTTON(generate_button), g_text.generate_label);
    gtk_button_set_label(GTK_BUTTON(copy_button), g_text.copy_label);
    gtk_entry_set_placeholder_text(GTK_ENTRY(length_entry), g_text.length_placeholder);
    gtk_entry_set_placeholder_text(GTK_ENTRY(result_entry), g_text.result_placeholder);

    g_signal_handlers_block_by_func(language_combo, reinterpret_cast<gpointer>(on_language_changed), NULL);
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(language_combo));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(language_combo), g_text.language_french);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(language_combo), g_text.language_english);
    gtk_combo_box_set_active(GTK_COMBO_BOX(language_combo), g_is_french ? 0 : 1);
    g_signal_handlers_unblock_by_func(language_combo, reinterpret_cast<gpointer>(on_language_changed), NULL);

    gtk_label_set_text(GTK_LABEL(recording_label), g_text.capturing_label);
}

void on_language_changed(GtkComboBox* combo, gpointer) {
    const int active = gtk_combo_box_get_active(combo);
    g_is_french = (active == 0);
    g_text = g_is_french ? french_text() : english_text();
    apply_ui_text();
}

vector<unsigned char> capture_microphone_noise(unsigned int seconds) {
    snd_pcm_t* handle;
    const char* device = "default";

    unsigned int sample_rate = 44100;
    int channels = 1;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

    int err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        throw runtime_error(g_text.microphone_open_error);
    }

    err = snd_pcm_set_params(
        handle,
        format,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        channels,
        sample_rate,
        1,
        500000
    );

    if (err < 0) {
        snd_pcm_close(handle);
        throw runtime_error(g_text.microphone_config_error);
    }

    size_t total_samples = sample_rate * seconds;
    vector<int16_t> buffer(total_samples);
    size_t samples_read = 0;

    while (samples_read < total_samples) {
        snd_pcm_sframes_t frames = snd_pcm_readi(
            handle,
            buffer.data() + samples_read,
            total_samples - samples_read
        );

        if (frames == -EPIPE) {
            snd_pcm_prepare(handle);
            continue;
        }

        if (frames < 0) {
            snd_pcm_close(handle);
            throw runtime_error(g_text.microphone_capture_error);
        }

        samples_read += frames;
    }

    snd_pcm_close(handle);

    return vector<unsigned char>(
        reinterpret_cast<unsigned char*>(buffer.data()),
        reinterpret_cast<unsigned char*>(buffer.data()) + buffer.size() * sizeof(int16_t)
    );
}

void set_capture_microphone_noise_for_tests(CaptureMicrophoneNoiseFn capture_fn) {
    g_capture_microphone_noise_fn = capture_fn != nullptr ? capture_fn : capture_microphone_noise;
}

void reset_capture_microphone_noise_for_tests() {
    g_capture_microphone_noise_fn = capture_microphone_noise;
}

struct GenerationRequest {
    int length;
    PasswordOptions options;
};

struct GenerationResult {
    bool success;
    string password;
    string message;
};

void set_recording_ui(bool is_recording) {
    gtk_widget_set_sensitive(generate_button, !is_recording);
    gtk_widget_set_sensitive(copy_button, !is_recording);
    gtk_widget_set_sensitive(length_entry, !is_recording);
    gtk_widget_set_sensitive(lower_check, !is_recording);
    gtk_widget_set_sensitive(upper_check, !is_recording);
    gtk_widget_set_sensitive(digits_check, !is_recording);
    gtk_widget_set_sensitive(symbols_check, !is_recording);
    gtk_widget_set_sensitive(language_combo, !is_recording);

    if (is_recording) {
        gtk_widget_show(recording_box);
        gtk_widget_show(recording_spinner);
        gtk_widget_show(recording_label);
        gtk_spinner_start(GTK_SPINNER(recording_spinner));
    } else {
        gtk_spinner_stop(GTK_SPINNER(recording_spinner));
        gtk_widget_hide(recording_box);
    }
}

gboolean finish_generation_on_ui_thread(gpointer data) {
    unique_ptr<GenerationResult> result(static_cast<GenerationResult*>(data));

    set_recording_ui(false);

    if (result->success) {
        gtk_entry_set_text(GTK_ENTRY(result_entry), result->password.c_str());
    }

    gtk_label_set_text(GTK_LABEL(status_label), result->message.c_str());

    return FALSE;
}

void run_generation_worker(GenerationRequest request) {
    auto result = make_unique<GenerationResult>();

    try {
        const vector<unsigned char> microphone_noise = g_capture_microphone_noise_fn(2);
        const vector<unsigned char> seed = sha256_bytes(microphone_noise);

        result->password = generate_password_from_seed(request.length, request.options, seed);
        result->message = g_text.generated_label;
        result->success = true;
    } catch (const exception& e) {
        result->password = "";
        result->message = e.what();
        result->success = false;
    }

    g_idle_add(finish_generation_on_ui_thread, result.release());
}

void on_generate_clicked(GtkWidget*, gpointer) {
    try {
        const char* length_text = gtk_entry_get_text(GTK_ENTRY(length_entry));
        int length = stoi(length_text);

        if (length <= 0) {
            throw runtime_error(g_text.length_positive_error);
        }
        if (length > static_cast<int>(kMaxPasswordLength)) {
            throw runtime_error(g_text.length_max_error);
        }

        const PasswordOptions options{
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lower_check)) != 0,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(upper_check)) != 0,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(digits_check)) != 0,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(symbols_check)) != 0
        };

        gtk_label_set_text(GTK_LABEL(status_label), g_text.capturing_label);
        gtk_label_set_text(GTK_LABEL(recording_label), g_text.capturing_label);

        set_recording_ui(true);

        GenerationRequest request{length, options};
        thread(run_generation_worker, request).detach();

    } catch (const exception& e) {
        gtk_label_set_text(GTK_LABEL(status_label), e.what());
    }
}

void on_copy_clicked(GtkWidget*, gpointer) {
    const char* password = gtk_entry_get_text(GTK_ENTRY(result_entry));
    if (password == nullptr || password[0] == '\0') {
        gtk_label_set_text(GTK_LABEL(status_label), g_text.nothing_to_copy_label);
        return;
    }

    GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, password, -1);

    gtk_label_set_text(GTK_LABEL(status_label), g_text.copied_label);
}

void on_window_destroy(GtkWidget*, gpointer) {
    if (gtk_main_level() > 0) {
        gtk_main_quit();
    }
}

void build_application_ui() {
    g_set_prgname("mdp-generator");

    if (g_file_test(kLocalIconPath, G_FILE_TEST_EXISTS)) {
        gtk_window_set_default_icon_from_file(kLocalIconPath, nullptr);
    }

    window_widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window_widget), 520, 360);
    gtk_container_set_border_width(GTK_CONTAINER(window_widget), 15);

    if (g_file_test(kLocalIconPath, G_FILE_TEST_EXISTS)) {
        gtk_window_set_icon_from_file(GTK_WINDOW(window_widget), kLocalIconPath, nullptr);
    } else {
        gtk_window_set_icon_name(GTK_WINDOW(window_widget), "mdp-logo");
    }

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window_widget), box);

    title_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(box), title_label, FALSE, FALSE, 5);

    GtkWidget* language_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(box), language_box, FALSE, FALSE, 0);

    language_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(language_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(language_box), language_label, FALSE, FALSE, 0);

    language_combo = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(language_box), language_combo, FALSE, FALSE, 0);

    length_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(length_entry), "32");
    gtk_box_pack_start(GTK_BOX(box), length_entry, FALSE, FALSE, 5);

    options_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(options_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(box), options_label, FALSE, FALSE, 0);

    lower_check = gtk_check_button_new_with_label("");
    upper_check = gtk_check_button_new_with_label("");
    digits_check = gtk_check_button_new_with_label("");
    symbols_check = gtk_check_button_new_with_label("");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lower_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(upper_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(digits_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(symbols_check), TRUE);

    gtk_box_pack_start(GTK_BOX(box), lower_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), upper_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), digits_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), symbols_check, FALSE, FALSE, 0);

    generate_button = gtk_button_new_with_label("");
    gtk_box_pack_start(GTK_BOX(box), generate_button, FALSE, FALSE, 10);

    recording_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_no_show_all(recording_box, TRUE);

    recording_spinner = gtk_spinner_new();
    gtk_box_pack_start(GTK_BOX(recording_box), recording_spinner, FALSE, FALSE, 0);

    recording_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(recording_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(recording_box), recording_label, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(box), recording_box, FALSE, FALSE, 5);

    result_entry = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(result_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(box), result_entry, FALSE, FALSE, 5);

    copy_button = gtk_button_new_with_label("");
    gtk_box_pack_start(GTK_BOX(box), copy_button, FALSE, FALSE, 5);

    status_label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(status_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(status_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 5);

    apply_ui_text();

    g_signal_connect(language_combo, "changed", G_CALLBACK(on_language_changed), NULL);
    g_signal_connect(generate_button, "clicked", G_CALLBACK(on_generate_clicked), NULL);
    g_signal_connect(copy_button, "clicked", G_CALLBACK(on_copy_clicked), NULL);
    g_signal_connect(window_widget, "destroy", G_CALLBACK(on_window_destroy), NULL);
}

void destroy_application_ui() {
    if (window_widget != nullptr) {
        gtk_widget_destroy(window_widget);
    }

    length_entry = nullptr;
    lower_check = nullptr;
    upper_check = nullptr;
    digits_check = nullptr;
    symbols_check = nullptr;
    result_entry = nullptr;
    status_label = nullptr;
    window_widget = nullptr;
    title_label = nullptr;
    language_label = nullptr;
    language_combo = nullptr;
    options_label = nullptr;
    generate_button = nullptr;
    copy_button = nullptr;
    recording_box = nullptr;
    recording_spinner = nullptr;
    recording_label = nullptr;
}

int run_application(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    g_text = detect_ui_text();
    build_application_ui();
    gtk_widget_show_all(window_widget);
    gtk_main();
    return 0;
}

int main(int argc, char* argv[]) {
    return run_application(argc, argv);
}
