#include <mostrarDiccionarioYTablas.h>

void _log_tablas_recursivamente(t_tabla_de_pagina* tabla, int indentacion) {
    if (tabla == NULL) {
        return;
    }

    // Crear una cadena de indentación para una bonita visualización
    char indent_str[indentacion * 2 + 1];
    memset(indent_str, ' ', indentacion * 2);
    indent_str[indentacion * 2] = '\0';

    log_info(logger, "%sTabla Nivel: %d", indent_str, tabla->nivel);

    for (int i = 0; i < list_size(tabla->entradas); i++) {
        if (tabla->nivel < configs->cant_niveles) {
            // Tabla de nivel intermedio
            t_entrada* entrada = list_get(tabla->entradas, i);
            if (entrada->ptrTabla != NULL) {
                log_info(logger, "%s  Entrada %d -> Apunta a Tabla de Nivel %d", indent_str, i, entrada->ptrTabla->nivel);
                _log_tablas_recursivamente(entrada->ptrTabla, indentacion + 1);
            } else {
                log_info(logger, "%s  Entrada %d -> NULL", indent_str, i);
            }
        } else {
            // Tabla de último nivel
            t_entrada_ult_tabla* entrada_final = list_get(tabla->entradas, i);
            if (entrada_final->nroFrame != -1) {
                log_info(logger, "%s  Entrada %d -> Frame: %d", indent_str, i, entrada_final->nroFrame);
            } else {
                log_info(logger, "%s  Entrada %d -> Frame no asignado", indent_str, i);
            }
        }
    }
}

void imprimir_diccionario_completo() {
    
    void _log_proceso_detalle(char* pid_str, void* value_void) {
        t_value_proceso* value_proceso = (t_value_proceso*) value_void;

        log_info(logger, "--------------------------------------------------");
        log_info(logger, ">> PID: %s (Tamaño: %d bytes)", pid_str, value_proceso->tamanio_proceso);
        
        log_info(logger, "  Instrucciones:");
        for (int i = 0; i < list_size(value_proceso->instrucciones); i++) {
            char* instruccion = list_get(value_proceso->instrucciones, i);
            log_info(logger, "    [%d]: %s", i, instruccion);
        }

        log_info(logger, "  Estructura de Tablas de Páginas:");
        _log_tablas_recursivamente(value_proceso->tabla_primer_nvl, 2); // Inicia con indentación nivel 2
    }

    log_info(logger, "============ INICIO DUMP DICCIONARIO DE PROCESOS ============");
    pthread_mutex_lock(&mutex_diccionario_instrucciones);
    
    if (dictionary_is_empty(dicc_pids_con_instrucciones)) {
        log_info(logger, "El diccionario de procesos está vacío.");
    } else {
        dictionary_iterator(dicc_pids_con_instrucciones, _log_proceso_detalle);
    }

    pthread_mutex_unlock(&mutex_diccionario_instrucciones);
    log_info(logger, "============= FIN DUMP DICCIONARIO DE PROCESOS ==============");
}

void imprimir_estado_frames() {
    log_info(logger, "============ INICIO ESTADO DE FRAMES EN MEMORIA ============");
    pthread_mutex_lock(&mutex_frames);

    int total_frames = list_size(lista_frames);
    for (int i = 0; i < total_frames; i++) {
        t_info_frame* frame_info = list_get(lista_frames, i);
        if (frame_info->libre) {
            log_info(logger, "Frame %03d: [LIBRE]", i);
        } else {
            log_info(logger, "Frame %03d: [OCUPADO] - PID: %d, Página: %d", i, frame_info->pid_proceso, frame_info->nro_pagina);
        }
    }

    pthread_mutex_unlock(&mutex_frames);
    log_info(logger, "============= FIN ESTADO DE FRAMES EN MEMORIA ==============");
}