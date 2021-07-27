/*
 * funcionesMemoria.c
 *
 *  Created on: 21 jul. 2021
 *      Author: utnso
 */
#include "funcionesMemoria.h"

int menorEntreDos(int elem1, int elem2){
    if(elem1 > elem2){
        return elem2;
    }
    else
        return elem1;
}
int encontrarFrameDisponible(){
    for (int j = 0; j < cantidadPaginas; ++j) {
        if (bitarrayMemoria[j] == 0){
            return j;
        }
    }
    return -1;
}

int encontrarFrameEnSwapDisponible(){
    int i=0;
    while (bitarraySwap[i]==1){
        if (i==tamSwap/tamPagina){
            break;
        }
        i++;
    }
    return i;
}
int calcular_direccion_logica_archivo( int idPatota){
	pthread_mutex_lock(&mutexMemoria);
	tipoUniversal ='A';
	t_list *listaFiltrada;
	listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
	elementoEnLista_struct *elementoBuscado = malloc(sizeof(elementoEnLista_struct));
	for(int i =0; i< list_size(listaFiltrada);i++){
		elementoBuscado = list_get(listaFiltrada,i);
		if(elementoBuscado->ID==idPatota){
			break;
		}
	}
	pthread_mutex_unlock(&mutexMemoria);

	return (elementoBuscado->segmentoOPagina+elementoBuscado->offsetEnPagina);


}
int calcular_direccion_logica_patota(int idPatota){
	pthread_mutex_lock(&mutexMemoria);
	tipoUniversal ='P';
	t_list *listaFiltrada;
	listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
	elementoEnLista_struct *elementoBuscado = malloc(sizeof(elementoEnLista_struct));
	for(int i =0; i< list_size(listaFiltrada);i++){
		elementoBuscado = list_get(listaFiltrada,i);
		if(elementoBuscado->ID==idPatota){
			break;
		}
	}
	pthread_mutex_unlock(&mutexMemoria);

	return (elementoBuscado->segmentoOPagina+elementoBuscado->offsetEnPagina);
}

void guardar_en_memoria_general(void* payload,int idElemento,int tamPayload,int pid,char tipo){
    pthread_mutex_lock(&mutexMemoria);
    espacioLibre -= tamPayload;
    if (strcmp(esquemaMemoria,"PAGINACION")==0){
		guardar_en_memoria_paginacion(payload, idElemento, tamPayload, pid, tipo);

	}else if(strcmp(esquemaMemoria,"SEGMENTACION")==0){
		guardar_en_memoria_segmentacion( payload, idElemento, tamPayload, pid, tipo,tipoDeGuardado);
	}
    pthread_mutex_unlock(&mutexMemoria);


}

void* buscar_en_memoria_general(int idElementoABuscar,int PID, char tipo){
	void* retornar;
    pthread_mutex_lock(&mutexMemoria);
    if (strcmp(esquemaMemoria,"PAGINACION")==0){
		retornar =  buscar_en_memoria_paginacion( idElementoABuscar, PID,  tipo);

	}else if(strcmp(esquemaMemoria,"SEGMENTACION")==0){
		retornar =  buscar_de_memoria_segmentacion( idElementoABuscar, PID,tipo);
	}
    pthread_mutex_unlock(&mutexMemoria);
	return retornar;



}
void *borrar_de_memoria_general(int idElemento, int idPatota, char tipo){
    pthread_mutex_lock(&mutexMemoria);

	if (strcmp(esquemaMemoria,"PAGINACION")==0){
		borrar_de_memoria_paginacion( idElemento, idPatota,  tipo);

	}else if(strcmp(esquemaMemoria,"SEGMENTACION")==0){
		borrar_de_memoria_segmentacion(idElemento,idPatota,tipo);
	}
    pthread_mutex_unlock(&mutexMemoria);


}

void guardar_en_memoria_paginacion(void* payload,int idElemento,int tamPayload,int pid,char tipo){
    int indicePaginaCorrespondiente;
    int indiceTablaCorrespondiente;
    elementoEnLista_struct *nuevoElemento= malloc(sizeof(elementoEnLista_struct));
    nuevoElemento->tipo = tipo;
    nuevoElemento->PID = pid;
    tablaEnLista_struct *tablaCorrespondiente = malloc(sizeof(tablaEnLista_struct));
    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaEnLista_struct *tablaIterante;
        tablaIterante = list_get(listaDeTablasDePaginas,i);
        if(tablaIterante->idPatota == pid){
            tablaCorrespondiente = tablaIterante;
            indiceTablaCorrespondiente = i;
            break;
        }

    }

    paginaEnTabla_struct *paginaParcialmenteLlena = malloc(sizeof (paginaEnTabla_struct));
    paginaParcialmenteLlena->frame = -1;
    for (int i = 0; i < list_size(tablaCorrespondiente->tablaDePaginas); ++i) {
        paginaEnTabla_struct *paginaIterante;
        paginaIterante = list_get(tablaCorrespondiente->tablaDePaginas,i);
        if(paginaIterante->espacioOcupado < tamPagina && paginaIterante->presencia==1){
            paginaParcialmenteLlena = paginaIterante;
            indicePaginaCorrespondiente = i;
            break;
        }

    }

    int payLoadYaGuardado = 0;
    int *direccionFisica;
    if (paginaParcialmenteLlena->frame != -1){
        paginaParaReemplazar_struct *paginaReemplazable3 = malloc(sizeof(paginaParaReemplazar_struct));
        paginaReemplazable3->uso=1;
        paginaReemplazable3->PID=pid;
        direccionFisica = (int)memoria + (int)(paginaParcialmenteLlena->frame * tamPagina + paginaParcialmenteLlena->espacioOcupado);
        int menorEntre2 = menorEntreDos(tamPayload,(tamPagina-paginaParcialmenteLlena->espacioOcupado));
        memcpy(direccionFisica,payload,menorEntre2);
        nuevoElemento->offsetEnPagina = paginaParcialmenteLlena->espacioOcupado;
        nuevoElemento->segmentoOPagina = indicePaginaCorrespondiente;
        nuevoElemento->tamanio = tamPayload;
        nuevoElemento->ID = idElemento;
        list_add(listaElementos,nuevoElemento);
        payLoadYaGuardado += menorEntreDos(tamPayload,tamPagina-paginaParcialmenteLlena->espacioOcupado);
        payload = (int)payload + menorEntreDos(tamPayload,tamPagina-paginaParcialmenteLlena->espacioOcupado);
        paginaParcialmenteLlena->espacioOcupado += menorEntreDos(tamPayload,tamPagina - paginaParcialmenteLlena->espacioOcupado);
        list_replace(tablaCorrespondiente->tablaDePaginas,indicePaginaCorrespondiente,paginaParcialmenteLlena);
        while (payLoadYaGuardado<tamPayload) {
            int frameDisponible = encontrarFrameDisponible();
            if (frameDisponible == -1){
                guardar_en_swap(payload,idElemento,tamPayload-payLoadYaGuardado,pid,tipo);
                break;
            }
            paginaReemplazable3->nroFrame = frameDisponible;
            direccionFisica = (int)memoria + (int)(frameDisponible * tamPagina);
            menorEntre2 = menorEntreDos(tamPagina,tamPayload-payLoadYaGuardado);
            memcpy(direccionFisica,payload, menorEntre2);
            paginaEnTabla_struct *nuevaPagina = malloc(sizeof (paginaEnTabla_struct));
            nuevaPagina->frame = frameDisponible;
            nuevaPagina->presencia = 1;
            nuevaPagina->espacioOcupado = menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
            payLoadYaGuardado = (int)payLoadYaGuardado + menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
            payload = (int)payload + menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
            paginaReemplazable3->nroPagina = list_add(tablaCorrespondiente->tablaDePaginas,nuevaPagina);
            //log_info(logger,"PID que le cargo a la tabla de frames en el guardado: %d",paginaReemplazable3->PID);
            queue_push(tablaDeFrames,paginaReemplazable3);
            bitarrayMemoria[frameDisponible] = 1;
            paginaParaReemplazar_struct *paginaDePrueba = malloc(sizeof(paginaParaReemplazar_struct));

        }

    }
    else {
        int frameDisponible = encontrarFrameDisponible();
        paginaParaReemplazar_struct *paginaReemplazable = malloc(sizeof(paginaParaReemplazar_struct));
        paginaReemplazable->uso=1;
        paginaReemplazable->PID=pid;
        if (frameDisponible == -1) {
            guardar_en_swap(payload, idElemento, tamPayload, pid, tipo);
        } else{
            int *direccionFisica = memoria + (frameDisponible * tamPagina);
            memcpy(direccionFisica, payload, menorEntreDos(tamPayload - payLoadYaGuardado, tamPagina));
            paginaEnTabla_struct *nuevaPagina = malloc(sizeof(paginaEnTabla_struct));
            nuevaPagina->frame = frameDisponible;
            nuevaPagina->presencia = 1;
            nuevaPagina->espacioOcupado = menorEntreDos(tamPayload - payLoadYaGuardado, tamPagina);
            paginaReemplazable->nroFrame = frameDisponible;
            payload = (int) payload + menorEntreDos(tamPayload - payLoadYaGuardado, tamPagina);
            payLoadYaGuardado = (int) payLoadYaGuardado + menorEntreDos(tamPayload - payLoadYaGuardado, tamPagina);
            nuevoElemento->segmentoOPagina = list_add(tablaCorrespondiente->tablaDePaginas, nuevaPagina);
            nuevoElemento->offsetEnPagina = 0;
            nuevoElemento->tamanio = tamPayload;
            nuevoElemento->ID = idElemento;
            list_add(listaElementos,nuevoElemento);
            paginaReemplazable->nroPagina = nuevoElemento->segmentoOPagina;
            bitarrayMemoria[frameDisponible] = 1;
            queue_push(tablaDeFrames, paginaReemplazable);
            paginaParaReemplazar_struct *paginaDePrueba = malloc(sizeof(paginaParaReemplazar_struct));
            while (payLoadYaGuardado < tamPayload) {
                paginaParaReemplazar_struct *paginaReemplazable2 = malloc(sizeof(paginaParaReemplazar_struct));
                paginaReemplazable2->uso=1;
                paginaReemplazable2->PID=pid;
                int frameDisponible = encontrarFrameDisponible();
                if (frameDisponible == -1) {
                    guardar_en_swap(payload, idElemento, tamPayload-payLoadYaGuardado, pid, tipo);
                    break;
                }else{
					int *direccionFisica = memoria + (frameDisponible * tamPagina);
					memcpy(direccionFisica, payload, menorEntreDos(tamPayload - payLoadYaGuardado, tamPagina));
					paginaEnTabla_struct *nuevaPagina = malloc(sizeof(paginaEnTabla_struct));
					nuevaPagina->frame = frameDisponible;
					nuevaPagina->presencia = 1;
					nuevaPagina->espacioOcupado = menorEntreDos(tamPayload - payLoadYaGuardado, tamPagina);
					paginaReemplazable2->nroFrame = frameDisponible;
					payload += menorEntreDos(tamPayload - payLoadYaGuardado, tamPagina);
					payLoadYaGuardado += menorEntreDos(tamPayload - payLoadYaGuardado, tamPagina);
					paginaReemplazable2->nroPagina = list_add(tablaCorrespondiente->tablaDePaginas, nuevaPagina);
					queue_push(tablaDeFrames, paginaReemplazable2);
					bitarrayMemoria[frameDisponible] = 1;
                }
            }
        }
    }
    /*memcpy(memoria,payload, tamPayload/2);
    printf("Memoria: %d \n", memoria);
    printf("memoria + 500: %d \n", memoria+500);
    payload += tamPayload/2;
    memcpy(memoria+200,payload, tamPayload/2);
    void* payloadRecompuesto = malloc(tamPayload);
    printf("payloadRecompuesto puntero inicial: %d\n", payloadRecompuesto);
    memcpy(payloadRecompuesto,memoria, tamPayload/2);
    memcpy((payloadRecompuesto+tamPayload/2),memoria+200, tamPayload/2);
    tripulante_struct *payloadMegaCasteado = malloc(tamPayload);
    payloadMegaCasteado = payloadRecompuesto;
    printf("puntero payload recompuesto: %d\n",payloadRecompuesto);
    printf("Pos X del payload recompuesto: %d \n",payloadMegaCasteado->posx);*/
}


void guardar_en_swap(void* payload,int idElemento,int tamPayload,int pid,char tipo){
    int indicePaginaCorrespondiente;
    int indiceTablaCorrespondiente;

    elementoEnLista_struct *nuevoElemento= malloc(sizeof(elementoEnLista_struct));
    nuevoElemento->tipo = 'M';

    for(int i=0;i<list_size(listaElementos);i++){
    	elementoEnLista_struct *elementoIterante= malloc(sizeof(elementoEnLista_struct));
    	elementoIterante = list_get(listaElementos,i);
    	if((elementoIterante->ID == idElemento) && (elementoIterante->tipo == tipo)){
    		nuevoElemento = elementoIterante;
    		break;
    	}
    }




    tablaEnLista_struct *tablaCorrespondiente = malloc(sizeof(tablaEnLista_struct));

    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaEnLista_struct *tablaIterante;
        tablaIterante = list_get(listaDeTablasDePaginas,i);
        if(tablaIterante->idPatota == pid){
            tablaCorrespondiente = tablaIterante;
            indiceTablaCorrespondiente = i;
            break;
        }

    }

    paginaEnTabla_struct *paginaParcialmenteLlena = malloc(sizeof (paginaEnTabla_struct));
    paginaParcialmenteLlena->frame = -1;
    for (int i = 0; i < list_size(tablaCorrespondiente->tablaDePaginas); ++i) {
        paginaEnTabla_struct *paginaIterante;
        paginaIterante = list_get(tablaCorrespondiente->tablaDePaginas,i);
        if(paginaIterante->espacioOcupado < tamPagina && paginaIterante->presencia==0){
            paginaParcialmenteLlena = paginaIterante;
            indicePaginaCorrespondiente = i;
            break;
        }

    }

    int payLoadYaGuardado = 0;
    int *direccionFisica;

    if (paginaParcialmenteLlena->frame != -1){

        direccionFisica = (int)memoriaSwap + (int)(paginaParcialmenteLlena->frame * tamPagina + paginaParcialmenteLlena->espacioOcupado);
        int menorEntre2 = menorEntreDos(tamPayload,(tamPagina-paginaParcialmenteLlena->espacioOcupado));
        memcpy(direccionFisica,payload,menorEntre2);
        if(nuevoElemento->tipo=='M'){
        	nuevoElemento->tipo = tipo;
        	nuevoElemento->offsetEnPagina = paginaParcialmenteLlena->espacioOcupado;
        	nuevoElemento->segmentoOPagina = indicePaginaCorrespondiente;
        	nuevoElemento->tamanio = tamPayload;
        	nuevoElemento->ID = idElemento;
        	list_add(listaElementos,nuevoElemento);
        }

        payLoadYaGuardado += menorEntreDos(tamPayload,tamPagina-paginaParcialmenteLlena->espacioOcupado);
        payload = (int)payload + menorEntreDos(tamPayload,tamPagina-paginaParcialmenteLlena->espacioOcupado);
        paginaParcialmenteLlena->espacioOcupado += menorEntreDos(tamPayload,tamPagina - paginaParcialmenteLlena->espacioOcupado);
        list_replace(tablaCorrespondiente->tablaDePaginas,indicePaginaCorrespondiente,paginaParcialmenteLlena);
        while (payLoadYaGuardado<tamPayload) {
            int frameDisponible = encontrarFrameEnSwapDisponible();
            direccionFisica = (int)memoriaSwap + (int)(frameDisponible * tamPagina);
            menorEntre2 = menorEntreDos(tamPagina,tamPayload-payLoadYaGuardado);
            memcpy(direccionFisica,payload, menorEntre2);
            paginaEnTabla_struct *nuevaPagina = malloc(sizeof (paginaEnTabla_struct));
            nuevaPagina->frame = frameDisponible;
            nuevaPagina->presencia = 0;
            nuevaPagina->espacioOcupado = menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
            payLoadYaGuardado = (int)payLoadYaGuardado + menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
            payload = (int)payload + menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
            list_add(tablaCorrespondiente->tablaDePaginas,nuevaPagina);
            bitarraySwap[frameDisponible] = 1;
        }

    }
    else{
        int frameDisponible = encontrarFrameEnSwapDisponible();
        int *direccionFisica =(int) memoriaSwap + (frameDisponible * tamPagina);
        char* payloadProbando;
        payloadProbando=payload;
        //printf("Payload la concha de tu madre: %s\n",payloadProbando);
        memcpy(direccionFisica,payload, menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina));
        paginaEnTabla_struct *nuevaPagina = malloc(sizeof (paginaEnTabla_struct));
        nuevaPagina->frame = frameDisponible;
        nuevaPagina->presencia = 0;
        nuevaPagina->espacioOcupado = menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
        list_add(tablaCorrespondiente->tablaDePaginas,nuevaPagina);
        payload =(int)payload + menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
        payLoadYaGuardado =(int)payLoadYaGuardado + menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
        if(nuevoElemento->tipo == 'M'){
        	nuevoElemento->tipo = tipo;
        	nuevoElemento->segmentoOPagina = list_add(tablaCorrespondiente->tablaDePaginas,nuevaPagina);
        	nuevoElemento->offsetEnPagina=0;
        	nuevoElemento->tamanio = tamPayload;
        	nuevoElemento->ID = idElemento;
        	list_add(listaElementos,nuevoElemento);
        }

        bitarraySwap[frameDisponible] = 1;

        while (payLoadYaGuardado<tamPayload) {

            int frameDisponible = encontrarFrameEnSwapDisponible();
            int *direccionFisica = (int) memoriaSwap + (frameDisponible * tamPagina);
            memcpy(direccionFisica,payload, menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina));
            paginaEnTabla_struct *nuevaPagina = malloc(sizeof (paginaEnTabla_struct));
            nuevaPagina->frame = frameDisponible;
            nuevaPagina->presencia=0;
            nuevaPagina->espacioOcupado = menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
            payload +=menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
            payLoadYaGuardado += menorEntreDos(tamPayload-payLoadYaGuardado,tamPagina);
            list_add(tablaCorrespondiente->tablaDePaginas,nuevaPagina);
            bitarraySwap[frameDisponible] = 1;
        }

    }
    msync(memoriaSwap, tamSwap, MS_SYNC);
}




void* minimo_segmentos_en_tabla(void *elem1,void*elem2){
    segmentoEnTabla_struct *elem1bis = elem1;
    segmentoEnTabla_struct *elem2bis = elem2;
    if(elem1bis->inicio < elem2bis->inicio){
        return elem1bis;
    } else{
        return elem2bis;
    }

}
void* minimo_hueco_libre(void *elem1,void*elem2){
    espacio_struct *elem1bis = elem1;
    espacio_struct *elem2bis = elem2;
    if(elem1bis->tamanio < elem2bis->tamanio){
        return elem1bis;
    } else{
        return elem2bis;
    }

}
bool ordenar_por_posicion_local(void *tam1, void *tam2){
    segmentoEnTabla_struct *tam1bis = tam1;
    segmentoEnTabla_struct *tam2bis = tam2;
    return (tam1bis->inicio < tam2bis->inicio);

}

bool ordenar_por_posicion_global(void *tam1, void *tam2){
    segmentoEnTablaGlobal_struct *tam1bis = tam1;
    segmentoEnTablaGlobal_struct  *tam2bis = tam2;
    return (tam1bis->inicio < tam2bis->inicio);

}

bool ordenar_por_nro_frame(void *pag1, void *pag2){
    paginaParaReemplazar_struct *pag1bis = pag1;
    paginaParaReemplazar_struct  *pag2bis = pag2;
    return (pag1bis->nroFrame < pag2bis->nroFrame);

}

bool filtrarPorTipo(void* elemento){
    elementoEnLista_struct *comparador = elemento;
    return comparador->tipo == tipoUniversal;
}

void traerPaginaAMemoria(paginaEnTabla_struct* paginaATraer, t_list* tablaDePaginas,int indiceDeLaPaginaATraer,int PID){
    int frameEnSwap = paginaATraer->frame;
    //log_info(logger, "Cantidad de frames al entrar a traer pagina a memoria: %d \n",list_size(tablaDeFrames->elements));
    paginaParaReemplazar_struct *paginaAReemplazar = malloc(sizeof(paginaParaReemplazar_struct));
    if (strcmp(alg_remplazo,"LRU")==0){
        paginaAReemplazar = queue_pop(tablaDeFrames);
    } else{
        while (1) {
        	list_sort(tablaDeFrames->elements,ordenar_por_nro_frame);
            paginaAReemplazar = list_get(tablaDeFrames->elements, punteroReemplazo);
           // log_info(logger, "PID de la pagina a reemplazar: %d\n",paginaAReemplazar->PID);
            if (paginaAReemplazar->uso==1){
                paginaAReemplazar->uso = 0;
                //log_info(logger, "PID de la pagina a reemplazar 2: %d\n",paginaAReemplazar->PID);
                list_replace(tablaDeFrames->elements,punteroReemplazo,paginaAReemplazar);
                if (punteroReemplazo+1 == queue_size(tablaDeFrames)){
                    punteroReemplazo = 0;
                } else{
                    punteroReemplazo++;
                }
            } else{
                list_remove(tablaDeFrames->elements,punteroReemplazo);
                if(punteroReemplazo == queue_size(tablaDeFrames)){
                	punteroReemplazo = 0;
                }
                break;
            }
        }

    }

    int* direccionFisicaPaginaEnSwap = (int)memoriaSwap + (paginaATraer->frame * tamPagina);
    int* direccionFisicaPaginaEnMemoria = (int) memoria + (paginaAReemplazar->nroFrame * tamPagina);
    void* direccionAuxiliar = malloc(tamPagina);

    memcpy(direccionAuxiliar,direccionFisicaPaginaEnMemoria,tamPagina);
    memcpy(direccionFisicaPaginaEnMemoria,direccionFisicaPaginaEnSwap,tamPagina);
    memcpy(direccionFisicaPaginaEnSwap,direccionAuxiliar,tamPagina);
    paginaEnTabla_struct *paginaAActualizar = malloc(sizeof(paginaEnTabla_struct));
    paginaAActualizar = list_get(tablaDePaginas,indiceDeLaPaginaATraer);
    paginaAActualizar->presencia = 1;
    paginaAActualizar->frame = paginaAReemplazar->nroFrame;
    list_replace(tablaDePaginas,indiceDeLaPaginaATraer,paginaAActualizar);
    paginaParaReemplazar_struct *paginaAReponer = malloc(sizeof(paginaParaReemplazar_struct));
    paginaAReponer->nroFrame=paginaAReemplazar->nroFrame;
    paginaAReponer->nroPagina = indiceDeLaPaginaATraer;
    paginaAReponer->uso = 1;
    paginaAReponer->PID = PID;
    //log_info(logger, "PID antes de pushear %d\n",paginaAReponer->PID);
    queue_push(tablaDeFrames,paginaAReponer);
    tablaEnLista_struct *tablaBuscada = malloc(sizeof(tablaEnLista_struct));
    t_list *tablaDePaginasBuscada;
    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaBuscada = list_get(listaDeTablasDePaginas,i);
        if (tablaBuscada->idPatota == paginaAReemplazar->PID){
            tablaDePaginasBuscada = tablaBuscada->tablaDePaginas;
            //log_info(logger, "Entre al if\n");
            break;
        }
        //log_info(logger, "Buscando la tabla de la patota: %d y estoy en la de la %d\n",paginaAReemplazar->PID,tablaBuscada->idPatota);
    }

    paginaEnTabla_struct *paginaAActualizar2 = malloc(sizeof(paginaEnTabla_struct));
    //log_info(logger, "Voy a intentar traer el indice: %d y la tabla de paginas tiene %d elementos\n",paginaAReemplazar->nroPagina,list_size(tablaDePaginasBuscada));
    paginaAActualizar2 = list_get(tablaDePaginasBuscada,paginaAReemplazar->nroPagina);
    paginaAActualizar2->presencia = 0;
    paginaAActualizar2->frame = frameEnSwap;
    list_replace(tablaDePaginasBuscada,paginaAReemplazar->nroPagina,paginaAActualizar2);
    //log_info(logger, "Cantidad de frames al salir de traer pagina a memoria: %d \n",list_size(tablaDeFrames->elements));
    //free(direccionAuxiliar);
}

void* buscar_en_memoria_paginacion(int idElementoABuscar,int PID, char tipo){
	//log_info(logger, "Cantidad de frames al entrar a buscar en memoria: %d \n",list_size(tablaDeFrames->elements));
	tipoUniversal = tipo;
    t_list* listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
    elementoEnLista_struct *elementoEvaluado = malloc(sizeof(elementoEnLista_struct));
    int paginaInicial= -1,offset=-1,tamanioPayload=-1;
    for (int i = 0; i < list_size(listaFiltrada); ++i) {
        elementoEvaluado = list_get(listaFiltrada,i);
        if (elementoEvaluado->ID == idElementoABuscar){
            paginaInicial = elementoEvaluado->segmentoOPagina;
            offset = elementoEvaluado->offsetEnPagina;
            tamanioPayload = elementoEvaluado->tamanio;
            break;
        }
    }
    if (paginaInicial == -1){
        return 0;
    }
    tablaEnLista_struct *tablaBuscada = malloc(sizeof(tablaEnLista_struct));
    t_list *tablaDePaginas;
    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaBuscada = list_get(listaDeTablasDePaginas,i);
        if (tablaBuscada->idPatota == PID){
            tablaDePaginas = tablaBuscada->tablaDePaginas;
            break;
        }
    }

    void* payloadADevolver = malloc(tamanioPayload);
    int tamanioPorGuardar = tamanioPayload;
    paginaEnTabla_struct *paginaDeLectura = malloc(sizeof(paginaEnTabla_struct));
    int movimientoDepagina=1;
    paginaDeLectura = list_get(tablaDePaginas,paginaInicial);
    if (paginaDeLectura->presencia == 0){
        traerPaginaAMemoria(paginaDeLectura,tablaDePaginas,paginaInicial,PID);
        msync(memoriaSwap, tamSwap, MS_SYNC);
        paginaDeLectura = list_get(tablaDePaginas,paginaInicial);
    }else if(paginaDeLectura->presencia == 1){
        paginaParaReemplazar_struct *paginaAux = malloc(sizeof(paginaParaReemplazar_struct));
        //log_info(logger, "Pagina que estoy buscando: %d\n",paginaInicial);
        for(int j=0;j<list_size(tablaDeFrames->elements);j++){
        	paginaParaReemplazar_struct *paginaIterante = malloc(sizeof(paginaParaReemplazar_struct ));
        	paginaIterante = list_get(tablaDeFrames->elements,j);
        	if(paginaIterante->nroPagina == paginaInicial){
        		paginaAux = list_remove(tablaDeFrames->elements,j);
        		//log_info(logger, "PID en el list_remove: %d\n",paginaAux->PID,paginaAux->nroPagina);
        		paginaAux->uso = 1;
        		queue_push(tablaDeFrames,paginaAux);
        		break;
        	}
        }
        //log_info(logger, "PID antes del queue push: %d\n",paginaAux->PID,paginaAux->nroPagina);

    }
    int* direccionFisicaDeLaPagina;
    direccionFisicaDeLaPagina = (int)memoria + (paginaDeLectura->frame * tamPagina + offset);
    memcpy(payloadADevolver,direccionFisicaDeLaPagina, menorEntreDos(tamanioPayload,tamPagina-offset));
    payloadADevolver += menorEntreDos(tamanioPayload,tamPagina-offset);
    tamanioPorGuardar -= menorEntreDos(tamanioPayload,tamPagina-offset);
    while(tamanioPorGuardar>0){
        paginaDeLectura = list_get(tablaDePaginas,paginaInicial+movimientoDepagina);
        if (paginaDeLectura->presencia == 0){
            //printf("Frame a traer de swap a memoria: %d\n",paginaDeLectura->frame);
        	traerPaginaAMemoria(paginaDeLectura,tablaDePaginas, paginaInicial+movimientoDepagina,PID);
            msync(memoriaSwap, tamSwap, MS_SYNC);
            paginaDeLectura = list_get(tablaDePaginas,paginaInicial+movimientoDepagina);
        }else{
        	paginaParaReemplazar_struct *paginaAux = malloc(sizeof(paginaParaReemplazar_struct));
        	        for(int j=0;j<list_size(tablaDeFrames->elements);j++){
        	        	paginaParaReemplazar_struct *paginaIterante = malloc(sizeof(paginaParaReemplazar_struct ));
        	        	paginaIterante = list_get(tablaDeFrames->elements,j);
        	        	if(paginaIterante->nroPagina == paginaInicial){
        	        		paginaAux = list_remove(tablaDeFrames->elements,j);
        	        		//log_info(logger, "PID en el list_remove: %d\n",paginaAux->PID,paginaAux->nroPagina);
        	        		paginaAux->uso = 1;
        	        		queue_push(tablaDeFrames,paginaAux);
        	        		break;
        	        	}
        	        }
        	        //log_info(logger, "PID antes del queue push: %d\n",paginaAux->PID,paginaAux->nroPagina);

        }
        direccionFisicaDeLaPagina = (int)memoria + (paginaDeLectura->frame * tamPagina);
        memcpy(payloadADevolver,direccionFisicaDeLaPagina, menorEntreDos(tamanioPorGuardar,tamPagina));
        payloadADevolver += menorEntreDos(tamanioPorGuardar,tamPagina);
        tamanioPorGuardar -= menorEntreDos(tamanioPorGuardar,tamPagina);
        movimientoDepagina ++;
    }

    payloadADevolver -= tamanioPayload;
    //log_info(logger, "Cantidad de frames al salir de buscar en memoria: %d \n",list_size(tablaDeFrames->elements));
    return payloadADevolver;

}
void actualizarListaElementos(int paginaEliminada,int PID){
    for (int i = 0; i < list_size(listaElementos); ++i) {
        elementoEnLista_struct *elementoIterante = malloc(sizeof(elementoEnLista_struct));
        		elementoIterante = list_get(listaElementos,i);
            if((elementoIterante->segmentoOPagina > paginaEliminada) && (elementoIterante->PID == PID)){
                elementoIterante->segmentoOPagina -= 1;
                list_replace(listaElementos,i,elementoIterante);
            }
    }
}


void actualizarListaGlobalDeSegmentos(int paginaEliminada,int PID){
    for (int i = 0; i < list_size(listaGlobalDeSegmentos); ++i) {
        segmentoEnTablaGlobal_struct *segmentoIterante = malloc(sizeof(segmentoEnTablaGlobal_struct));
        		segmentoIterante = list_get(listaGlobalDeSegmentos,i);
        if((segmentoIterante->segmentoEnLocal > paginaEliminada) && (segmentoIterante->idPatota == PID)){
            segmentoIterante->segmentoEnLocal -= 1;
            list_replace(listaGlobalDeSegmentos,i,segmentoIterante);
        }
    }
}

void *borrar_de_memoria_paginacion(int idElemento, int idPatota, char tipo){
	tipoUniversal = tipo;
    t_list* listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
    elementoEnLista_struct *elementoEvaluado = malloc(sizeof(elementoEnLista_struct));
    int paginaInicial,offset,tamanioPayload,posicionElementoEvaluado;
    for (int i = 0; i < list_size(listaFiltrada); ++i) {
        elementoEvaluado = list_get(listaFiltrada,i);
        if (elementoEvaluado->ID == idElemento){
            paginaInicial = elementoEvaluado->segmentoOPagina;
            offset = elementoEvaluado->offsetEnPagina;
            tamanioPayload = elementoEvaluado->tamanio;
            posicionElementoEvaluado = i;
            break;
        }
    }
    espacioLibre += tamanioPayload;
    tablaEnLista_struct *tablaBuscada = malloc(sizeof(tablaEnLista_struct));
    t_list *tablaDePaginas;
    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaBuscada = list_get(listaDeTablasDePaginas,i);
        if (tablaBuscada->idPatota == idPatota){
            tablaDePaginas = tablaBuscada->tablaDePaginas;
            break;
        }
    }
    int payloadBorrado=0;
    if ((offset == 0 && tamanioPayload >= tamPagina) || (offset == 0 && paginaInicial == list_size(tablaDePaginas)-1)){
        paginaEnTabla_struct* paginaABorrar = malloc(sizeof(paginaEnTabla_struct));
    	paginaABorrar = list_get(tablaDePaginas, paginaInicial);
    	if (paginaABorrar->presencia == 1){
    		bitarrayMemoria[paginaABorrar->frame] = 0;
    	}else{
    		bitarraySwap[paginaABorrar->frame] = 0;
    	}

        list_remove_and_destroy_element(tablaDePaginas,paginaInicial,free);
        list_remove_and_destroy_element(listaElementos,posicionElementoEvaluado,free);
        actualizarListaElementos(paginaInicial,idPatota);
        payloadBorrado += tamPagina;
        actualizarListaElementos(paginaInicial,idPatota);
    }else if(offset == 0 && tamanioPayload < tamPagina){
    	paginaEnTabla_struct *paginaAActualizar = malloc(sizeof(paginaEnTabla_struct));
    	paginaAActualizar =	list_get(tablaDePaginas,paginaInicial);
    	paginaAActualizar->espacioOcupado -= tamanioPayload;
    	if (paginaAActualizar->espacioOcupado == 0){
    		if (paginaAActualizar->presencia == 1){
    		   bitarrayMemoria[paginaAActualizar->frame] = 0;
    		}else{
    		   bitarraySwap[paginaAActualizar->frame] = 0;
    		}
    		list_remove_and_destroy_element(tablaDePaginas,paginaInicial,free);
    	}else{
    	    list_replace(tablaDePaginas,paginaInicial,paginaAActualizar);
    	}
    	list_remove_and_destroy_element(listaElementos,posicionElementoEvaluado,free);
    	actualizarListaElementos(paginaInicial,idPatota);
    	payloadBorrado += tamanioPayload;
    }
    else if(offset!=0 && tamanioPayload >= tamPagina-offset){
        paginaEnTabla_struct *paginaAActualizar = malloc(sizeof(paginaEnTabla_struct));
        paginaAActualizar =	list_get(tablaDePaginas,paginaInicial);
        paginaAActualizar->espacioOcupado -= tamPagina-offset;
        if (paginaAActualizar->espacioOcupado == 0){
        	if (paginaAActualizar->presencia == 1){
        		bitarrayMemoria[paginaAActualizar->frame] = 0;
        	}else{
        		bitarraySwap[paginaAActualizar->frame] = 0;
        	}
        	list_remove(tablaDePaginas,paginaInicial);
        }else{
        	list_replace(tablaDePaginas,paginaInicial,paginaAActualizar);
        }

        list_remove_and_destroy_element(listaElementos,posicionElementoEvaluado,free);
        actualizarListaElementos(paginaInicial,idPatota);
        payloadBorrado += (tamPagina-offset);
    }else if(offset!=0 && tamanioPayload < tamPagina-offset){
    	paginaEnTabla_struct *paginaAActualizar = malloc(sizeof(paginaEnTabla_struct));
    	paginaAActualizar = list_get(tablaDePaginas,paginaInicial);
    	paginaAActualizar->espacioOcupado -= tamanioPayload;
    	if (paginaAActualizar->espacioOcupado == 0){
    		if (paginaAActualizar->presencia == 1){
    		   bitarrayMemoria[paginaAActualizar->frame] = 0;
    		}else{
    		   bitarraySwap[paginaAActualizar->frame] = 0;
    		}
    		list_remove_and_destroy_element(tablaDePaginas,paginaInicial,free);
    	}else{
    		list_replace(tablaDePaginas,paginaInicial,paginaAActualizar);
    	}
    	payloadBorrado += tamanioPayload;
    	list_remove_and_destroy_element(listaElementos,posicionElementoEvaluado,free);
    	actualizarListaElementos(paginaInicial,idPatota);
    }
    while(payloadBorrado != tamanioPayload){
    	paginaInicial++;
    	if ((tamanioPayload -  payloadBorrado >= tamPagina) || (paginaInicial == list_size(tablaDePaginas)-1)){
    		paginaEnTabla_struct* paginaABorrar = malloc(sizeof(paginaEnTabla_struct));
    		paginaABorrar = list_get(tablaDePaginas, paginaInicial);
    		if (paginaABorrar->presencia == 1){
    			bitarrayMemoria[paginaABorrar->frame] = 0;
    		}else{
    			bitarraySwap[paginaABorrar->frame] = 0;
    		}

    		list_remove_and_destroy_element(tablaDePaginas,paginaInicial,free);
    		payloadBorrado += tamPagina;
    		actualizarListaElementos(paginaInicial,idPatota);
    	}else if(tamanioPayload < tamPagina){
        	paginaEnTabla_struct *paginaAActualizar = malloc(sizeof(paginaEnTabla_struct));
        	paginaAActualizar =	list_get(tablaDePaginas,paginaInicial);
        	paginaAActualizar->espacioOcupado -= tamanioPayload;
        	if (paginaAActualizar->espacioOcupado == 0){
        		if (paginaAActualizar->presencia == 1){
        		   bitarrayMemoria[paginaAActualizar->frame] = 0;
        		}else{
        		   bitarraySwap[paginaAActualizar->frame] = 0;
        		}
        		list_remove_and_destroy_element(tablaDePaginas,paginaInicial,free);
        	}else{
        	    list_replace(tablaDePaginas,paginaInicial,paginaAActualizar);
        	}
        	list_remove_and_destroy_element(listaElementos,posicionElementoEvaluado,free);
        	actualizarListaElementos(paginaInicial,idPatota);
        	payloadBorrado += tamanioPayload;
        }

    }
}

void guardar_en_memoria_segmentacion(void* payload,int idElemento,int tamPayload,uint32_t pid,char tipo, int tipoDeGuardado)
{
    int huecoLibre;
    t_list *listaSegmentos;
    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaEnLista_struct *tablaIterante = malloc(sizeof(tablaEnLista_struct));
        tablaIterante = list_get(listaDeTablasDePaginas,i);
        if (tablaIterante->idPatota == pid){
            listaSegmentos=tablaIterante->tablaDePaginas;
        }
    }



    if (list_is_empty(listaGlobalDeSegmentos) == 1)
    {
        //Guarda directamente en memoria y lo agrega a la lista de segmentos
        //*(tripulante_struct *)memoria = *(tripulante_struct *)payload;
        memcpy(memoria,payload,tamPayload);
        segmentoEnTabla_struct *nuevoSegmento = malloc(sizeof(segmentoEnTabla_struct));
        segmentoEnTablaGlobal_struct *nuevoSegmentoGlobal = malloc(sizeof(segmentoEnTablaGlobal_struct));
        nuevoSegmentoGlobal->inicio=memoria;
        nuevoSegmentoGlobal->tamanio=tamPayload;
        nuevoSegmentoGlobal->idPatota=pid;
        nuevoSegmento->inicio = memoria;
        nuevoSegmento->tamanio = tamPayload;
        elementoEnLista_struct *elementoNuevo = malloc(sizeof(elementoEnLista_struct));
        int segmentoGuardado = list_add_sorted(listaSegmentos,nuevoSegmento,ordenar_por_posicion_local);
        elementoNuevo->segmentoOPagina = segmentoGuardado;
        nuevoSegmentoGlobal->segmentoEnLocal = segmentoGuardado;
        list_add_sorted(listaGlobalDeSegmentos,nuevoSegmentoGlobal,ordenar_por_posicion_global);
        elementoNuevo->offsetEnPagina=0;
        elementoNuevo->tipo = tipo;
        elementoNuevo->tamanio = tamPayload;
        elementoNuevo->ID = idElemento;
        elementoNuevo->PID=pid;
        log_info(logger,"Segmento en el que guardo las tareas: %d y el tipo: %c",elementoNuevo->segmentoOPagina,elementoNuevo->tipo);
        list_add(listaElementos,elementoNuevo);
    }
    else
    {
        int tamanioListaSegmentos = list_size(listaGlobalDeSegmentos);
        switch (tipoDeGuardado) {
            case FIRSTFIT: {
            	segmentoEnTablaGlobal_struct* segmentoGlobalAComparar = malloc(sizeof(segmentoEnTablaGlobal_struct));
            	segmentoGlobalAComparar = list_get(listaGlobalDeSegmentos, 0);
            	if (segmentoGlobalAComparar->inicio != memoria){
                    huecoLibre = (int)segmentoGlobalAComparar->inicio - (int)memoria;
                    if (huecoLibre >= tamPayload){
                        segmentoEnTabla_struct *nuevoSegmento = malloc(sizeof(segmentoEnTabla_struct));
                        segmentoEnTablaGlobal_struct *nuevoSegmentoGlobal = malloc(sizeof(segmentoEnTablaGlobal_struct));
                        nuevoSegmentoGlobal->inicio=memoria;
                        nuevoSegmentoGlobal->tamanio=tamPayload;
                        nuevoSegmentoGlobal->idPatota=pid;
                        nuevoSegmento->inicio = memoria;
                        nuevoSegmento->tamanio = tamPayload;
                        elementoEnLista_struct *elementoNuevo = malloc(sizeof(elementoEnLista_struct));
                        elementoNuevo->segmentoOPagina = list_add_sorted(listaSegmentos,nuevoSegmento,ordenar_por_posicion_local);
                        nuevoSegmentoGlobal->segmentoEnLocal = elementoNuevo->segmentoOPagina;
                        list_add_sorted(listaGlobalDeSegmentos,nuevoSegmentoGlobal,ordenar_por_posicion_global);
                        elementoNuevo->offsetEnPagina=0;
                        elementoNuevo->tipo = tipo;
                        elementoNuevo->tamanio = tamPayload;
                        elementoNuevo->ID = idElemento;
                        elementoNuevo->PID = pid;
                        log_info(logger,"Segmento en el que guardo las tareas: %d y el tipo: %c y el ID de la patota",elementoNuevo->segmentoOPagina,elementoNuevo->tipo,elementoNuevo->PID);
                        actualizar_lista_elementos_segmentacion(elementoNuevo->segmentoOPagina,pid);
                        list_add(listaElementos,elementoNuevo);
                        break;
                    }

                }
                for (int i = 0; i < tamanioListaSegmentos; i++) {
                    segmentoEnTablaGlobal_struct *segmentoIterante = malloc(sizeof(segmentoEnTablaGlobal_struct));
                    segmentoIterante = list_get(listaGlobalDeSegmentos, i);
                    if (i + 1 == (list_size(listaGlobalDeSegmentos))) {

                        huecoLibre = ((int)memoria + tamMemoria) -  ((int)segmentoIterante->inicio + segmentoIterante->tamanio);
                    } else{

                        segmentoEnTablaGlobal_struct *segmentoSiguiente = list_get(listaGlobalDeSegmentos, i + 1);
                        huecoLibre =((int)segmentoSiguiente->inicio) - ((int)segmentoIterante->inicio + segmentoIterante->tamanio);
                    }

                    if (tamPayload <= huecoLibre) {

                        int *posicionInicioHuecoLibre = (int)(segmentoIterante->inicio) + (segmentoIterante->tamanio);
                        //*(tripulante_struct *) posicionInicioHuecoLibre = tcb;
                        memcpy(posicionInicioHuecoLibre,payload,tamPayload);
                        segmentoEnTabla_struct *nuevoSegmento = malloc(sizeof(segmentoEnTabla_struct));
                        segmentoEnTablaGlobal_struct *nuevoSegmentoGlobal = malloc(sizeof(segmentoEnTablaGlobal_struct));
                        nuevoSegmentoGlobal->inicio=posicionInicioHuecoLibre;
                        nuevoSegmentoGlobal->tamanio=tamPayload;
                        nuevoSegmentoGlobal->idPatota=pid;
                        nuevoSegmento->inicio = posicionInicioHuecoLibre;
                        nuevoSegmento->tamanio = tamPayload;
                        elementoEnLista_struct *elementoNuevo = malloc(sizeof(elementoEnLista_struct));
                        elementoNuevo->segmentoOPagina = list_add_sorted(listaSegmentos,nuevoSegmento,ordenar_por_posicion_local);
                        nuevoSegmentoGlobal->segmentoEnLocal = elementoNuevo->segmentoOPagina;
                        list_add_sorted(listaGlobalDeSegmentos,nuevoSegmentoGlobal,ordenar_por_posicion_global);
                        elementoNuevo->offsetEnPagina = 0;
                        elementoNuevo->tipo = tipo;
                        elementoNuevo->tamanio = tamPayload;
                        elementoNuevo->ID = idElemento;
                        elementoNuevo->PID = pid;
                        log_info(logger,"Tipo de lo que guardo: %c",elementoNuevo->tipo);
                        log_info(logger,"Segmento que le pongo a la lista de elementos: %d",elementoNuevo->segmentoOPagina);
                        log_info(logger,"Inicio del segmento en la lista de segmentos: %d",nuevoSegmento->inicio);
                        log_info(logger,"Inicio del segmento en la lista global de segmentos: %d",nuevoSegmentoGlobal->inicio);
                        actualizar_lista_elementos_segmentacion(elementoNuevo->segmentoOPagina,pid);
                        list_add(listaElementos,elementoNuevo);
                        break;
                    }

                }
                break;
            }

            case BESTFIT:{

                t_list *listaDeEspaciosLibres;
                listaDeEspaciosLibres = list_create();
                if ((int)((segmentoEnTablaGlobal_struct *)(list_get(listaGlobalDeSegmentos, 0)))->inicio != memoria){
                	segmentoEnTablaGlobal_struct* segmentoIteranteSeg = list_get(listaGlobalDeSegmentos, 0);
                	huecoLibre = (int)segmentoIteranteSeg->inicio - (int)memoria;
                    if (huecoLibre >= tamPayload){
                        espacio_struct *nuevoHuecoLibre = malloc(sizeof (espacio_struct)) ;
                        nuevoHuecoLibre->tamanio = huecoLibre;
                        nuevoHuecoLibre->ptrHuecoLibre = memoria;
                        list_add(listaDeEspaciosLibres,nuevoHuecoLibre);

                    }
                }
                for (int i = 0; i < list_size(listaGlobalDeSegmentos); i++) {
                    segmentoEnTablaGlobal_struct *segmentoIterante;
                    segmentoIterante = list_get(listaGlobalDeSegmentos, i);

                    if (i + 1 == (list_size(listaGlobalDeSegmentos))) {

                        huecoLibre = ((int)memoria + tamMemoria) - ((int)segmentoIterante->inicio + segmentoIterante->tamanio);
                    } else {

                        segmentoEnTablaGlobal_struct *segmentoSiguiente = list_get(listaGlobalDeSegmentos, i + 1);
                        huecoLibre = (int)segmentoSiguiente->inicio - ((int)segmentoIterante->inicio + segmentoIterante->tamanio);
                    }

                    if (tamPayload <= huecoLibre) {
                        int *posicionInicioHuecoLibre = (int)segmentoIterante->inicio + segmentoIterante->tamanio;
                        espacio_struct *nuevoHuecoLibre = malloc(sizeof (espacio_struct)) ;
                        nuevoHuecoLibre->tamanio = huecoLibre;
                        nuevoHuecoLibre->ptrHuecoLibre = posicionInicioHuecoLibre;
                        list_add(listaDeEspaciosLibres,nuevoHuecoLibre);
                    }


                }
                if (list_is_empty(listaDeEspaciosLibres) == 1){
                    compactacion();

                }
                else{
                    espacio_struct *punteroHuecoMinimo;
                    punteroHuecoMinimo = list_get_minimum(listaDeEspaciosLibres,minimo_hueco_libre);
                    //*(tripulante_struct *) punteroHuecoMinimo->ptrHuecoLibre = tcb;
                    memcpy(punteroHuecoMinimo->ptrHuecoLibre,payload,tamPayload);
                    segmentoEnTabla_struct *nuevoSegmento = malloc(sizeof(segmentoEnTabla_struct));
                    segmentoEnTablaGlobal_struct *nuevoSegmentoGlobal = malloc(sizeof(segmentoEnTablaGlobal_struct));
                    nuevoSegmentoGlobal->inicio=punteroHuecoMinimo->ptrHuecoLibre;
                    nuevoSegmentoGlobal->tamanio=tamPayload;
                    nuevoSegmentoGlobal->idPatota=pid;
                    nuevoSegmento->inicio = punteroHuecoMinimo->ptrHuecoLibre;
                    nuevoSegmento->tamanio = tamPayload;
                    elementoEnLista_struct *elementoNuevo = malloc(sizeof(elementoEnLista_struct));
                    int segmentoGuardado = list_add_sorted(listaSegmentos,nuevoSegmento,ordenar_por_posicion_local);
                    elementoNuevo->segmentoOPagina = segmentoGuardado;
                    nuevoSegmentoGlobal->segmentoEnLocal = elementoNuevo->segmentoOPagina;
                    list_add_sorted(listaGlobalDeSegmentos,nuevoSegmentoGlobal,ordenar_por_posicion_global);
                    elementoNuevo->offsetEnPagina=0;
                    elementoNuevo->tipo = tipo;
                    elementoNuevo->tamanio = tamPayload;
                    elementoNuevo->ID = idElemento;
                    elementoNuevo->PID = pid;
                    actualizar_lista_elementos_segmentacion(elementoNuevo->segmentoOPagina,pid);
                    list_add(listaElementos,elementoNuevo);

                }
                list_destroy(listaDeEspaciosLibres);
                break;
            }
        }
    }
}

void borrar_de_memoria_segmentacion(int idElementoABorrar, int idPatota, char tipoDeElemento){

    tipoUniversal = tipoDeElemento;
    t_list *listaSegmentos;
    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaEnLista_struct *tablaIterante = malloc(sizeof(tablaEnLista_struct));
        tablaIterante = list_get(listaDeTablasDePaginas,i);
        if (tablaIterante->idPatota == idPatota){
            listaSegmentos=tablaIterante->tablaDePaginas;
        }
    }
    for (int i=0;i< list_size(listaElementos);i++){
        elementoEnLista_struct *elementoEvaluado = list_get(listaElementos,i);
        segmentoEnTabla_struct *segmentoEvaluado = list_get(listaSegmentos,elementoEvaluado->segmentoOPagina);

            if ((elementoEvaluado->ID == idElementoABorrar) && (elementoEvaluado->tipo == tipoDeElemento)){
                segmentoEnTablaGlobal_struct *segmentoGlobalIterante = malloc(sizeof(segmentoGlobalIterante));
                for (int j = 0; j < list_size(listaGlobalDeSegmentos); ++j) {
                    segmentoGlobalIterante = list_get(listaGlobalDeSegmentos,j);
                    if ((segmentoGlobalIterante->idPatota == idPatota)&&(segmentoGlobalIterante->segmentoEnLocal == elementoEvaluado->segmentoOPagina)){
                        list_remove(listaGlobalDeSegmentos,j);
                        actualizarListaElementos(elementoEvaluado->segmentoOPagina,idPatota);
                        actualizarListaGlobalDeSegmentos(elementoEvaluado->segmentoOPagina,idPatota);
                        break;
                    }
                }
                list_remove(listaSegmentos,elementoEvaluado->segmentoOPagina);
                list_remove(listaElementos,i);
                break;
            }


    }

}

void *buscar_de_memoria_segmentacion(int idElementoABuscar,int idPatota, char tipoDeElemento){
    tipoUniversal = tipoDeElemento;
    t_list* listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
    t_list *listaSegmentos;
    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaEnLista_struct *tablaIterante = malloc(sizeof(tablaEnLista_struct));
        tablaIterante = list_get(listaDeTablasDePaginas,i);
        if (tablaIterante->idPatota == idPatota){
            listaSegmentos=tablaIterante->tablaDePaginas;
            break;
        }
    }
    for (int s=0;s< list_size(listaFiltrada);s++){
        elementoEnLista_struct *elementoEvaluado = malloc(sizeof(elementoEnLista_struct));
        elementoEvaluado= list_get(listaFiltrada,s);
        segmentoEnTabla_struct *segmentoEvaluado = malloc(sizeof(segmentoEnTabla_struct));
        segmentoEvaluado = list_get(listaSegmentos,elementoEvaluado->segmentoOPagina);
        log_info(logger,"Estoy buscando las tareas");
        if (tipoDeElemento == 'T'){
            tcb* elementoABuscar = malloc(sizeof(tcb));
            //elementoABuscar = (tripulante_struct*)segmentoEvaluado->inicio;
            memcpy(elementoABuscar,segmentoEvaluado->inicio, sizeof(tcb));
            if (elementoABuscar->id == idElementoABuscar && elementoEvaluado->tipo=='T'){
                return elementoABuscar;
            }else{
                //free(elementoEvaluado);
                //free(segmentoEvaluado);
            }
        }else if(tipoDeElemento == 'A'){
            char *elementoABuscar = malloc(elementoEvaluado->tamanio);
            log_info(logger,"Tipo del elemento evaluado: %c y PID %d",elementoEvaluado->tipo,elementoEvaluado->PID);
            if (elementoEvaluado->ID == idElementoABuscar && elementoEvaluado->tipo == 'A'){
                memcpy(elementoABuscar,segmentoEvaluado->inicio, elementoEvaluado->tamanio);
                log_info(logger,"Encontre las tareas en la direccion %d y son de tamanio %d",segmentoEvaluado->inicio,elementoEvaluado->tamanio);
                return elementoABuscar;

            }
        }else if(tipoDeElemento == 'P'){
            pcb *elementoABuscar = malloc(sizeof(pcb));
            if (elementoEvaluado->ID == idElementoABuscar && elementoEvaluado->tipo == 'P'){
                memcpy(elementoABuscar,segmentoEvaluado->inicio, elementoEvaluado->tamanio);
                return elementoABuscar;
            }
        }
    }
}

void compactacion(){
    for (int i =0;i<list_size(listaGlobalDeSegmentos);i++){
        if(i==0){
            segmentoEnTablaGlobal_struct *primerSegmento = malloc(sizeof(segmentoEnTablaGlobal_struct));
            primerSegmento = list_get(listaGlobalDeSegmentos,0);
            if(primerSegmento->inicio != memoria){
                memcpy(memoria,primerSegmento->inicio,primerSegmento->tamanio);
                primerSegmento->inicio = memoria;
                list_replace(listaGlobalDeSegmentos,0,primerSegmento);


                t_list *listaSegmentosLocal = malloc(sizeof(tablaEnLista_struct));
                for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
                    tablaEnLista_struct *tablaIterante = malloc(sizeof(tablaEnLista_struct));
                    tablaIterante = list_get(listaDeTablasDePaginas,i);
                    if (tablaIterante->idPatota == primerSegmento->idPatota){
                        listaSegmentosLocal=tablaIterante->tablaDePaginas;
                    }
                }
                segmentoEnTabla_struct *primerSegmentoLocal = list_get(listaSegmentosLocal,0);
                primerSegmentoLocal->inicio=memoria;
                list_replace(listaSegmentosLocal,0,primerSegmentoLocal);

            }
        }else{
            segmentoEnTablaGlobal_struct *segmentoActual = list_get(listaGlobalDeSegmentos,i);
            segmentoEnTablaGlobal_struct *segmentoAnterior = list_get(listaGlobalDeSegmentos,i-1);
            if ((int)segmentoActual->inicio != ((int)segmentoAnterior->inicio + segmentoAnterior->tamanio)){
                memcpy((int)segmentoAnterior->inicio+segmentoAnterior->tamanio,segmentoActual->inicio,segmentoActual->tamanio);
                segmentoActual->inicio = (int)segmentoAnterior->inicio+segmentoAnterior->tamanio;
                list_replace(listaGlobalDeSegmentos,i,segmentoActual);


                t_list *listaSegmentosLocalActual = malloc(sizeof(tablaEnLista_struct));
                for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
                    tablaEnLista_struct *tablaIterante = malloc(sizeof(tablaEnLista_struct));
                    tablaIterante = list_get(listaDeTablasDePaginas,i);
                    if (tablaIterante->idPatota == segmentoActual->idPatota){
                        listaSegmentosLocalActual=tablaIterante->tablaDePaginas;
                    }
                }

                t_list *listaSegmentosLocalAnterior = malloc(sizeof(tablaEnLista_struct));
                for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
                    tablaEnLista_struct *tablaIterante = malloc(sizeof(tablaEnLista_struct));
                    tablaIterante = list_get(listaDeTablasDePaginas,i);
                    if (tablaIterante->idPatota == segmentoAnterior->idPatota){
                        listaSegmentosLocalAnterior=tablaIterante->tablaDePaginas;
                    }
                }

                segmentoEnTabla_struct *segmentoLocalActual = list_get(listaSegmentosLocalActual,segmentoActual->segmentoEnLocal);
                segmentoLocalActual->inicio=segmentoActual->inicio;
                list_replace(listaSegmentosLocalActual,segmentoActual->segmentoEnLocal,segmentoLocalActual);


            }
        }
    }
}

void actualizar_estado_paginacion(uint32_t idElemento, uint32_t idPatota, char nuevoEstado){
	tipoUniversal = 'T';
	t_list* listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
	elementoEnLista_struct *elementoAReemplazar = malloc(sizeof(elementoEnLista_struct));
	for(int i=0;i<list_size(listaFiltrada);i++){
		elementoAReemplazar = list_get(listaFiltrada,i);
		if(elementoAReemplazar->ID == idElemento){
			break;
		}
	}

	tablaEnLista_struct *tablaBuscada = malloc(sizeof(tablaEnLista_struct));
	t_list *tablaDePaginas;
	for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
		tablaBuscada = list_get(listaDeTablasDePaginas,i);
	    if (tablaBuscada->idPatota == idPatota){
	    	tablaDePaginas = tablaBuscada->tablaDePaginas;
	        break;
	    }
	}

	paginaEnTabla_struct *paginaInicial=list_get(tablaDePaginas,elementoAReemplazar->segmentoOPagina);
	int primeraPagina = elementoAReemplazar->segmentoOPagina;


	tcb* tcbAModificar = malloc(sizeof(tcb));

	tcbAModificar = buscar_en_memoria_paginacion(idElemento, idPatota, 'T');
	tcbAModificar->estado = nuevoEstado;
	void* payload = tcbAModificar;
	int frameInicial = paginaInicial->frame;
	int* direccionFisica;
	int payloadYaGuardado=0;
	int tamPayload = 21;
	direccionFisica = (int)memoria + (frameInicial * tamPagina + elementoAReemplazar->offsetEnPagina);
	int menorEntre2 = menorEntreDos(tamPayload,(tamPagina-elementoAReemplazar->offsetEnPagina));
	memcpy(direccionFisica,payload,menorEntre2);
	payloadYaGuardado += menorEntreDos(tamPayload,tamPagina-elementoAReemplazar->offsetEnPagina);
	payload = (int)payload + menorEntreDos(tamPayload,tamPagina-elementoAReemplazar->offsetEnPagina);
	while (payloadYaGuardado<tamPayload) {
		primeraPagina++;
	    paginaInicial=list_get(tablaDePaginas,primeraPagina);
	    frameInicial = paginaInicial->frame;
	    if (paginaInicial->presencia == 0){
	    	direccionFisica = (int)memoriaSwap + (frameInicial * tamPagina);
	    }else{
	    	direccionFisica = (int)memoria + (frameInicial * tamPagina);
	    }
	    menorEntre2 = menorEntreDos(tamPagina,tamPayload-payloadYaGuardado);
	    memcpy(direccionFisica,payload, menorEntre2);
	    payloadYaGuardado = (int)payloadYaGuardado + menorEntreDos(tamPayload-payloadYaGuardado,tamPagina);
	    payload = (int)payload + menorEntreDos(tamPayload-payloadYaGuardado,tamPagina);
	}
}





void actualizar_posicion_paginacion(uint32_t idElemento, uint32_t idPatota, uint32_t nuevaPosX,uint32_t nuevaPosY){
	tipoUniversal = 'T';
	t_list* listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
	elementoEnLista_struct *elementoAReemplazar = malloc(sizeof(elementoEnLista_struct));
	for(int i=0;i<list_size(listaFiltrada);i++){
		elementoAReemplazar = list_get(listaFiltrada,i);
		if(elementoAReemplazar->ID == idElemento){
			break;
		}
	}

	tablaEnLista_struct *tablaBuscada = malloc(sizeof(tablaEnLista_struct));
	t_list *tablaDePaginas;
	for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
		tablaBuscada = list_get(listaDeTablasDePaginas,i);
	    if (tablaBuscada->idPatota == idPatota){
	    	tablaDePaginas = tablaBuscada->tablaDePaginas;
	        break;
	    }
	}

	paginaEnTabla_struct *paginaInicial=list_get(tablaDePaginas,elementoAReemplazar->segmentoOPagina);
	int primeraPagina = elementoAReemplazar->segmentoOPagina;


	tcb* tcbAModificar = malloc(sizeof(tcb));
	tcbAModificar = buscar_en_memoria_paginacion(idElemento, idPatota, 'T');
	int frameInicial = paginaInicial->frame;
	tcbAModificar->posX = nuevaPosX;
	tcbAModificar->posY = nuevaPosY;
	void* payload = tcbAModificar;

	int* direccionFisica;
	int payloadYaGuardado=0;
	int tamPayload = 21;
	direccionFisica = (int)memoria + (frameInicial * tamPagina + elementoAReemplazar->offsetEnPagina);
	        int menorEntre2 = menorEntreDos(tamPayload,(tamPagina-elementoAReemplazar->offsetEnPagina));
	        memcpy(direccionFisica,payload,menorEntre2);
	        payloadYaGuardado += menorEntreDos(tamPayload,tamPagina-elementoAReemplazar->offsetEnPagina);
	        payload = (int)payload + menorEntreDos(tamPayload,tamPagina-elementoAReemplazar->offsetEnPagina);
	        while (payloadYaGuardado<tamPayload) {
	        	primeraPagina++;
	        	paginaInicial=list_get(tablaDePaginas,primeraPagina);
	        	frameInicial = paginaInicial->frame;
	        	direccionFisica = (int)memoria + (frameInicial * tamPagina);
	        	menorEntre2 = menorEntreDos(tamPagina,tamPayload-payloadYaGuardado);
	        	memcpy(direccionFisica,payload, menorEntre2);
	        	paginaEnTabla_struct *nuevaPagina = malloc(sizeof (paginaEnTabla_struct));
	        	payloadYaGuardado = (int)payloadYaGuardado + menorEntreDos(tamPayload-payloadYaGuardado,tamPagina);
	        	payload = (int)payload + menorEntreDos(tamPayload-payloadYaGuardado,tamPagina);
	        }
}






void actualizar_estado_segmentacion(uint32_t idElemento, uint32_t idPatota, char nuevoEstado){
	tipoUniversal = 'T';
	t_list* listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
	elementoEnLista_struct *elementoAReemplazar = malloc(sizeof(elementoEnLista_struct));
	for(int i=0;i<list_size(listaFiltrada);i++){
		elementoAReemplazar = list_get(listaFiltrada,i);
		if(elementoAReemplazar->ID == idElemento){
			break;
		}
	}

	tablaEnLista_struct *tablaBuscada = malloc(sizeof(tablaEnLista_struct));
	t_list *tablaDePaginas;
	for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
		tablaBuscada = list_get(listaDeTablasDePaginas,i);
	    if (tablaBuscada->idPatota == idPatota){
	    	tablaDePaginas = tablaBuscada->tablaDePaginas;
	        break;
	    }
	}

	segmentoEnTabla_struct *paginaInicial=list_get(tablaDePaginas,elementoAReemplazar->segmentoOPagina);
	int inicioSegmento = paginaInicial->inicio;
	tcb* tcbAModificar = malloc(sizeof(tcb));
	tcbAModificar = buscar_de_memoria_segmentacion(idElemento, idPatota, 'T');
	tcbAModificar->estado = nuevoEstado;
	void* payload = tcbAModificar;
	int* direccionFisica;
	int payloadYaGuardado=0;
	int tamPayload = 21;

	memcpy(inicioSegmento,tcbAModificar,paginaInicial->tamanio);
}




void actualizar_posicion_segmentacion(uint32_t idElemento, uint32_t idPatota, uint32_t nuevaPosX,uint32_t nuevaPosY){
	tipoUniversal = 'T';
	t_list* listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
	elementoEnLista_struct *elementoAReemplazar = malloc(sizeof(elementoEnLista_struct));
	for(int i=0;i<list_size(listaFiltrada);i++){
		elementoAReemplazar = list_get(listaFiltrada,i);
		if(elementoAReemplazar->ID == idElemento){
			break;
		}
	}

	tablaEnLista_struct *tablaBuscada = malloc(sizeof(tablaEnLista_struct));
	t_list *tablaDePaginas;
	for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
		tablaBuscada = list_get(listaDeTablasDePaginas,i);
	    if (tablaBuscada->idPatota == idPatota){
	    	tablaDePaginas = tablaBuscada->tablaDePaginas;
	        break;
	    }
	}

	segmentoEnTabla_struct *paginaInicial=list_get(tablaDePaginas,elementoAReemplazar->segmentoOPagina);
	int inicioSegmento = paginaInicial->inicio;

	tcb* tcbAModificar = malloc(sizeof(tcb));
	tcbAModificar = buscar_de_memoria_segmentacion(idElemento, idPatota, 'T');
	tcbAModificar->posX = nuevaPosX;
	tcbAModificar->posY = nuevaPosY;
	void* payload = tcbAModificar;

	int* direccionFisica;
	int payloadYaGuardado=0;
	int tamPayload = 21;

	memcpy(inicioSegmento,tcbAModificar,paginaInicial->tamanio);
}

void actualizar_indice_segmentacion(uint32_t idElemento, uint32_t idPatota){
    tipoUniversal = 'T';
    t_list* listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
    elementoEnLista_struct *elementoAReemplazar = malloc(sizeof(elementoEnLista_struct));
    for(int i=0;i<list_size(listaFiltrada);i++){
        elementoAReemplazar = list_get(listaFiltrada,i);
        if(elementoAReemplazar->ID == idElemento){
            break;
        }
    }

    tablaEnLista_struct *tablaBuscada = malloc(sizeof(tablaEnLista_struct));
    t_list *tablaDePaginas;
    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaBuscada = list_get(listaDeTablasDePaginas,i);
        if (tablaBuscada->idPatota == idPatota){
            tablaDePaginas = tablaBuscada->tablaDePaginas;
            break;
        }
    }

    segmentoEnTabla_struct* paginaInicial=list_get(tablaDePaginas,elementoAReemplazar->segmentoOPagina);
    int inicioSegmento = paginaInicial->inicio;

    tcb* tcbAModificar = malloc(sizeof(tcb));
    tcbAModificar = buscar_de_memoria_segmentacion(idElemento, idPatota, 'T');
    tcbAModificar->proxTarea++;
    void* payload = tcbAModificar;

    int* direccionFisica;
    int payloadYaGuardado=0;
    int tamPayload = 21;

    memcpy(inicioSegmento,tcbAModificar,paginaInicial->tamanio);
}

void actualizar_indice_paginacion(uint32_t idElemento, uint32_t idPatota){
    tipoUniversal = 'T';
    t_list* listaFiltrada = list_filter(listaElementos,filtrarPorTipo);
    elementoEnLista_struct *elementoAReemplazar = malloc(sizeof(elementoEnLista_struct));
    for(int i=0;i<list_size(listaFiltrada);i++){
        elementoAReemplazar = list_get(listaFiltrada,i);
        if(elementoAReemplazar->ID == idElemento){
            break;
        }
    }

    tablaEnLista_struct *tablaBuscada = malloc(sizeof(tablaEnLista_struct));
    t_list *tablaDePaginas;
    for (int i = 0; i < list_size(listaDeTablasDePaginas); ++i) {
        tablaBuscada = list_get(listaDeTablasDePaginas,i);
        if (tablaBuscada->idPatota == idPatota){
            tablaDePaginas = tablaBuscada->tablaDePaginas;
            break;
        }
    }

    paginaEnTabla_struct* paginaInicial=list_get(tablaDePaginas,elementoAReemplazar->segmentoOPagina);
    int primeraPagina = elementoAReemplazar->segmentoOPagina;


    tcb* tcbAModificar = malloc(sizeof(tcb));
    tcbAModificar = buscar_en_memoria_paginacion(idElemento, idPatota, 'T');
    int frameInicial = paginaInicial->frame;
    tcbAModificar->proxTarea++;
    void* payload = tcbAModificar;

    int* direccionFisica;
    int payloadYaGuardado=0;
    int tamPayload = 21;
    direccionFisica = (int)memoria + (frameInicial * tamPagina + elementoAReemplazar->offsetEnPagina);
    int menorEntre2 = menorEntreDos(tamPayload,(tamPagina-elementoAReemplazar->offsetEnPagina));
    memcpy(direccionFisica,payload,menorEntre2);
    payloadYaGuardado += menorEntreDos(tamPayload,tamPagina-elementoAReemplazar->offsetEnPagina);
    payload = (int)payload + menorEntreDos(tamPayload,tamPagina-elementoAReemplazar->offsetEnPagina);
    while (payloadYaGuardado<tamPayload) {
        primeraPagina++;
        paginaInicial=list_get(tablaDePaginas,primeraPagina);
        frameInicial = paginaInicial->frame;
        if (paginaInicial->presencia == 0){
            direccionFisica = (int)memoriaSwap + (frameInicial * tamPagina);
        }else{
            direccionFisica = (int)memoria + (frameInicial * tamPagina);
        }
        menorEntre2 = menorEntreDos(tamPagina,tamPayload-payloadYaGuardado);
        memcpy(direccionFisica,payload, menorEntre2);
        payloadYaGuardado = (int)payloadYaGuardado + menorEntreDos(tamPayload-payloadYaGuardado,tamPagina);
        payload = (int)payload + menorEntreDos(tamPayload-payloadYaGuardado,tamPagina);
    }
}

void actualizar_lista_elementos_segmentacion(int segmentoAgregado,int PID){
	elementoEnLista_struct* elementoIterante = malloc(sizeof(elementoEnLista_struct));
	for (int i=0;i<list_size(listaElementos);i++){
		elementoIterante = list_get(listaElementos,i);
		if ((elementoIterante->segmentoOPagina >= segmentoAgregado) && (elementoIterante->PID == PID)){
			log_info(logger,"Entre al maldito if");
			elementoIterante->segmentoOPagina++;
			list_replace(listaElementos,i,elementoIterante);
		}
	}
}
