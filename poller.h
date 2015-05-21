#ifndef DATOS_H
#include "datos.h"
#endif
//----------------VARIABLES----------------
int fin;

int t_monitor;

int fin_lectura;

int sesiones_activas;

/*
 * Descriptor de fichero fd_set. Nos sirve para leer las respuestas a las PDU enviadas
 * Al abrir la sesion snmp se añade un descriptor a fdset
 */
fd_set fdset;

//Globales para kafka
static rd_kafka_t *rk;
rd_kafka_topic_t *rkt;
