#include "ciclo.h"  // implementa

int crear_conexion_segun(char* tipo, char* ip, char* puerto) {
    int conexion = crear_conexion(ip, puerto);
    if (conexion == -1) {
        log_error(logger, "No se pudo conectar al %s en %s:%s", tipo, ip, puerto);
        return EXIT_FAILURE;
    } else {
        log_info(logger, "Conexión establecida con %s en %s:%s", tipo, ip, puerto);
    }
    return conexion;
}

int check_interrupt(int conexion){
    uint32_t codOp;
    int pidReady = -1;
    // Verifica si hay una interrupcion en el puerto de interrupcion
    codOp = recibir_operacion_no_bloqueante(conexion);

    bool hayInterrupciones = codOp == INTERRUPCION;

    if(hayInterrupciones){
    pidReady= recibir_pid(conexion);
    }
    
    return pidReady;
}