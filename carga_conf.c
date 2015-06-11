#include "carga_conf.h"
#include "logger.h"
#include "conf.h"
#include "productor_kafka.h"
#include "poller_thread.h"


/*
 * Funcion: void lanzado_hilos()
 * 		Funcion que se encarga de lanzar los hilos
 * 		que hacen poller
 */
void lanzado_hilos() {

	int fin_hilos_lanzados;
	int fin_lanzar_hilos=1;

	st_host * lista_host_prov;

	//Almacenara provisionalmente el puntero al hilo
	pthread_t p_hilo_prov;

	lista_host_prov = lista_hosts;
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
}



/*
 * Funcion: config_estructura ()
 * 		1)Carga directorio mibs
 * 		2)Procesa los OID
 * 		3)Calculo de llenado de hilos
 * 		4)Establece cortes
 */
void config_estructura ()  {

	st_host * lista_host_prov;

	int num_host=0;

	netsnmp_init_mib();
	//TODO desde fichero configuracion
	add_mibdir("/usr/local/share/snmp/mibs");
	/*
	 * Una vez tenemos la lista creada,
	 * vamos a procesar los oid
	 */
	procesa_oid(lista_hosts);

	//------------------HILOS------------------

	/*
	 * Llenado de hilos:
	 * 1) De la configuracion tenemos el numero de hilos que podemos lanzar
	 * 2) Calculamos el numero de host a monitorizar
	 */
	lista_host_prov = lista_hosts;
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
	lista_cortes (lista_hosts,host_hilo,resto,max_hilos);
	LOG_PRINT("Tenemos disponibles %d hilos",max_hilos);
	LOG_PRINT("Vamos a lanzar %d hosts por hilo",host_hilo);
	LOG_PRINT("Con un resto a repartir de %d",resto);

}



/*
 * Funcion: int config_kafka ()
 * 		Funcion que cargara la configuracion
 * 		de kafka.
 * 		TODO: salir hasta configuracion correcta o fin
 */
int config_kafka () {
	//Valor devuelto
	int error=0;

	//Configuración KAFKA
	rd_kafka_conf_t *conf;
	rd_kafka_topic_conf_t *topic_conf;

	char errstr[512];

	//-----------------------------KAFKA-----------------------------
	if (fin != 0)
	{
		conf = rd_kafka_conf_new();
		topic_conf = rd_kafka_topic_conf_new();
		rd_kafka_conf_set_dr_cb(conf, msg_delivered);

		// Create Kafka handle
		rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
		if (!(rk)) {
			LOG_PRINT("Intento fallido para kafka handle");
			error=1;
		}
		else {
			LOG_PRINT("Intento con exito para kafka handle");
		}

		if (!error) {
			// Set logger
			rd_kafka_set_logger(rk, logger_kafka);
			rd_kafka_set_log_level(rk, LOG_DEBUG);

			// Add brokers
			if (rd_kafka_brokers_add(rk, brokers) == 0) {
				LOG_PRINT("Intento fallido para kafka broker");
				error = 1;
			}
			else {
				LOG_PRINT("Intento con exito para kafka broker");
			}

			if (!error) {
				// Create topic
				rkt = rd_kafka_topic_new(rk, topics, topic_conf);
				if (rkt)
					LOG_PRINT("Intento con exito para kafka topic");
				else {
					LOG_PRINT("Intento fallido para kafka topic");
					error = 1;
				}
			}
		}
	}

	//TODO: kafka configurado pero NO EN USO
	//Antes de empezar, debemos lanzar un hilo que hago poll
	//TODO: pthread_create(p_kafka_poll,NULL,poll_kafka,NULL);
	//-----------------------------KAFKA-----------------------------



	return error;
}


/*
 * Funcion: int carga_config()
 * 		Funcion que llamara a los parseadores de los
 * 		ficheros de configuracion e intentara cargar la
 * 		configuracion hasta que sea correcta
 * 		NO CONTEMPLA la lista vacia
 */
int carga_config()  {
	//Valor devuelto
	int error = 0;

	//--------------CONFIGURACION--------------
	LOG_PRINT("Intento cargar configuracion general");
	//Lamamos a la funcion que lee la configuracion
	do {
		error_confic = 0;
		//Carga la configuracion general -> Guarda en variables poller.h
		carga_conf();
	} while ((error_confic==1)||(fin==0));

	if (error_confic != 1)
		LOG_PRINT("Configuracion general cargada correctamente");
	else {
		LOG_PRINT("Fallo en carga de configuracion general");
		error=1;
	}


	LOG_PRINT("Configuracion general:");
	LOG_PRINT("Numero maximo de hilos: %d",max_hilos);
	LOG_PRINT("Broker de kafka: %s",brokers);
	LOG_PRINT("Topic de kafka: %s",topics);
	LOG_PRINT("Tiempo monitorizacion: %d",t_monitor);

	LOG_PRINT("Intento cargar configuracion hosts");
	//Llamamos a la funcion que crea la estructura dinamica
	do {
		error_estdin = 0;
		//Carga la configuracion hosts -> Gaurda en lista_hosts en poller.h
		carga_monitor();
	} while ((error_estdin==1)||(fin==0));
	if (error_estdin != 1)
		LOG_PRINT("Configuracion hosts cargada correctamente");
	else {
		LOG_PRINT("Fallo en carga de configuracion hosts");
		error=1;
	}


	if (lista_hosts == NULL) {
		error=1;
		LOG_PRINT("Error en creacion de estructura dinamica");
	}
	else {
		LOG_PRINT("Creacion de estructura dinamica correcta");
	}

	return error;

}
