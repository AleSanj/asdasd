/*
 ============================================================================
 Name        : mi_ram_hq.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

// =============== PAHTS =================
//-------------------------------------
//PARA EJECUTAR DESDE ECLIPSE USAR:
//#define PATH_CONFIG "src/mi_ram_hq.config"
//-------------------------------------
//PARA EJECUTAR DESDE CONSOLA USAR:
#define PATH_CONFIG "../src/mi_ram_hq.config"
//-------------------------------------

#include "mi_ram_hq.h"


#define ASSERT_CREATE(nivel, id, err)


int main(void) {
	int socketCliente, socketServer;

	config = config_create(PATH_CONFIG);
	funcionando=true;
	tamMemoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	esquemaMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");
	tamPagina = config_get_int_value(config, "TAMANIO_PAGINA");
	tamSwap = config_get_int_value(config, "TAMANIO_SWAP");
	path_swap = config_get_string_value(config, "PATH_SWAP");
	alg_remplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	crit_seleccion = config_get_string_value(config, "CRITERIO_SELECCION");
	puerto = config_get_string_value(config, "PUERTO");
	pthread_mutex_init(&mutexMemoria, NULL);
	for(int i =0; i<94;i++){
		vectorIdTripulantes[i] = -1;
	}

	signal(SIGUSR1,&manejoDump);
	signal(SIGUSR2,&manejoCompactacion);
	if (strcmp(crit_seleccion,"FF") == 0){
		tipoDeGuardado = FIRSTFIT;
	}else{
		tipoDeGuardado = BESTFIT;
	}
	if((logger = log_create("../log.txt", "Memoria", 0, LOG_LEVEL_INFO)) == NULL)
		{
			printf(" No pude leer el logger\n");
			exit(1);
		}
	memoria = malloc(tamMemoria);
	listaElementos = list_create();
	listaDeTablasDePaginas = list_create();
	if (strcmp(esquemaMemoria,"PAGINACION")==0){
		memoriaSwap = malloc(tamSwap);
		cantidadPaginas = tamMemoria / tamPagina;
		cantidadPaginasSwap = tamSwap / tamPagina;
		bitarrayMemoria = calloc(cantidadPaginas, sizeof(int));
		bitarraySwap = calloc(cantidadPaginasSwap, sizeof(int));
		tablaDeFrames = queue_create();
		punteroReemplazo = 0;
		espacioLibre = tamMemoria + tamSwap;
		int swap = open(path_swap, O_CREAT | O_RDWR);
		ftruncate(swap, tamSwap);
		mmap(memoriaSwap, tamSwap, PROT_READ | PROT_WRITE, MAP_SHARED, swap, 0);
		close(swap);
		msync(memoriaSwap, tamSwap, MS_SYNC);


	}else if (strcmp(esquemaMemoria,"SEGMENTACION")==0){
		listaSegmentos = list_create();
		espacioLibre = tamMemoria;
		listaGlobalDeSegmentos = list_create();

	}

	nivel = crear_mapa();
	socketServer = crear_server(puerto);

	while (funcionando) {
		socketCliente = esperar_cliente(socketServer, 5);
		if (socketCliente == -1)
			continue;

			pthread_t hiloCliente;
			pthread_create(&hiloCliente,NULL,(void*)administrar_cliente,socketCliente);
			pthread_join(hiloCliente,NULL);
	}


	liberar_conexion(socketServer);

	//crear_personajes(nivel, patota);
	return EXIT_SUCCESS;
	}



void administrar_cliente(int socketCliente){
		int respuesta;
		t_paquete* paquete_recibido = recibir_paquete(socketCliente, &respuesta);
		if (paquete_recibido->codigo_operacion == -1 || respuesta == ERROR) {
			liberar_conexion(socketCliente);
			eliminar_paquete(paquete_recibido);
		}

//		printf("PAQUETE DE TIPO %d RECIBIDO\n",paquete_recibido->codigo_operacion);

	switch(paquete_recibido->codigo_operacion) {
		case INICIAR_PATOTA:;
			//printf("Inicio una patota /n");
			t_iniciar_patota* estructura_iniciar_patota = deserializar_iniciar_patota(paquete_recibido);
			//imprimir_paquete_iniciar_patota(estructura_iniciar_patota);
			int espacioNecesario;
			espacioNecesario = estructura_iniciar_patota->tamanio_tareas + (estructura_iniciar_patota->cantTripulantes*21)+8;
			if(espacioLibre<espacioNecesario){
				log_info(logger, "No tengo espacio en la memoria para guardar la patota %d\n",estructura_iniciar_patota->idPatota);
				char* fault = strdup("fault");
				uint32_t tamanio_fault = strlen(fault)+1;
				send(socketCliente,&tamanio_fault,sizeof(uint32_t),0);
				send(socketCliente, fault,tamanio_fault,0);
				free(fault);
			}else{

				pcb* nuevaPatota = malloc(sizeof(pcb));
				nuevaPatota->id = estructura_iniciar_patota->idPatota;
				if (strcmp(esquemaMemoria,"PAGINACION")==0){
					tablaEnLista_struct *nuevaTablaPatota = malloc(sizeof(tablaEnLista_struct));
					nuevaTablaPatota->idPatota = nuevaPatota->id;
					log_info(logger, "Recibi la patota: %d\n",nuevaPatota->id);
					nuevaTablaPatota->tablaDePaginas = list_create();
					list_add(listaDeTablasDePaginas, nuevaTablaPatota);
				}else{
					tablaEnLista_struct *nuevaListaDeTablasDePaginas = malloc(sizeof(tablaEnLista_struct));
					nuevaListaDeTablasDePaginas->tablaDePaginas = list_create();
					nuevaListaDeTablasDePaginas->idPatota=estructura_iniciar_patota->idPatota;
					list_add(listaDeTablasDePaginas,nuevaListaDeTablasDePaginas);
				}
				guardar_en_memoria_general(estructura_iniciar_patota->Tareas,estructura_iniciar_patota->idPatota,estructura_iniciar_patota->tamanio_tareas,estructura_iniciar_patota->idPatota,'A');
				log_info(logger, "Guarde las tareas de la patota %d\n",estructura_iniciar_patota->idPatota);
				nuevaPatota->tareas =  calcular_direccion_logica_archivo(estructura_iniciar_patota->idPatota);
				guardar_en_memoria_general(nuevaPatota,estructura_iniciar_patota->idPatota,sizeof(pcb),estructura_iniciar_patota->idPatota,'P');
				log_info(logger, "Guarde el PCB de la patota %d\n",estructura_iniciar_patota->idPatota);
			}
			break;
		case TRIPULANTE:;
			//printf("CASE TRIPULANTE /n");
				t_tripulante* estructura_tripulante = deserializar_tripulante(paquete_recibido);
				tcb *nuevoTripulante = malloc(sizeof(tcb));
				nuevoTripulante->id = estructura_tripulante->id_tripulante;
				nuevoTripulante->estado = 'R';
				nuevoTripulante->posX = estructura_tripulante->posicion_x;
				nuevoTripulante->posY = estructura_tripulante->posicion_y;
				nuevoTripulante->proxTarea=0;
				nuevoTripulante->dirLogicaPcb=(uint32_t)calcular_direccion_logica_patota((int)estructura_tripulante->id_patota);
				guardar_en_memoria_general(nuevoTripulante,estructura_tripulante->id_tripulante,21,estructura_tripulante->id_patota,'T');
				for(int i =0; i<94;i++){
					if (vectorIdTripulantes[i]==-1){
						vectorIdTripulantes[i] = nuevoTripulante->id;
						pthread_mutex_lock(&mutexMapa);
						dibujarTripulante(nuevoTripulante,(i+33));
						pthread_mutex_unlock(&mutexMapa);
						break;
					}
				}
				log_info(logger, "Guarde el tripulante %d\n",estructura_tripulante->id_tripulante);
				//printf("CREE UN TRIPULANTE: %d\n",nuevoTripulante->id);

		//		liberar_tripulante(estructura_tripulante);
				liberar_conexion(socketCliente);
				break;
		case ELIMINAR_TRIPULANTE:;
			//printf("eliminar TRIPULANTE /n");
				t_tripulante* tripulante_a_eliminar = deserializar_tripulante(paquete_recibido);
				borrar_de_memoria_general(tripulante_a_eliminar->id_tripulante, tripulante_a_eliminar->id_patota, 'T');
				for(int i =0; i<94;i++){
					if (vectorIdTripulantes[i]==tripulante_a_eliminar->id_tripulante){
						vectorIdTripulantes[i] = 0;
						pthread_mutex_lock(&mutexMapa);
						borrarTripulante((i+33));
						pthread_mutex_unlock(&mutexMapa);
						break;
					}
				}
				log_info(logger, "Borre el tripulante %d\n",tripulante_a_eliminar->id_tripulante);
					break;

		case PEDIR_TAREA:;
			//printf("PEDIR TAREA /n");
				t_tripulante* tripulante_solicitud = deserializar_tripulante(paquete_recibido);
				//imprimir_paquete_tripulante(tripulante_solicitud);
				char*tareas = string_new();
				tareas = buscar_en_memoria_general(tripulante_solicitud->id_patota,tripulante_solicitud->id_patota,'A');
				char **arrayTareas = string_split(tareas,"|");
				tcb *tripulanteATraer = malloc(sizeof(tcb));
				tripulanteATraer = buscar_en_memoria_general(tripulante_solicitud->id_tripulante,tripulante_solicitud->id_patota,'T');
				pcb* patotaATraer = malloc(sizeof(pcb));
				patotaATraer = buscar_en_memoria_general(tripulante_solicitud->id_patota,tripulante_solicitud->id_patota,'P');

				int i=0;
				int totalDeTareas=0;
				while(arrayTareas[i]!=NULL){
					totalDeTareas++;
					i++;
				}
				if(tripulanteATraer->proxTarea==totalDeTareas){
					char* fault = strdup("fault");
					uint32_t tamanio_fault = strlen(fault)+1;
					send(socketCliente,&tamanio_fault,sizeof(uint32_t),0);
					send(socketCliente, fault,tamanio_fault,0);
					free(fault);
					log_info(logger, "Mande la tarea fault\n");
				}else{
					//log_info(logger, "Soy el tripulante %d, estoy en x=%d y=%d\n",tripulanteATraer->id,tripulanteATraer->posX,tripulanteATraer->posY);
					int tamanio_tarea = strlen(arrayTareas[tripulanteATraer->proxTarea])+1;
					send(socketCliente, &tamanio_tarea,sizeof(uint32_t),0);
					send(socketCliente, arrayTareas[tripulanteATraer->proxTarea],tamanio_tarea,0);
					if(strcmp(esquemaMemoria,"PAGINACION")==0){
						pthread_mutex_lock(&mutexMemoria);
						actualizar_indice_paginacion(tripulante_solicitud->id_tripulante,tripulante_solicitud->id_patota);
						pthread_mutex_unlock(&mutexMemoria);
					}else if (strcmp(esquemaMemoria,"SEGMENTACION")==0){
						pthread_mutex_lock(&mutexMemoria);
						actualizar_indice_segmentacion(tripulante_solicitud->id_tripulante,tripulante_solicitud->id_patota);
						pthread_mutex_unlock(&mutexMemoria);
					}
					log_info(logger, "Mande la tarea %s\n",arrayTareas[tripulanteATraer->proxTarea]);
				}

				for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
				    tablaEnLista_struct *tablaBuscada= malloc(sizeof(tablaEnLista_struct));
					tablaBuscada = list_get(listaDeTablasDePaginas,i);
				}
				liberar_conexion(socketCliente);

				break;


		case ACTUALIZAR_POS:;
			//printf("ACTUALIZAR POS/n");
				t_tripulante* tripulante_a_mover = deserializar_tripulante(paquete_recibido);
				//imprimir_paquete_tripulante(tripulante_a_mover);
				tcb *tripulanteAMover = malloc(sizeof(tcb));
				tripulanteAMover = buscar_en_memoria_general(tripulante_a_mover->id_tripulante,tripulante_a_mover->id_patota, 'T');
				tripulanteAMover->posX = tripulante_a_mover->posicion_x;
				tripulanteAMover->posY = tripulante_a_mover->posicion_y;
				if(strcmp(esquemaMemoria,"PAGINACION")==0){
					pthread_mutex_lock(&mutexMemoria);
					actualizar_posicion_paginacion(tripulante_a_mover->id_tripulante,tripulante_a_mover->id_patota,tripulante_a_mover->posicion_x,tripulante_a_mover->posicion_y);
					pthread_mutex_unlock(&mutexMemoria);
				}else if (strcmp(esquemaMemoria,"SEGMENTACION")==0){
					pthread_mutex_lock(&mutexMemoria);
					actualizar_posicion_segmentacion(tripulante_a_mover->id_tripulante,tripulante_a_mover->id_patota,tripulante_a_mover->posicion_x,tripulante_a_mover->posicion_y);
					pthread_mutex_unlock(&mutexMemoria);
				}
				id_and_pos *tripulanteEnMapa = malloc(sizeof(id_and_pos));
				tripulanteEnMapa->idTripulante = tripulanteAMover->id;
				tripulanteEnMapa->posX = tripulante_a_mover->posicion_x;
				tripulanteEnMapa->posY = tripulante_a_mover->posicion_y;

				for(int i =0; i<94;i++){
					if (vectorIdTripulantes[i]==tripulanteEnMapa->idTripulante){
						pthread_mutex_lock(&mutexMapa);
						actualizarPosicion(tripulanteEnMapa,i+33);
						pthread_mutex_unlock(&mutexMapa);
						break;
					}
				}
				for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
					tablaEnLista_struct *tablaBuscada= malloc(sizeof(tablaEnLista_struct));
					tablaBuscada = list_get(listaDeTablasDePaginas,i);
				}
				break;

		case ACTUALIZAR_ESTADO:;
			//printf("ACTUALIZAR_ESTADO /n");
				t_cambio_estado* tripulante_a_actualizar = deserializar_cambio_estado(paquete_recibido);
				//imprimir_paquete_cambio_estado(tripulante_a_actualizar);
				if(strcmp(esquemaMemoria,"PAGINACION")==0){
					pthread_mutex_lock(&mutexMemoria);
					actualizar_estado_paginacion(tripulante_a_actualizar->id_tripulante,tripulante_a_actualizar->id_patota,tripulante_a_actualizar->estado);
					pthread_mutex_unlock(&mutexMemoria);
					//printf("CAMBIO ESTADO: %c\n",tcbDePrueba->estado);
				}else{
					pthread_mutex_lock(&mutexMemoria);
					actualizar_estado_segmentacion(tripulante_a_actualizar->id_tripulante,tripulante_a_actualizar->id_patota,tripulante_a_actualizar->estado);
					pthread_mutex_unlock(&mutexMemoria);
				}
				log_info(logger, "Actualice el estado del tripulante %d a %c\n",tripulante_a_actualizar->id_tripulante, tripulante_a_actualizar->estado);
				for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
					tablaEnLista_struct *tablaBuscada= malloc(sizeof(tablaEnLista_struct));
					tablaBuscada = list_get(listaDeTablasDePaginas,i);
				}
				break;

		//case FINALIZAR:;
			//break;
		default:;

			break;


	}
}

char intAChar(int numero){
	return numero + '0';
}

void actualizarPosicion(id_and_pos* nuevaPos,char id){
	item_mover(nivel, id, nuevaPos-> posX,nuevaPos->posY);
	nivel_gui_dibujar(nivel);
}


void dibujarTripulante(tcb* tripulante, char id){
	int err;
	//printf("el id en dibu tripu es: %c",id);
	//char* id[3] = '0';
	//char id = intAChar(tripulante->id);
	err = personaje_crear(nivel, id, tripulante->posX, tripulante->posY);
	ASSERT_CREATE(nivel, id, err);

	/*if(err) {
		//printf("Error: %s\n", nivel_gui_string_error(err));
	}*/
	nivel_gui_dibujar(nivel);

	//free (id);
}
void borrarTripulante( char id){
	item_borrar(nivel, id);
}

NIVEL *crear_mapa(){
		NIVEL *nivel;
		int cols=9, rows=9;
		int err;

		nivel_gui_inicializar();

		nivel_gui_get_area_nivel(&cols, &rows);

		nivel = nivel_crear("Mapa de la nave");
		nivel_gui_dibujar(nivel);
		//printf("Ya Dibuje \n");
		return nivel;
}

void dumpDeMemoria(){
	char* timestamp = (char*) temporal_get_string_time("%d-%m-%y_%H:%M:%S");
	char* nombreArchivo = string_new();
	//string_append(&nombreArchivo,"/home/utnso/TP/tp-2021-1c-Cebollitas-subcampeon/mi_ram_hq/");
	string_append(&nombreArchivo,"Dump_");
	string_append(&nombreArchivo,timestamp);
	string_append(&nombreArchivo,".dmp");
	FILE* dmp = fopen (nombreArchivo, "w+");
	//FILE* dmp = fopen ("Dump_", "w+");
	free(nombreArchivo);
	char* lineaAAgregar=string_new();
	fprintf(dmp,"Dump: %s \n",timestamp);
	if(strcmp(esquemaMemoria,"PAGINACION")==0){
		for(int i =0;i<(tamMemoria/tamPagina);i++){

			if(bitarrayMemoria[i] == 0){
				fprintf(dmp,"Marco: %d  Estado:Libre  Proceso:-  Pagina:- \n",i);
			}else{
				for(int j=0;j<list_size(listaDeTablasDePaginas);j++){
					tablaEnLista_struct *tablaAEvaluar = malloc(sizeof(tablaEnLista_struct));
					tablaAEvaluar = list_get(listaDeTablasDePaginas,j);
					for(int k = 0; k<list_size(tablaAEvaluar->tablaDePaginas);k++){
						paginaEnTabla_struct* paginaBuscada = malloc(sizeof(paginaEnTabla_struct));
						paginaBuscada = list_get(tablaAEvaluar->tablaDePaginas,k);
						if(paginaBuscada->frame == i){
							fprintf(dmp,"Marco:%d  Estado:Ocupado  Proceso:%d  Pagina:%d\n",i,tablaAEvaluar->idPatota,k);
						}
					}
				}

			}
		}
	}else{
		for(int i=0; i < list_size(listaDeTablasDePaginas);i++){
			tablaEnLista_struct *tablaDePaginas = malloc(sizeof(tablaEnLista_struct));
			tablaDePaginas = list_get(listaDeTablasDePaginas,i);
			for(int k=0; k<list_size(tablaDePaginas->tablaDePaginas);k++){
				segmentoEnTabla_struct* segmentoIterante = malloc(sizeof(segmentoEnTabla_struct));
				segmentoIterante = list_get(tablaDePaginas->tablaDePaginas,k);
				fprintf(dmp,"Proceso:%d  Segmento:%d  Inicio:%X  Tamanio:%dB \n",tablaDePaginas->idPatota,k,segmentoIterante->inicio,segmentoIterante->tamanio);
			}
		}
	}

	fclose(dmp);
}

void manejoDump(int signal){
	dumpDeMemoria();
}

void manejoCompactacion(int signal){
	compactacion();
}

