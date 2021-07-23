#ifndef SRC_CONEXION_H_
#define SRC_CONEXION_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "TAD_PATOTA.h"
#include "TAD_TRIPULANTE.h"

/*
 * Creo el enum para no tener que pensar si devuelvo un "uno" o un "cero""
 */

#define ERROR 0
#define OK 1
//typedef enum {
//	ERROR,
//	OK
//} RESPUESTA;
// NO SE PORQUE ECLIPSE ME TOMA MAL LOS ENUM LPM



typedef struct {
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_buffer;

typedef struct {
    uint8_t codigo_operacion;
    t_buffer* buffer;
} t_paquete;
typedef enum {
	MIRAM,
	MONGOSTORE,
	DISCORDIADOR
}CONEXION_A;
/*========================== TIPOS DE MENSAJES =================================*/
typedef enum {
	INICIAR_PATOTA, //Es lo mismo que iniciar_paota?		// LISTO
	TRIPULANTE,												// LISTO
	PATOTA,			//Cuando se manda el tipo de mensaje patota ?
	PEDIR_TAREA,		//pedir y enviar tarea se podria hacer en un solo mensaje, una vez hecha la conexion se puede hacer
	ENVIOTAREA,		//send y recv las veces que quieran
	ACTUALIZAR_POS,											//LISTO
	ELIMINAR_TRIPULANTE,									//LISTO
	ACTUALIZAR_ESTADO,										//LISTO
	SABOTAJE
}CODE_OP;

typedef struct
{
	uint8_t idTripulante;
	uint8_t posX;
	uint8_t posY;
}id_and_pos;

typedef struct
{
	uint32_t idTripulante;
	uint32_t estado_length;
	char* estado;
}cambio_estado;



//================================
/*
 * Hay que determinara como se mandan los FILE*, o si debemos mandar un char*
 */
typedef struct
{
	uint8_t idPatota;
	uint8_t cantTripulantes;
	uint32_t tamanio_tareas;
	char* Tareas;
//	FILE* Tareas;
}t_iniciar_patota;

typedef struct
{
	uint8_t id_tripulante;
	uint8_t id_patota;
	uint8_t posicion_x;
	uint8_t posicion_y;

}t_tripulante;

typedef struct {

	uint8_t id_tripulante;
	uint8_t id_patota;
	uint32_t tamanio_estado;
	char* estado;

}t_cambio_estado;

/*
typedef struct {
	uint8_t id_tripulante;
	uint8_t id_
}t_solicitar_tarea;
					Capaz podemos usar t_tripulante
*/

typedef struct {
	uint8_t id_tripulante;
}t_eliminar_tripulante;
/*========================== TODOS =================================*/
void crear_buffer(t_paquete* paquete);
t_paquete* crear_paquete(CODE_OP codigo);
int enviar_paquete(t_paquete* paquete, int socket_cliente);
t_paquete* recibir_paquete(int socket_cliente, int* respuesta);

void liberar_conexion(int);
void eliminar_paquete(t_paquete* paquete);
/*========================== CLIENTE =================================*/
int crear_conexion(char*, char*);
int cliente(CONEXION_A modulo);


/*========================== SERVIDOR =================================*/
int crear_server(char*);
int esperar_cliente(int,int);

/*========================== MANEJO DE PAQUETES =================================*/
void* serializar_paquete(t_paquete* paquete, int bytes);

//---------------------------- INICIAR_PATOTA ----------------------------
/*
 * Hay que ver bien como se mandan los FILE*, o si mandamos un char*
 */

//void serializar_iniciar_patota(t_iniciar_patota* tareaPatota, int socket); //todo//todavia no lo borre, preguntar por las dudas
//iniciar_patota* deserializar_iniciar_patota(t_buffer* buffer);

void agregar_paquete_iniciar_patota(t_paquete* paquete, t_iniciar_patota* estructura);
t_iniciar_patota* deserializar_iniciar_patota(t_paquete* paquete);
void imprimir_paquete_iniciar_patota(t_iniciar_patota* estructura);
void liberar_iniciar_patota(t_iniciar_patota* estructura);



//--------------------------- TRIPULANTE -----------------------------
void serializar_tripulante(Tripulante* unTripulante, int socket);			//todo//todavia no lo borre, preguntar por las dudas
//Tripulante* deserializar_tripulante(t_buffer* buffer);

void agregar_paquete_tripulante(t_paquete* paquete, t_tripulante* estructura);
t_tripulante* deserializar_tripulante(t_paquete* paquete);
void imprimir_paquete_tripulante(t_tripulante* estructura);
void liberar_t_tripulante(t_tripulante* estructura);

//--------------- ELIMINAR_TRIPULANTE ----------------------------------------
void serializar_eliminar_tripulante(int idTripulante, int socket);			//todo//todavia no lo borre, preguntar por las dudas
//int deserializar_eliminar_tripulante(t_buffer* buffer);

void agregar_paquete_eliminar_tripulante(t_paquete* paquete, t_eliminar_tripulante* estructura);
t_eliminar_tripulante* deserializar_eliminar_tripulante(t_paquete* paquete);
void imprimir_paquete_eliminar_tripulante(t_eliminar_tripulante* estructura);
void liberar_t_eliminar_tripulante(t_eliminar_tripulante* estructura);

//--------------- MOVIMIENTO_TRIPULANTE ----------------------------------------
// Se puede utilizar el mismo mensaje T_TRIPULANTE, pero cambiando el codigo de operacion
void serializar_id_and_pos(Tripulante* pos, int socket);
id_and_pos* deserializar_id_and_pos(t_buffer* buffer);

//------------------- CAMBIO_ESTADO -------------------------------------
void serializar_cambio_estado(Tripulante* estado, int socket);				//todo//todavia no lo borre, preguntar por las dudas
//cambio_estado* deserializar_cambio_estado(t_buffer* buffer);

void agregar_paquete_cambio_estado(t_paquete* paquete, t_cambio_estado* estructura);
t_cambio_estado* deserializar_cambio_estado(t_paquete* paquete);
void imprimir_paquete_cambio_estado(t_cambio_estado* estructura);
void liberar_t_cambio_estado(t_cambio_estado* estructura);


//---------------------- SOLICITAR TAREA ----------------------------------
void serializar_tarea_tripulante( Tripulante* tareaTrip, int socket);
char* enviar_paquete_tarea(t_paquete* paquete,int socket);

/*
 * CAPAZ PODEMOS USAR UN T_TRIPULANTE O T_ELIMINAR_TRIPULANTE
 */

//--------------------------------------------------------
void serializar_tarea(char* tarea, int socket);
char* deserializar_tarea(t_buffer* buffer);


//--------------------------------------------------------
void serializar_patota( Patota* unaPatota, int socket);
Patota* deserializarPatota(t_buffer* buffer);

//--------------------------------------------------------
void serializar_sabotaje(char* sabotaje, int socket);
char* deserializar_sabotaje(t_buffer* buffer);




#endif /* SRC_CONEXION_H_ */
