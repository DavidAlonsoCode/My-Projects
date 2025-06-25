#include "ciclo.h"        // implementa
#include "cache.h"       // importa
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE_CADENA 30

// Estas estructuras contienen la tabla de instrucciones para usarse con bsearch
typedef struct {
    char* cadena;
    t_instruccion operacion;
} t_tabla_instruccion;


const t_tabla_instruccion tabla[] = {
    {"DUMP_MEMORY", INST_DUMP_MEMORY},
    {"EXIT", INST_EXIT},
    {"GOTO", INST_GOTO},
    {"INIT_PROC", INST_INIT_PROC},
    {"IO", INST_IO},
    {"NOOP", INST_NOOP},
    {"READ", INST_READ},
    {"WRITE", INST_WRITE}
};

// Esta funcion compara la cadena con el contenido de la tabla
int comparar_instrucciones(const void* a, const void* b) {
    const char* clave = (const char*)a; //Clave
    const t_tabla_instruccion* instruccion = (const t_tabla_instruccion*)b; //Valor
    return strcmp(clave, instruccion->cadena);
}

// Dado una cadena de chars, el ptr de la cadena y un index devuelve una cadena hasta un espacio
char* obtener_cadena(char* instruccion, char* ptr, int* index) {
    char* cadena = malloc(MAX_SIZE_CADENA);
    int indexInicial = *index;

    while (isalpha(instruccion[*index]) || instruccion[*index] == '_') {
        (*index)++;
    }

    int tam_cadena = *index - indexInicial; 
    strncpy(cadena, ptr, tam_cadena);
    cadena[tam_cadena] = '\0';

    return cadena;
}

// Dado una cadena de chars, el ptr de la cadena y un index devuelve una cadena hasta un espacio
char* obtener_alfanumerico(char* instruccion, char* ptr, int* index) {
    char* cadena = malloc(MAX_SIZE_CADENA);
    int indexInicial = *index;

    while (isdigit(instruccion[*index]) || isalpha(instruccion[*index]) || instruccion[*index] == '_') {
        (*index)++;
    }

    int tam_cadena = *index - indexInicial; 
    strncpy(cadena, ptr, tam_cadena);
    cadena[tam_cadena] = '\0';

    return cadena;
}

// Dado una cadena de chars, el ptr de la cadena y un index devuelve un entero
int obtener_entero(char* instruccion, char* ptr, int* index) {
    char* cadena = malloc(MAX_SIZE_CADENA);
    int indexInicial = *index;

    while(isdigit(instruccion[*index])) {
        (*index)++;
    }

    int tam_cadena = *index - indexInicial; 
    strncpy(cadena, ptr, tam_cadena); // strncpy copia en cadena desde ptr hasta tam_cadena
    cadena[tam_cadena] = '\0'; 

    return atoi(cadena); // Se convierte la cadena a un entero
}

void recibir_mensaje_desde_memoria(int fd_conexion) {
    uint32_t codOp = EN_ESPERA; 
    
    while(codOp != MENSAJE) {
        codOp = recibir_operacion(fd_conexion);
    }

    recibir_mensaje_desde_cliente(fd_conexion);
}

// Para recibir los datos de un proceso enviados por el kernel a traves de una estructura
t_proceso_actual* recibir_paquete_proceso_cpu(int conexion_kernel) {
    t_proceso_actual* proceso = malloc(sizeof(t_proceso_actual));
    uint32_t codOp = EN_ESPERA;
    
    while(codOp != PROCESO_A_EJECUTAR) {
        codOp = recibir_operacion(conexion_kernel);
    }

    uint32_t size;
    void *buffer = recibir_buffer(&size, conexion_kernel);

    int desplazamiento = 0;
    memcpy(&(proceso->pid), buffer + desplazamiento, sizeof(uint32_t)); //recibo el pid

    desplazamiento += sizeof(uint32_t);
    memcpy(&(proceso->pc), buffer + desplazamiento, sizeof(uint32_t)); //recibo el pc

    free(buffer);
    return proceso;
}


t_instruccion fetch_decode_execute(t_proceso_actual* proceso) {

    /* FETCH */
    // Se recibe el pid y pc del proceso a ejecutar
    char* instruccion = fetch(proceso); // Con los datos del proceso, se obtiene la instruccion desde memoria


    /* DECODE */
    //Lee la cadena MIENTRAS sean digitos y la guarda en cadena
    int i = 0;
    char* ptr = instruccion;
    char* cadena = obtener_cadena(instruccion, ptr, &i);
    i++;

    t_tabla_instruccion* instruccionEncontrada = bsearch(cadena, tabla, sizeof(tabla)/sizeof(t_tabla_instruccion), sizeof(t_tabla_instruccion), comparar_instrucciones);

    //Bsearch devuelve NULL si no encuentra la instruccion, pero como no habra errores lexicos ni sintacticos no se verifica
    t_instruccion operacion = instruccionEncontrada->operacion;

    // Dada la operacion obtenida
    // se solicitan los argumentos segun se requiera
    uint32_t pid = proceso->pid; // para loguear el pid del proceso

    switch(operacion) {
        case INST_NOOP:  // No hace nada

            /* EXECUTE */
            log_info(logger, "## PID: %d - Ejecutando INST_NOOP sin parametros", pid);
            free(cadena);
            break;

        case INST_WRITE: { // WRITE <direccion[int]> <datos[string]>
            uint32_t direccion = 0;
            char* datos;

            ptr = instruccion + i; // se salta el espacio y se guarda la referencia del ptr de la cadena
            direccion = obtener_entero(instruccion, ptr, &i);
            i++; // se salta el espacio

            ptr = instruccion + i;
            datos = obtener_alfanumerico(instruccion, ptr, &i);
            
            t_entrada_cache* entrada_cache;
            /* EXECUTE */
            if((entrada_cache = verificar_en_cache(direccion)) != NULL) {
                log_info(logger, "PID: %d - Cache HIT - Pagina: %d", pid, entrada_cache->pagina);
                ejecutar_write_en_cache(entrada_cache, datos);
                log_info(logger, "## PID: %d - Ejecutando INST_WRITE desde CACHE: direccion=%d, datos=%s", pid, direccion, datos);

            } else {
                uint32_t direccionFisica = traducir_a_direccion_fisica(direccion, pid);

                log_info(logger, "## PID: %d - Ejecutando INST_WRITE con parametros: direccion=%d, datos=%s", pid, direccion, datos);
                ejecutar_write(proceso, direccionFisica, datos); 
                agregar_a_cache(direccionFisica, datos, pid); // Se guardan los datos escritos en la cache (el PID solo se usa para el log)
            }
            break;
        }
            
        case INST_READ: { // READ <direccion[int]> <tamanio[int]>
            uint32_t direccion = 0;
            uint32_t tamanio   = 0;
            char* datos;    

            ptr = instruccion + i;
            direccion = obtener_entero(instruccion, ptr, &i);
            i++; // se salta el espacio

            ptr = instruccion + i;
            tamanio = obtener_entero(instruccion, ptr, &i);

            t_entrada_cache* entrada_cache;
            /* EXECUTE */
            if((entrada_cache = verificar_en_cache(direccion)) != NULL) {
                log_info(logger, "PID: %d - Cache Hit - Pagina: %d", pid, entrada_cache->pagina);
                datos = ejecutar_read_en_cache(entrada_cache, tamanio);
                log_info(logger, "## PID: %d - Ejecutando INST_READ desde CACHE: direccion=%d, tamanio=%d", pid, direccion, tamanio); 
                printf("## PID: %d - READ - Datos leidos: %s\n", pid, datos);
                
            } else {
                uint32_t direccionFisica = traducir_a_direccion_fisica(direccion, pid);

                log_info(logger, "## PID: %d - Ejecutando INST_READ con parametros: direccion=%d, tamanio=%d", pid, direccion, tamanio);
                datos = ejecutar_read(proceso, direccionFisica, tamanio);
                agregar_a_cache(direccionFisica, datos, pid); // Se guardan los datos leidos en la cache (el PID solo se usa para el log)
            }
            break;
        }
            
        case INST_GOTO: { // GOTO <valor[int]>
            uint32_t valor = 0;

            ptr = instruccion + i;
            valor = obtener_entero(instruccion, ptr, &i);

            /* EXECUTE */
            log_info(logger, "## PID: %d - Ejecutando INST_GOTO con parametro: valor=%d", pid, valor);
            ejecutar_goto(proceso, valor); // -> manejar con precaucion
            break;
        }

        case INST_IO: { // IO <dispositivo[string]> <tiempo[int]>
            char* dispositivo;
            uint32_t tiempo = 0;

            ptr = instruccion + i;
            dispositivo = obtener_cadena(instruccion, ptr, &i);
            i++; // se salta el espacio

            ptr = instruccion + i;
            tiempo = obtener_entero(instruccion, ptr, &i);

            /* EXECUTE */
            log_info(logger, "## PID: %d - Ejecutando INST_IO con parametros: dispositivo=%s, tiempo=%d", pid, dispositivo, tiempo);
            ejecutar_io(proceso, dispositivo, tiempo);
            break;
        }
            
        case INST_INIT_PROC: { // INIT_PROC <archivo[string]> <tamanio[int]>
            char* archivo;
            uint32_t tamanio = 0;
            
            ptr = instruccion + i;
            archivo = obtener_alfanumerico(instruccion, ptr, &i);
            i++; // se salta el espacio
        
            ptr = instruccion + i;
            tamanio = obtener_entero(instruccion, ptr, &i);

            /* EXECUTE */
            log_info(logger, "## PID: %d - Ejecutando INST_INIT_PROC con parametros: archivo=%s, tamanio=%d", pid, archivo, tamanio);
            ejecutar_init_proc(proceso, archivo, tamanio);
            break;
        }

        case INST_DUMP_MEMORY: // NO TIENE ARGUMENTOS

            /* EXECUTE */
            log_info(logger, "## PID: %d - Ejecutando INST_DUMP_MEMORY", pid);
            ejecutar_dump_memory(proceso);
            break;

        case INST_EXIT: // FINALIZA EL PROCESO

            //Esta actualización es innecesaria dado que luego del EXIT el proceso termina
            //Pero se implementa para realizar las pruebas con un solo proceso
            vaciar_entradas_tlb(pid); 
            vaciar_entradas_cache(pid, proceso->fd_conexion_memoria); 

            /* EXECUTE */
            log_info(logger, "## PID: %d - Ejecutando INST_EXIT", pid); 
            ejecutar_exit(proceso);
            break;

        default:
            // Si no se reconoce la operacion, se logea el error y sale de la funcion
            log_error(logger, "Instruccion no reconocida: %s", cadena);
            free(cadena);
            exit(EXIT_FAILURE);
            //return;
    }    

    // si todo se ejecuto correctamente, termina la funcion y prosigue a la siguiente etapa
    free(instruccion);
    return operacion; // se retorna la operacion para verificar si debe terminarse el ciclo
}

