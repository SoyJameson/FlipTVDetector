#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <infrared_worker.h>

// Estructura para el estado de la aplicación
typedef struct {
    FuriMessageQueue* input_queue;
    ViewDispatcher* view_dispatcher;
    bool tv_detected;
    char model[64];
} TVScanner;

// Prototipos de funciones públicas
int32_t tv_scanner_app(void* p);