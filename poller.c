/*
 * Programa: Poller SNMP
 * Autor: Alberto Jodar
 * Fecha: 29-06-2015
 * Descripción:
 * Trabajo fin de grado.
 * TODO
 * 1) Comprobar que se haya abierto kafka -- eugenio
 * 3) Envio por kafka -- pruebas
 * 4) Mas versiones SNMP -- lo ultimo
 */

#include "poller.h"

#include "estructura.h"
#include "poller_thread.h"
#include "logger.h"
#include "productor_kafka.h"
#include "conf.h"

//----------------GLOBALES----------------

int main () {

//------------------FLAGS------------------


//----------------VARIABLES----------------
	int kafkaoff = 1;
	int brokeroff = 1;

	int kafka_conf_ok = 1;
	int estructura_ok = 1;

	st_host * lista_host_prov;

	pthread_t lectura;
	pthread_t * p_lectura = &lectura;
	pthread_t kafka_poll;
	pthread_t * p_kafka_poll = &kafka_poll;


	LOG_PRINT("Iniciando el programa....");
	fin = 1;
	fin_lectura=1;
	sesiones_activas = 0;
	recarga_config = 0;

	//Registramos los handler
	signal(SIGINT, handlerCierre);
	signal (SIGHUP, recargaConfig);

	//Inicializamos el descriptor fdset vacio. fdset variable global
	FD_ZERO(&fdset);

	lista_hosts=NULL;

	//------------------------------CONFIGURACION------------------------------
	//Esta en esta funcion hasta que se cargue la configuracion o SIGINT
	if(carga_config()) {
		//Se ha producido un error
		LOG_PRINT("La lista esta vacia");
		estructura_ok=0;
	}
	else
		imprime_lista(lista_hosts);
	//No salimos hasta que no hayamos cargado correctamente la simulacion

	//------------------------------KAFKA------------------------------
	//TODO: Reintentar hasta ser correcta la config de kafka
	if (config_kafka()) {
		//Se ha producido un error
		LOG_PRINT("Error en kafka");
		kafka_conf_ok = 0;
		/*
		 * Activamos este flag, para que, si no se ha configurado kafka
		 * correctamente, saltarnos el cierre de kafka al final
		 */
	}
	//------------------------------ESTRUCTURA------------------------------
	//Se llega a este punto solo si configuracion correcta o fin
	if (fin != 0)
	{
		config_estructura ();
		imprime_cortes(lista_hosts);
	}

	//------------------------------HILOS------------------------------
	/*
	 * Tenemos la lista marcada en las variables corte
	 * A cada hilo le pasaremos el puntero a un nodo, y
	 * monitorizara hasta que encuentre corte = 1 o NULL
	 */
	if (fin != 0) {

		//-----------------HILO POLLER-----------------
		lanzado_hilos();

		//TODO provisional
		lista_host_prov = lista_hosts;
		while (lista_host_prov != NULL) {
			fprintf(stdout,"Identificador de hilo %lu\n",lista_host_prov->thread);
			lista_host_prov=lista_host_prov->next;
		}


		//-----------------HILO LECTURA-----------------
		//Lanzamos un hilo para la lectura
		pthread_create(p_lectura,NULL,hilo_lectura,NULL);
		//-----------------HILO POLL KAFKA-----------------
		//TODO
	}

	//Los hilos solo terminara ante SIGINT
	/*
	 * Tenemos que esperar la finalizacion de cada hilo
	 * usando solo el puntero al hilo del nodo que sirve como corte,
	 * si no, estamos repitiendo
	 */
	if (hilos_lanzados > 0) {
		lista_host_prov = lista_hosts;
		while (lista_host_prov->next != NULL) {
			if (lista_host_prov->corte == 1) {
				pthread_join(lista_host_prov->thread,NULL);
			}
			lista_host_prov = lista_host_prov->next;
		}
		//Sale cuando estamos en el ultimo nodo
		pthread_join(lista_host_prov->thread,NULL);
		pthread_join(lectura,NULL);
	}

	//Este es el ultimo hilo que debemos esperar
	//TODO: pthread_join(kafka_poll,NULL);

	/*if (kafka_conf_ok == 1)
	{
		//Finalizamos el productor KAFKA
		// Destroy topic
		rd_kafka_topic_destroy(rkt);
		// Destroy the handle
		rd_kafka_destroy(rk);
		// Let background threads clean up and terminate cleanly.
		rd_kafka_wait_destroyed(2000);
	}*/
	libera_host(lista_hosts);

return 0;

}

/*
 * Funcion: void handlerCierre (int dummy)
 * 		Funcion que detectara la señal de cierre.
 * 		Este handler, ante la recepcion de Ctrl-C (SIGINT)
 * 		cerrara todas las sesiones y finalizara el programa
 */
void handlerCierre (int dummy) {
	fin = 0;
}

/*
 * Funcion: void recargaConfig(int dummy)
 * 		Funcion que recarga la configuracion y creara la estructura dinamica
 * 		Borrara y reconstruira cortes
 */
void recargaConfig(int dummy) {
	st_host * lista_host_prov;
	int estructura_ok = 1;

	recarga_config = 1;
	//Esto hace que terminen los hillos

	//Esperamos la finalizacion de los hilos de poller
	lista_host_prov = lista_hosts;
	while (lista_host_prov->next != NULL) {
		if (lista_host_prov->corte == 1) {
			pthread_join(lista_host_prov->thread,NULL);
		}
		lista_host_prov = lista_host_prov->next;
	}
	//Sale cuando estamos en el ultimo nodo
	pthread_join(lista_host_prov->thread,NULL);

	//Una vez han finalizado todos los hilos:
	libera_host(lista_hosts);
	lista_hosts = NULL;
	//------------------------------CONFIGURACION------------------------------
	//Esta en esta funcion hasta que se cargue la configuracion o SIGINT
	if(carga_config()) {
		//Se ha producido un error
		LOG_PRINT("La lista esta vacia");
		estructura_ok=0;
		//Si ha habido error en la nueva configuracion-> FIN
		fin = 0;
	}
	else
		imprime_lista(lista_hosts);
	//No salimos hasta que no hayamos cargado correctamente la simulacion
	//------------------------------ESTRUCTURA------------------------------
	//Se llega a este punto solo si configuracion correcta o fin
	if (fin != 0)
	{
		config_estructura ();
		imprime_cortes(lista_hosts);
	}
	//------------------------------HILOS------------------------------
	lanzado_hilos();

}
