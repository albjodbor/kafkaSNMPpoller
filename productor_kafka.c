#include "productor_kafka.h"
#include "logger.h"



/**
 * Message delivery report callback.
 * Called once for each message.
 */
void msg_delivered (rd_kafka_t *rk, void *payload, size_t len, int error_code,
			   	   	   void *opaque, void *msg_opaque) {

	if (error_code)
		LOG_PRINT("Message delivery failed: %s",rd_kafka_err2str(error_code));
	else
		LOG_PRINT("Message delivered: %s",(char *)payload);
}

/**
* Kafka logger callback
* Usa nuestro poller interno
*/
void logger_kafka (const rd_kafka_t *rk, int level, const char *fac, const char *buf) {
	//struct timeval tv;
	//gettimeofday(&tv, NULL);

	LOG_PRINT("RDKAFKA: %s\n", buf);
}

/*
 * produce_msg
 * Función que, con la configuracion definida en el main
 * hara de productor, y enviara la cadena pasada como parametro
 */
void produce_msg (char * cadena) {
	/*
	* Argumentos:
	* Topic: rd_kafka_topic_t *rkt			--> GLOBAL
	* particion: int32_t partitition		--> RD_KAFKA_PARTITION_UA
	* 											O un numero de particiones (0..N)
	* ------------------------------------------------------------------------------------
	* ANOTACION--> If partition is RD_KAFKA_PARTITION_UA the configured partitioner will
	* be run for each message (slower), otherwise the messages will be enqueued
	* to the specified partition directly (faster).
	* ------------------------------------------------------------------------------------
	* flags: int msgflags					--> RD_KAFKA_MSG_F_FREE
	* ------------------------------------------------------------------------------------
	* ANOTACION--> RD_KAFKA_MSG_F_FREE - rdkafka will free(3) 'payload' when it is done
	* 				with it.
	* ------------------------------------------------------------------------------------
	* payload: void *payload				--> datos a enviar --> como cadena ¿?
	* longitud: size_t len					-->	longitud de los datos
	* key: const void *key					--> DE MOMENTO NO
	* keylen: size_t keylen				--> 	||
	* ------------------------------------------------------------------------------------
	* ANOTACION--> 'key' is an optional message key of size 'keylen' bytes, if non-NULL it
	* will be passed to the topic partitioner as well as be sent with the
	* message to the broker and passed on to the consumer.
	* ------------------------------------------------------------------------------------
	* msg_opaque: void *msg_opaque			--> DE MOMENTO NO
	* ------------------------------------------------------------------------------------
	* ANOTACION--> 'msg_opaque' is an optional application-provided per-message opaque
	* pointer that will provided in the delivery report callback (`dr_cb`) for
	* referencing this message.
	* ------------------------------------------------------------------------------------
	*/

	//OJO-->RD_KAFKA_PARTITION_UA y  RD_KAFKA_MSG_F_COPY
	if (rd_kafka_produce(rkt, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_FREE,
			   cadena, sizeof(cadena), NULL, 0, NULL) == -1) {
		LOG_PRINT("Failed to produce");
	}
   /*
	* TODO:
	* rd_kafka_poll(rk, 0);
	* Hacer un hilo que haga constantemente POLL
	* IMPORTANTE: Tenemos que liberar definitivo
	*/


}


