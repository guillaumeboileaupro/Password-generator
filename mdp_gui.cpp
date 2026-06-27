#include <gtk/gtk.h>
#include <alsa/asoundlib.h>
#include "password_core.h"

#include <cerrno>
#include <glib/gstdio.h>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

GtkWidget* length_entry;
GtkWidget* lower_check;
GtkWidget* upper_check;
GtkWidget* digits_check;
GtkWidget* symbols_check;
GtkWidget* result_entry;
GtkWidget* status_label;

const size_t kMaxPasswordLength = 1024;
const char* kLocalIconPath = "mdp-logo.png";

vector<unsigned char> capture_microphone_noise(unsigned int seconds = 2) {
    snd_pcm_t* handle;
    const char* device = "default";

    unsigned int sample_rate = 44100;
    int channels = 1;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

    int err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        throw runtime_error("Impossible d'ouvrir le microphone.");
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
        throw runtime_error("Impossible de configurer le microphone.");
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
            throw runtime_error("Erreur pendant la capture du microphone.");
        }

        samples_read += frames;
    }

    snd_pcm_close(handle);

    return vector<unsigned char>(
        reinterpret_cast<unsigned char*>(buffer.data()),
        reinterpret_cast<unsigned char*>(buffer.data()) + buffer.size() * sizeof(int16_t)
    );
}

void on_generate_clicked(GtkWidget*, gpointer) {
    gtk_label_set_text(
        GTK_LABEL(status_label),
        "Capture du bruit du microphone pendant 2 secondes..."
    );
    while (gtk_events_pending()) gtk_main_iteration();

    try {
        const char* length_text = gtk_entry_get_text(GTK_ENTRY(length_entry));
        int length = stoi(length_text);

        if (length <= 0) {
            throw runtime_error("La longueur doit etre positive.");
        }
        if (length > static_cast<int>(kMaxPasswordLength)) {
            throw runtime_error("La longueur maximale est 1024.");
        }

        const PasswordOptions options{
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lower_check)) != 0,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(upper_check)) != 0,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(digits_check)) != 0,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(symbols_check)) != 0
        };

        const vector<unsigned char> microphone_noise = capture_microphone_noise(2);
        const vector<unsigned char> seed = sha256_bytes(microphone_noise);
        const string password = generate_password_from_seed(length, options, seed);

        gtk_entry_set_text(GTK_ENTRY(result_entry), password.c_str());
        gtk_label_set_text(GTK_LABEL(status_label), "Mot de passe genere a partir du bruit du microphone.");

    } catch (const exception& e) {
        gtk_label_set_text(GTK_LABEL(status_label), e.what());
    }
}

void on_copy_clicked(GtkWidget*, gpointer) {
    const char* password = gtk_entry_get_text(GTK_ENTRY(result_entry));
    if (password == nullptr || password[0] == '\0') {
        gtk_label_set_text(GTK_LABEL(status_label), "Aucun mot de passe a copier.");
        return;
    }

    GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, password, -1);

    gtk_label_set_text(GTK_LABEL(status_label), "Mot de passe copie dans le presse-papiers.");
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    g_set_prgname("mdp-generator");
    g_set_application_name("Generateur de mots de passe");

    if (g_file_test(kLocalIconPath, G_FILE_TEST_EXISTS)) {
        gtk_window_set_default_icon_from_file(kLocalIconPath, nullptr);
    }

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Generateur de mots de passe");
    gtk_window_set_default_size(GTK_WINDOW(window), 520, 320);
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);

    if (g_file_test(kLocalIconPath, G_FILE_TEST_EXISTS)) {
        gtk_window_set_icon_from_file(GTK_WINDOW(window), kLocalIconPath, nullptr);
    } else {
        gtk_window_set_icon_name(GTK_WINDOW(window), "mdp-logo");
    }

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget* title = gtk_label_new("Generateur de mots de passe");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 5);

    length_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(length_entry), "32");
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(length_entry),
        "Longueur du mot de passe"
    );
    gtk_box_pack_start(GTK_BOX(box), length_entry, FALSE, FALSE, 5);

    GtkWidget* options_label = gtk_label_new("Types de caracteres");
    gtk_label_set_xalign(GTK_LABEL(options_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(box), options_label, FALSE, FALSE, 0);

    lower_check = gtk_check_button_new_with_label("Minuscules a-z");
    upper_check = gtk_check_button_new_with_label("Majuscules A-Z");
    digits_check = gtk_check_button_new_with_label("Chiffres 0-9");
    symbols_check = gtk_check_button_new_with_label("Symboles");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lower_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(upper_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(digits_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(symbols_check), TRUE);

    gtk_box_pack_start(GTK_BOX(box), lower_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), upper_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), digits_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), symbols_check, FALSE, FALSE, 0);

    GtkWidget* generate_button = gtk_button_new_with_label("Generer");
    gtk_box_pack_start(GTK_BOX(box), generate_button, FALSE, FALSE, 10);

    result_entry = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(result_entry), FALSE);
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(result_entry),
        "Le mot de passe apparaitra ici"
    );
    gtk_box_pack_start(GTK_BOX(box), result_entry, FALSE, FALSE, 5);

    GtkWidget* copy_button = gtk_button_new_with_label("Copier");
    gtk_box_pack_start(GTK_BOX(box), copy_button, FALSE, FALSE, 5);

    status_label = gtk_label_new("Pret");
    gtk_label_set_line_wrap(GTK_LABEL(status_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(status_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 5);

    g_signal_connect(generate_button, "clicked", G_CALLBACK(on_generate_clicked), NULL);
    g_signal_connect(copy_button, "clicked", G_CALLBACK(on_copy_clicked), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
