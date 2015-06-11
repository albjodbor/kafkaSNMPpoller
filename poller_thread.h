#ifndef DATOS_H
#include "datos.h"
#endif

#include "poller.h"

#define ESPERA_REINTENTOS 4;

/*
 * Funcion: void * poller (void * thread_args)
 *
 * 		Funcion ejecutada por hilos. Se encarga del envio de pdu continuo.
 * 		Recibira una lista enlazada de los hosts que tiene que monitorizar
 */
void * poller (void * thread_args);
/*
 * Funcion: int procesa_pdu(int operation, struct snmp_session *sp,
 * 	int reqid, struct snmp_pdu *pdu, void *magic)
 *
 * 		Funcion que se definirá de callback en las sesiones snmp.
 * 		Se llamara cada vez que se recibe una trama pdu.
 * 		SU funcion principal sera enviar por kafka los
 * 		datos de la PDU recibida
 *
 * 		Parametros callback:
 * 		1)int operation				--> the possible operations are RECEIVED_MESSAGE and TIMED_OUT
 * 		2)struct snmp_session* session	--> the session that was authenticated using community
 * 		3)int reqid					--> the request ID identifying the transaction within this session. Use 0 for traps
 * 		4)struct snmp_pdu* pdu			--> a pointer to PDU information. You must copy the information because it will be freed elsewhere
 * 		5)void* magic					--> a pointer to the data for callback()
 */
int procesa_pdu(int operation, struct snmp_session *sp, int reqid, struct snmp_pdu *pdu, void *magic);
/*
 * Funcion: void * hilo_lectura (void * thread_args)
 *
 * 		Funcion ejecutada en un hilo. Se encargará de recibir las
 * 		respuestas a las pdu enviadas por los poller a traves de la
 * 		funcion send
 */
void * hilo_lectura (void * thread_args);

/*
 * Funcion: nt print_result (int status, struct snmp_session *sp, struct snmp_pdu *pdu)
 *
 * 		Funcion provisional que imprime la PDU recibida por pantalla
 */

int print_result (int status, struct snmp_session *sp, struct snmp_pdu *pdu);
