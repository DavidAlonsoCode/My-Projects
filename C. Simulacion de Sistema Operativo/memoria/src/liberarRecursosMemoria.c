#include <liberarRecursosMemoria.h>

void destruir_diccionario_instrucciones() {
    // "Tomá liberar_instrucciones y tratala como si fuera una función que recibe (char*, void*)"
	dictionary_iterator(dicc_pids_con_instrucciones, (void(*)(char*, void*))liberar_instrucciones);
	dictionary_destroy(dicc_pids_con_instrucciones); // libera las claves también
}

//La funcion que recibe dictionary_iterator esta hecha para trabajar una key y un value, por eso son void* y char*, pero lo casteo a lista
void liberar_instrucciones(char* key, t_list* instrucciones) {
	// Itero la lista y libero cada instrucción
	list_destroy_and_destroy_elements(instrucciones, free);
	
	//free(key); // porque se hizo strdup de key_pid --> ahora no
}

void destruir_lista_frames(t_list* lista_frames) {
	if (lista_frames != NULL) {
		for (int i = 0; i < list_size(lista_frames); i++) {
			t_info_frame* frame = list_get(lista_frames, i);
			free(frame); // Liberamos cada frame
		}
		list_destroy(lista_frames); // Liberamos la lista
	}
}