#include <furi.h>
#include <gui/gui.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <infrared_worker.h>
#include <toolbox/stream/file_stream.h>

#define TAG "TVScanner"
#define BASE_PATH "/ext/tv_db"
#define DELAY_MS 5000

typedef struct {
    FuriMessageQueue* input_queue;
    ViewDispatcher* view_dispatcher;
    bool tv_detected;
    char model[64];
} TVScanner;

static void send_ir_signal(const char* path) {
    InfraredSignal* signal = infrared_signal_alloc();
    if(infrared_signal_load(signal, path)) {
        infrared_worker_tx(infrared_worker_alloc(), signal);
    }
    infrared_signal_free(signal);
}

static void process_folder(TVScanner* state, const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* dir = storage_file_alloc(storage);
    FuriString* full_path = furi_string_alloc();

    if(storage_dir_open(dir, path)) {
        FileInfo file_info;
        char name[256];
        while(storage_dir_read(dir, &file_info, name, sizeof(name)) && !state->tv_detected) {
            furi_string_printf(full_path, "%s/%s", path, name);
            
            if(file_info_is_dir(&file_info)) {
                process_folder(state, furi_string_get_cstr(full_path));
            } else if(strstr(name, ".ir")) {
                FURI_LOG_I(TAG, "Probando: %s", name);
                send_ir_signal(furi_string_get_cstr(full_path));

                DialogMessage* msg = dialog_message_alloc();
                dialog_message_set_header(msg, "¿Funcionó?", 0, 0, AlignLeft, AlignTop);
                dialog_message_set_text(msg, name, 0, 26, AlignLeft, AlignTop);
                dialog_message_set_buttons(msg, "No", NULL, "Sí");
                
                bool detected = false;
                dialog_message_show(state->view_dispatcher, msg, &detected, NULL);
                dialog_message_free(msg);

                if(detected) {
                    state->tv_detected = true;
                    strlcpy(state->model, name, sizeof(state->model));
                    break;
                }
                furi_delay_ms(DELAY_MS);
            }
        }
    }
    furi_string_free(full_path);
    storage_dir_close(dir);
    storage_file_free(dir);
    furi_record_close(RECORD_STORAGE);
}

int32_t tv_scanner_app(void* p) {
    TVScanner state = {.tv_detected = false};
    process_folder(&state, BASE_PATH);
    
    if(state.tv_detected) {
        FURI_LOG_I(TAG, "Modelo detectado: %s", state.model);
        // Mostrar resultado en pantalla
    }
    return 0;
}