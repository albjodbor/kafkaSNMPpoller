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


typedef struct oid {
  char Name[200];
  oid Oid[MAX_OID_LEN];
  size_t OidLen;
  struct oid *next;
}st_oid;

typedef struct host {
	char name[200];
	char community[200];
	st_oid *oids;
	pthread_t thread;
	int corte;
	struct snmp_session sesion;
	struct snmp_session *punt_sesion;
	int fallo_sesion;
	struct host *next;
}st_host;
