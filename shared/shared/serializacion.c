/*
 * serializacion.c
 *
 *  Created on: 6 may. 2021
 *      Author: utnso
 */
#include "conexion.h"
#include "TAD_PATOTA.h"
#include "TAD_TRIPULANTE.h"

//==============================INICIAR_PATOTA========================================

//void serializar_iniciar_patota( iniciar_patota* tareaPatota, int socket)
//{
//	// Lo que deberia enviar es el id de la patota, el FILE con las tareas y la cant de tripulantes
//	// Para eso ya creamos el TAD Inicio_patota
//	// POR FAVOR CAMBIALE EL NOMBRE POR ALGO MAS CLARO, HAY 3 QUE SON CASI IGUALES
//	// POR FAVOR CAMBIALE EL NOMBRE POR ALGO MAS CLARO, HAY 3 QUE SON CASI IGUALES
//	// POR FAVOR CAMBIALE EL NOMBRE POR ALGO MAS CLARO, HAY 3 QUE SON CASI IGUALES
//	// POR FAVOR CAMBIALE EL NOMBRE POR ALGO MAS CLARO, HAY 3 QUE SON CASI IGUALES
//	t_buffer* buffer = malloc(sizeof(t_buffer));
//
//		buffer->size = sizeof(uint8_t)  // Id
//					 + sizeof(uint32_t)//longitud de tarea
//		             + sizeof(FILE); // Tarea
//		             // La longitud del string nombre. Le sumamos 1 para enviar tambien el caracter centinela '\0'. Esto se podría obviar, pero entonces deberíamos agregar el centinela en el receptor.
//
//		void* stream = malloc(buffer->size);
//		int offset = 0; // Desplazamiento
//		memcpy(stream+offset,&(tareaPatota->idPatota),sizeof(uint8_t));
//		offset+=sizeof(uint8_t);
//		memcpy(stream+offset,&tareaPatota->cantTripulantes,sizeof(uint8_t));
//		offset+=sizeof(uint32_t);
//		memcpy(stream+offset,tareaPatota->Tareas,sizeof(FILE));
//
//		buffer->stream=stream;
//		t_paquete* paquete = malloc(sizeof(t_paquete));
//
//
//			//CODIGO DE OPERACION 1 = UN TRIUPLANTE
//			paquete->codigo_operacion = INICIAR_PATOTA; // Podemos usar una constante por operación
//			paquete->buffer = buffer; // Nuestro buffer de antes.
//
//			// Armamos el stream a enviar
//			void* a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
//			offset = 0;
//
//			memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
//
//			send(socket, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);
//
//			// No nos olvidamos de liberar la memoria que ya no usaremos
//			free(a_enviar);
//			free(paquete->buffer->stream);
//			free(paquete->buffer);
//			free(paquete);
//
//
//}
//iniciar_patota* deserializar_iniciar_patota(t_buffer* buffer)
//{
//	iniciar_patota* tareas=malloc(sizeof(tareasPatota));
//
//	void* stream=buffer->stream;
//
//	memcpy(&(tareas->idPatota),stream,sizeof(uint8_t));
//	stream+=sizeof(uint8_t);
//	memcpy(&(tareas->cantTripulantes),stream,sizeof(uint8_t));
//	stream+=sizeof(uint8_t);
//	memcpy(&(tareas->Tareas),stream,sizeof(FILE*));
//
//	return tareas;
//
//
//}
void agregar_paquete_iniciar_patota(t_paquete* paquete, t_iniciar_patota* estructura){
	int offset = 0;
	paquete->buffer->size += sizeof(uint32_t) + sizeof(uint8_t)*2 + estructura->tamanio_tareas;

	paquete->buffer->stream = realloc(paquete->buffer->stream,paquete->buffer->size);

	memcpy(paquete->buffer->stream, &(estructura->idPatota), sizeof(uint8_t));		//El entero para el idpatota
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + offset, &(estructura->cantTripulantes), sizeof(uint8_t));		//El entero de la cantidad de tcb
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + offset, &(estructura->tamanio_tareas), sizeof(uint32_t));		//El entero para el tamanio de tareas
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + offset, estructura->Tareas, estructura->tamanio_tareas);	//el archivo de tareas


}

t_iniciar_patota* deserializar_iniciar_patota(t_paquete* paquete){

	t_iniciar_patota* estructura = malloc(sizeof(t_iniciar_patota));
	int offset = 0;

	memcpy(&(estructura->idPatota), paquete->buffer->stream, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&(estructura->cantTripulantes), paquete->buffer->stream + offset , sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&(estructura->tamanio_tareas), paquete->buffer->stream + offset , sizeof(uint32_t));
	offset += sizeof(uint32_t);

	estructura->Tareas = (char*) malloc( (int) estructura->tamanio_tareas); //Es necesario  el casteo a (int)??
	memcpy(estructura->Tareas, paquete->buffer->stream + offset , estructura->tamanio_tareas);

	eliminar_paquete(paquete);
	return estructura;

}

void imprimir_paquete_iniciar_patota(t_iniciar_patota* estructura){
	printf("ID PATOTA: %d\n",estructura->idPatota);
	printf("CANTIDAD DE TRIPULANTES: %d\n",estructura->cantTripulantes);
	printf("TAMAÑO DE TAREAS: %d\n",estructura->tamanio_tareas);
	printf("TAREAS: %s\n",estructura->Tareas);
	puts("");
}

void liberar_t_iniciar_patota(t_iniciar_patota* estructura){
	free(estructura->Tareas);
	free(estructura);
}

//==============================TRIPUALNTE========================================

void agregar_paquete_tripulante(t_paquete* paquete, t_tripulante* estructura){
	int offset = 0;

	paquete->buffer->size += sizeof(uint8_t)*4;
	paquete->buffer->stream = realloc(paquete->buffer->stream,paquete->buffer->size);

	memcpy(paquete->buffer->stream, &(estructura->id_tripulante), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + offset, &(estructura->id_patota), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + offset, &(estructura->posicion_x), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + offset, &(estructura->posicion_y), sizeof(uint8_t));
}

t_tripulante* deserializar_tripulante(t_paquete* paquete){
	t_tripulante* estructura = malloc(sizeof(Tripulante));
	int offset = 0;

    memcpy(&(estructura->id_tripulante), paquete->buffer->stream, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&(estructura->id_patota), paquete->buffer->stream + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&(estructura->posicion_x), paquete->buffer->stream + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&(estructura->posicion_y), paquete->buffer->stream + offset, sizeof(uint8_t));

	eliminar_paquete(paquete);
	return estructura;


}

void imprimir_paquete_tripulante(t_tripulante* estructura){
	printf("ID TRIPULANTE: %d\n",estructura->id_tripulante);
	printf("ID PATOTA: %d\n",estructura->id_patota);
	printf("POSICION EN X: %d\n",estructura->posicion_x);
	printf("POSICION EN Y: %d\n",estructura->posicion_y);
	puts("");
}

void liberar_t_tripulante(t_tripulante* estructura){
	free(estructura);
}
//==============================ELIMINAR_TRIPUALNTE========================================

//============================== CAMBIO_ESTADO ========================================

void agregar_paquete_cambio_estado(t_paquete* paquete, t_cambio_estado* estructura){
	int offset = 0;

	paquete->buffer->size += sizeof(uint8_t) * 2 + sizeof(char);
	paquete->buffer->stream = realloc(paquete->buffer->stream,paquete->buffer->size);

	memcpy(paquete->buffer->stream, &(estructura->id_tripulante), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + offset, &(estructura->id_patota), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream+offset, &(estructura->estado), sizeof(char));

}

t_cambio_estado* deserializar_cambio_estado(t_paquete* paquete){
	t_cambio_estado* estructura = malloc(sizeof(t_cambio_estado));
	int offset = 0;

    memcpy(&(estructura->id_tripulante), paquete->buffer->stream, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&(estructura->id_patota), paquete->buffer->stream+offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&(estructura->estado), paquete->buffer->stream + offset, sizeof(char));

	eliminar_paquete(paquete);
	return estructura;

}
void imprimir_paquete_cambio_estado(t_cambio_estado* estructura){
	printf("ID TRIPULANTE: %d\n",estructura->id_tripulante);
	printf("ID PATOTA: %d\n",estructura->id_patota);
	printf("ESTADO: %c\n",estructura->estado);
	puts("");
}
void liberar_t_cambio_estado(t_cambio_estado* estructura){
	free(estructura);
}

//============================== PEDIDO_MONGO ========================================
void agregar_paquete_pedido_mongo(t_paquete* paquete, t_pedido_mongo* estructura){
	int offset = 0;

	paquete->buffer->size += sizeof(uint8_t) * sizeof(uint32_t) + estructura->tamanio_mensaje;
	paquete->buffer->stream = realloc(paquete->buffer->stream,paquete->buffer->size);

	memcpy(paquete->buffer->stream, &(estructura->id_tripulante), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream + offset, &(estructura->tamanio_mensaje), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream+offset, estructura->mensaje, estructura->tamanio_mensaje);


}
t_pedido_mongo* deserializar_pedido_mongo(t_paquete* paquete){
	t_pedido_mongo* estructura = malloc(sizeof(t_pedido_mongo));
	int offset = 0;

	memcpy(&(estructura->id_tripulante), paquete->buffer->stream, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&(estructura->tamanio_mensaje), paquete->buffer->stream+offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(estructura->mensaje, paquete->buffer->stream + offset, estructura->tamanio_mensaje);

	eliminar_paquete(paquete);
	return estructura;

}
void imprimir_pedido_mongo(t_pedido_mongo* estructura){
	printf("ID TRIPULANTE: %d\n",estructura->id_tripulante);
	printf("TAMANIO MENSAJE: %d\n",estructura->tamanio_mensaje);
	printf("MENSAJE: %s\n",estructura->mensaje);
	puts("");
}

void liberar_t_pedido_mongo(t_pedido_mongo* estructura){
	free(estructura->mensaje);
	free(estructura);
}

//============================== MOVIMIENTO_MONGO ========================================
void agregar_paquete_movimiento_mongo(t_paquete* paquete, t_movimiento_mongo* estructura){
	int offset = 0;

	paquete->buffer->size += sizeof(uint8_t) * 5;
	paquete->buffer->stream = realloc(paquete->buffer->stream,paquete->buffer->size);

	memcpy(paquete->buffer->stream, &(estructura->id_tripulante), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream, &(estructura->origen_x), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream, &(estructura->origen_y), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream, &(estructura->destino_x), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream, &(estructura->destino_y), sizeof(uint8_t));

}

t_movimiento_mongo* deserializar_movimiento_mongo(t_paquete* paquete){
	t_movimiento_mongo* estructura = malloc(sizeof(t_movimiento_mongo));
	int offset = 0;

	memcpy(&(estructura->id_tripulante), paquete->buffer->stream, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&(estructura->origen_x), paquete->buffer->stream, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&(estructura->origen_y), paquete->buffer->stream, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&(estructura->destino_x), paquete->buffer->stream, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&(estructura->destino_y), paquete->buffer->stream, sizeof(uint8_t));

	eliminar_paquete(paquete);
	return estructura;


}
void imprimir_movimiento_mongo(t_movimiento_mongo* estructura){
	printf("ID TRIPULANTE: %d\n",estructura->id_tripulante);
	printf("ORIGEN X: %d\n",estructura->origen_x);
	printf("ORIGEN Y: %d\n",estructura->origen_y);
	printf("DESTINO X: %d\n",estructura->destino_x);
	printf("DESTINO Y: %d\n",estructura->destino_y);

}
void liberar_t_movimiento_mongo(t_movimiento_mongo* estructura){
	free(estructura);
}

//============================== CONSUMIR_MONGO ========================================

void agregar_paquete_consumir_recurso(t_paquete* paquete, t_consumir_recurso* estructura){
	int offset = 0;

	paquete->buffer->size += sizeof(uint8_t) + sizeof(char)*2;
	paquete->buffer->stream = realloc(paquete->buffer->stream,paquete->buffer->size);

	memcpy(paquete->buffer->stream, &(estructura->cantidad), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(paquete->buffer->stream, &(estructura->tipo), sizeof(char));
	offset += sizeof(char);
	memcpy(paquete->buffer->stream, &(estructura->consumible), sizeof(char));

}
t_consumir_recurso* deserializar_consumir_recurso(t_paquete* paquete){
	t_consumir_recurso* estructura = malloc(sizeof(t_movimiento_mongo));
	int offset = 0;

	memcpy(&(estructura->cantidad), paquete->buffer->stream, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(&(estructura->tipo), paquete->buffer->stream, sizeof(char));
	offset += sizeof(char);
	memcpy(&(estructura->consumible), paquete->buffer->stream, sizeof(char));

	eliminar_paquete(paquete);
	return estructura;
}
void imprimir_consumir_recurso(t_consumir_recurso* estructura){
	printf("CANTIDAD: %d\n",estructura->cantidad);
	printf("TIPO: %d\n",estructura->tipo);
	printf("RECURSO: %d\n",estructura->consumible);

}
void liberar_t_consumir_recurso(t_consumir_recurso* estructura){
	free(estructura);
}


//============================== 				  ========================================
void serializar_tarea_tripulante( Tripulante* tareaTrip, int socket)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint8_t)*2;  // Id

	void* stream = malloc(buffer->size);
	int offset = 0; // Desplazamiento
	memcpy(stream + offset, &tareaTrip->id, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(stream+offset,&tareaTrip->idPatota, sizeof(uint8_t));

	buffer->stream=stream;
	t_paquete* paquete = malloc(sizeof(t_paquete));


			//CODIGO DE OPERACION 1 = UN TRIUPLANTE
			paquete->codigo_operacion = PEDIR_TAREA; // Podemos usar una constante por operación
			paquete->buffer = buffer; // Nuestro buffer de antes.

			// Armamos el stream a enviar
			void* a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
			offset = 0;

			memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));

			send(socket, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

			// No nos olvidamos de liberar la memoria que ya no usaremos
			free(a_enviar);
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);



}

tareaTripulante* deserializar_tarea_tripulante(t_buffer* buffer)
{
	tareaTripulante* tarea=malloc(sizeof(tareaTripulante));

	void* stream= buffer->stream;
	memcpy(&(tarea->idTripulante), stream, sizeof(uint8_t));
	stream+=sizeof(uint8_t);
	memcpy(&(tarea->idPatota), stream, sizeof(uint8_t));
	return tarea;
}

//void serializar_tarea(char* tarea, int socket)
//{
//	t_buffer* buffer = malloc(sizeof(t_buffer));
//
//	buffer->size = sizeof(uint32_t)+strlen(tarea)+1;
//	uint32_t tamano= strlen(tarea)+1;
//
//	void* stream = malloc(buffer->size);
//	int offset = 0; // Desplazamiento
//	memcpy(stream+offset,&tamano,sizeof(uint32_t));
//	offset+=sizeof(uint32_t);
//	memcpy(stream+offset,tarea,strlen(tarea)+1);
//	buffer->stream=stream;
//	free(tarea);
//	t_paquete* paquete = malloc(sizeof(t_paquete));
//
//
//				//CODIGO DE OPERACION 1 = UN TRIUPLANTE
//	paquete->codigo_operacion = ENVIOTAREA; // Podemos usar una constante por operación
//	paquete->buffer = buffer; // Nuestro buffer de antes.
//
//				// Armamos el stream a enviar
//	void* a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
//	offset = 0;
//
//    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
//
//	send(socket, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);
//
//				// No nos olvidamos de liberar la memoria que ya no usaremos
//	free(a_enviar);
//	free(paquete->buffer->stream);
//	free(paquete->buffer);
//				free(paquete);
//
//}

char* deserializar_tarea(t_buffer* buffer)
{
	uint32_t* tamano=malloc(sizeof(uint32_t));
	void* stream= buffer->stream;

	memcpy(&(tamano),stream,sizeof(uint32_t));
	stream+=sizeof(uint32_t);
	char* tarea=malloc((int)tamano);
	memcpy(tarea,stream,(int)tamano);
	return tarea;

}

void serializar_id_and_pos(Tripulante* pos, int socket)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint8_t) * 3 ;// Id, posx, posy
	void* stream = malloc(buffer->size);
	int offset = 0; // Desplazamiento
	memcpy(stream+offset,&pos->id,sizeof(uint8_t));
	offset+=sizeof(uint8_t);
	memcpy(stream+offset,&pos->posicionX,sizeof(uint8_t));
	offset+=sizeof(uint8_t);
	memcpy(stream+offset,&pos->posicionY,sizeof(uint8_t));
	buffer->stream=stream;

	t_paquete* paquete = malloc(sizeof(t_paquete));


	//CODIGO DE OPERACION 1 = UN TRIUPLANTE
		paquete->codigo_operacion = ACTUALIZAR_POS; // Podemos usar una constante por operación
		paquete->buffer = buffer; // Nuestro buffer de antes.

				// Armamos el stream a enviar
	    void* a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
				offset = 0;

	   memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));

	   send(socket, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

				// No nos olvidamos de liberar la memoria que ya no usaremos
				free(a_enviar);
				free(paquete->buffer->stream);
				free(paquete->buffer);
				free(paquete);

}

id_and_pos* deserializar_id_and_pos(t_buffer* buffer)
{
	id_and_pos* id_and_pos = malloc(sizeof(id_and_pos));
	void*stream=buffer->stream;
	//Deserealizamos campo por campo mviendonos en el buffer

	memcpy(&(id_and_pos -> idTripulante),stream,sizeof(uint8_t));
	stream+=sizeof(uint8_t);
	memcpy(&(id_and_pos->posX),stream,sizeof(uint8_t));
	stream+=sizeof(uint8_t);
	memcpy(&(id_and_pos->posY),stream,sizeof(uint8_t));

	return id_and_pos;
}


void serializar_sabotaje(char* sabotaje, int socket)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint32_t)+strlen(sabotaje)+1;
	uint32_t tamano= strlen(sabotaje)+1;

	void* stream = malloc(buffer->size);
	int offset = 0; // Desplazamiento
	memcpy(stream+offset,&tamano,sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(stream+offset,sabotaje,strlen(sabotaje)+1);
	buffer->stream=stream;
	free(sabotaje);
	t_paquete* paquete = malloc(sizeof(t_paquete));


				//CODIGO DE OPERACION 1 = UN TRIUPLANTE
	paquete->codigo_operacion = SABOTAJE; // Podemos usar una constante por operación
	paquete->buffer = buffer; // Nuestro buffer de antes.

				// Armamos el stream a enviar
	void* a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));

	send(socket, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

				// No nos olvidamos de liberar la memoria que ya no usaremos
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
				free(paquete);

}

char* deserializar_sabotaje(t_buffer* buffer)
{
	uint32_t* tamano=malloc(sizeof(uint32_t));
	void* stream= buffer->stream;

	memcpy(&(tamano),stream,sizeof(uint32_t));
	stream+=sizeof(uint32_t);
	char* sabotaje=malloc((int)tamano);
	memcpy(sabotaje,stream,(int)tamano);
	return sabotaje;

}
