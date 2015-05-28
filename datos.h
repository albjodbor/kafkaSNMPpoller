#ifndef DATOS_H
#define DATOS_H
#endif


#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

//Libreria de NET_SNMP
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

//Libreria rdkafka
#include <librdkafka/rdkafka.h>

//Libreria jansson
#include <jansson.h>


typedef struct oid {
  char * name;
  char * oid_name;

  oid Oid[MAX_OID_LEN];
  size_t OidLen;

  struct oid *next;
}st_oid;

typedef struct host {
	char * name;
	char * ip;
	char * community;
	char * version;

	int corte;
	int fallo_sesion;

	struct snmp_session sesion;
	struct snmp_session *punt_sesion;

	pthread_t thread;

	st_oid *oids;

	struct host *next;
}st_host;
