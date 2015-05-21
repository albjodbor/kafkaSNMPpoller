#ifndef DATOS_H
#include "datos.h"
#endif

#include "poller.h"

void msg_delivered (rd_kafka_t *rk, void *payload, size_t len, int error_code,
			   	   	   void *opaque, void *msg_opaque);
void logger_kafka (const rd_kafka_t *rk, int level, const char *fac, const char *buf);
/*
 * produce_msg
 * Función que, con la configuracion definida en el main
 * hara de productor, y enviara la cadena pasada como parametro
 */
void produce_msg (char * cadena);
/*
 * Funcion ejecutada por el hilo que ira haciendo poll
 */
void * poll_kafka (void * thread_args);
