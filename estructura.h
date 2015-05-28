#ifndef DATOS_H
#include "datos.h"
#endif

#include "poller.h"


void anida_host (st_host ** lugar, st_host * nodo);
void anida_oid (st_oid ** lugar, st_oid * nodo);
void libera_host (st_host * lista);
void libera_oid (st_oid * lista);
void imprime_lista (st_host * lista);
void lista_cortes (st_host * lista, int host, int resto, int hilos);
void imprime_cortes (st_host * lista);
void procesa_oid(st_host * lista_host);
