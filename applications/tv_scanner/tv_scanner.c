#include <furi.h>
#include <gui/gui.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <infrared_worker.h>

#define TAG "TVScanner"
#define BASE_PATH "/ext/tv_db"
#define DELAY_MS 5000

typedef struct {
    bool tv_detected;
    char model[64];
} TVScannerState;

static void send_ir_signal(const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    InfraredSignal* signal = infrared_signal_alloc();
    
    if (infrared_signal_load(signal, storage, path)) {
        InfraredWorker* worker = infrared_worker_alloc();
        infrared_worker_tx_start(worker, signal);
        infrared_worker_free(worker);
    }
    
    infrared_signal_free(signal);
    furi_record_close(RECORD_STORAGE);
}

static void process_folder(TVScannerState* state, const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* dir = storage_file_alloc(storage);
    FuriString* full_path = furi_string_alloc();

    if (storage_dir_open(dir, path)) {
        FileInfo file_info;
        char name[256];
        while (storage_dir_read(dir, &file_info, name, sizeof(name)) && !state->tv_detected) {
            furi_string_printf(full_path, "%s/%s", path, name);
            
            if (file_info_is_dir(&file_info)) {
                process_folder(state, furi_string_get_cstr(full_path));
            } else if (strstr(name, ".ir")) {
                FURI_LOG_I(TAG, "Probando: %s", name);
                send_ir_signal(furi_string_get_cstr(full_path));

                // Diálogo de confirmación
                DialogMessage* msg = dialog_message_alloc();
                dialog_message_set_header(msg, "¿Funcionó?", 0, 0, AlignLeft, AlignTop);
                dialog_message_set_text(msg, name, 0, 26, AlignLeft, AlignTop);
                dialog_message_set_buttons(msg, "No", NULL, "Sí");
                
                bool detected = false;
                dialog_message_show(dialogs_app_get(), msg, &detected);
                dialog_message_free(msg);

                if (detected) {
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
    TVScannerState state = {.tv_detected = false};
    process_folder(&state, BASE_PATH);
    
    if (state.tv_detected) {
        FURI_LOG_I(TAG, "Modelo detectado: %s", state.model);
    }
    return 0;
}
