/*
 * Programa: Poller SNMP
 * Autor: Alberto Jodar
 * Fecha: 29-06-2015
 * Descripción:
 * Trabajo fin de grado.
 * TODO
 * 1) Comprobar que se haya abierto kafka
 * 2) Recibo configuracion JSON
 * 3) Envio por kafka -- pruebas
 * 4) Mas versiones SNMP
 */

#include "poller.h"

#include "estructura.h"
#include "poller_thread.h"
#include "logger.h"
#include "productor_kafka.h"


//----------------FUNCIONES----------------
void handlerCierre (int dummy);

//----------------GLOBALES----------------

int main () {

//------------------FLAGS------------------
	int error_estructura=0;



//----------------VARIABLES----------------
	//TODO relleno desde ficheros
	int max_hilos = 10;
	int num_host = 0;
	int resto;
	int host_hilo;
	int hilos_lanzados;
	int fin_lanzar_hilos;
	int fin_hilos_lanzados;
	int kafkaoff = 1;
	int brokeroff = 1;

	st_host * lista_host;
	st_host * lista_host_prov;

	pthread_t lectura;
	pthread_t * p_lectura = &lectura;
	pthread_t kafka_poll;
	pthread_t * p_kafka_poll = &kafka_poll;

	//Almacenara provisionalmente el puntero al hilo
	pthread_t p_hilo_prov;

	//Configuración KAFKA
	rd_kafka_conf_t *conf;
	rd_kafka_topic_conf_t *topic_conf;
	//TODO --> Lectura de broker y topic desde fichero/linea comandos
	char *brokers;
	char *topic;
	char errstr[512];

/*
 * TODO
 * Parametros:
 * 1- Fichero con la configuracion
 * 2- Fichero lista de hosts
 */
	LOG_PRINT("Iniciando el programa....");
	fin=1;
	fin_lectura=1;
	sesiones_activas = 0;

	//Inicializamos el descriptor fdset vacio. fdset variable global
	FD_ZERO(&fdset);
	//--------------CONFIGURACION--------------

	//-----------------------------KAFKA-----------------------------
	/*
	 * PRIMERO: configuración de kafka
	 * Si no funciona reintentamos periódicamente sin lanzar el programa
	 */
	brokers = "localhost:9092";
	topic = "snmp";

	conf = rd_kafka_conf_new();
	topic_conf = rd_kafka_topic_conf_new();
	rd_kafka_conf_set_dr_cb(conf, msg_delivered);
	/* Create Kafka handle */
	rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
	if (!(rk)) {
		LOG_PRINT("Intento fallido para kafka handle");
	}
	else {
		LOG_PRINT("Intento con exito para kafka handle");
		kafkaoff=0;
	}

	/* Set logger */
	rd_kafka_set_logger(rk, logger_kafka);
	rd_kafka_set_log_level(rk, LOG_DEBUG);

	/* Add brokers */
	if (rd_kafka_brokers_add(rk, brokers) == 0) {
		LOG_PRINT("Intento fallido para kafka broker");
	}
	else {
		LOG_PRINT("Intento con exito para kafka broker");
		brokeroff=0;
	}
	/* Create topic */
	rkt = rd_kafka_topic_new(rk, topic, topic_conf);

	//Antes de empezar, debemos lanzar un hilo que hago poll
	//TODO: pthread_create(p_kafka_poll,NULL,poll_kafka,NULL);

	//-----------------------------KAFKA-----------------------------

	lista_host = procesa_fichero_host();

	netsnmp_init_mib();
	add_mibdir("/usr/local/share/snmp/mibs");
	/*
	 * Una vez tenemos la lista creada,
	 * vamos a procesar los oid
	 */
	procesa_oid(lista_host);


	//Establezco t_monitor
	//TODO desde fichero
	t_monitor = 2;

	if (lista_host == NULL) {
		error_estructura=1;
		LOG_PRINT("Error en creacion de estructura dinamica");
	}
	else {
		LOG_PRINT("Creacion de estructura dinamica correcta");
	}
	//Registramos el handler
	signal(SIGINT, handlerCierre);



//------------------HILOS------------------

	/*
	 * Llenado de hilos:
	 * 1) De la configuracion tenemos el numero de hilos que podemos lanzar
	 * 2) Calculamos el numero de host a monitorizar
	 */

	lista_host_prov = lista_host;
	while (lista_host_prov!=NULL){
		num_host++;
		lista_host_prov = lista_host_prov->next;
	}
	LOG_PRINT("Vamos a monitorizar %d hosts",num_host);


	/*
	 * CALCULAR CUANTOS HOSTS VAN EN CADA HILO:
	 *
	 * Num_host % Num_hilos -> Nos da el resto
	 * Num_host / Num_hilos -> Hosts que van en cada hilo
	 * Tenemos que ver que hacer con el resto
	 * --> Añadimos uno adicional en cada hilo
	 */

	resto = num_host%max_hilos;
	host_hilo = num_host/max_hilos;

	//Establecemos las variables corte
	lista_cortes (lista_host,host_hilo,resto,max_hilos);

	//PROVISIONAL
	imprime_lista(lista_host);
	imprime_cortes(lista_host);



	//-----------------HILOS POLLER-----------------
	/*
	 * Tenemos la lista marcada en las variables corte
	 * A cada hilo le pasaremos el puntero a un nodo, y
	 * monitorizara hasta que encuentre corte = 1 o NULL
	 */
	lista_host_prov = lista_host;
	hilos_lanzados = 0;
	fin_hilos_lanzados = 1;

	while ((hilos_lanzados < max_hilos)&&(fin_hilos_lanzados)) {
		pthread_create(&p_hilo_prov,NULL,poller,(void *)lista_host_prov);
		hilos_lanzados++;
		/*
		 * Avanzamos hasta el siguiente corte, copiando el puntero
		 * al hilo en cada nodo. Hasta que llegamos al corte o a NULL
		 */
		do {
			//En cada paso, almaceno el puntero al hilo
			lista_host_prov->thread = p_hilo_prov;
			//Cuando llegamos al corte o a NULL, paramos el bucle
			if (lista_host_prov->corte == 1)
				fin_lanzar_hilos = 0;
			else if (lista_host_prov->next ==NULL) {
				fin_lanzar_hilos = 0;
				fin_hilos_lanzados = 0;
			}
			/*
			 * 	Cuando llegamos al corte, avanzamos un host mas
			 * 	y se lo pasamos a pthread_create en la siguiente interaccion
			 */
			lista_host_prov = lista_host_prov->next;
		} while (fin_lanzar_hilos);


	}


	//-----------------HILO LECTURA-----------------
	//Lanzamos un hilo para la lectura
	pthread_create(p_lectura,NULL,hilo_lectura,NULL);


	/*
	 * Tenemos que esperar la finalizacion de cada hilo
	 * usando solo el puntero al hilo del nodo que sirve como corte,
	 * si no, estamos repitiendo
	 */
	lista_host_prov = lista_host;
	while (lista_host_prov->next != NULL) {
		if (lista_host_prov->corte == 1) {
			pthread_join(lista_host_prov->thread,NULL);
		}
		lista_host_prov = lista_host_prov->next;
	}
	//Sale cuando estamos en el ultimo nodo
	pthread_join(lista_host_prov->thread,NULL);
	pthread_join(lectura,NULL);
	//Este es el ultimo hilo que debemos esperar
	//TODO: pthread_join(kafka_poll,NULL);
	//Finalizamos el productor KAFKA
	/* Destroy topic */
	rd_kafka_topic_destroy(rkt);
	/* Destroy the handle */
	rd_kafka_destroy(rk);
	/* Let background threads clean up and terminate cleanly. */
	rd_kafka_wait_destroyed(2000);

	libera_host(lista_host);

return 0;

}

/*
 * Funcion: void handlerCierre (int dummy)
 * Funcion que detectara la señal de cierre.
 * Este handler, ante la recepcion de Ctrl-C (SIGINT)
 * cerrara todas las sesiones y finalizara el programa
 */
void handlerCierre (int dummy) {
	fin = 0;
}

