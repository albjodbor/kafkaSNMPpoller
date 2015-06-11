#ifndef DATOS_H
#include "datos.h"
#endif

//----------------FUNCIONES----------------
void handlerCierre (int dummy);
void recargaConfig (int dummy);

//----------------VARIABLES----------------
int fin;

int t_monitor;

int fin_lectura;

int sesiones_activas;

int error_estdin;
int error_confic;

int recarga_config;

int resto;
int host_hilo;

int hilos_lanzados;

/*
 * Descriptor de fichero fd_set. Nos sirve para leer las respuestas a las PDU enviadas
 * Al abrir la sesion snmp se añade un descriptor a fdset
 */
fd_set fdset;

//Globales para kafka
static rd_kafka_t *rk;
rd_kafka_topic_t *rkt;

//Estructura dinamica de host
st_host * lista_hosts;

//Valores de configuracion leidos desde fichero
char *brokers;
char *topics;
int max_hilos;
