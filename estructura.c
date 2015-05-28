#include "estructura.h"
//-------------------------------------------------------------------


void anida_host (st_host ** lugar, st_host * nodo) {
	st_host * prov;
	prov = *lugar;
	*lugar = nodo;
	nodo->next = prov;
}

void anida_oid (st_oid ** lugar, st_oid * nodo) {
	st_oid * prov;
	prov = *lugar;
	*lugar = nodo;
	nodo->next = prov;
}

void libera_host (st_host * lista) {
	st_host * prov;
	while (lista!=NULL) {
		//
		free(lista->name);
		free(lista->ip);
		free(lista->community);
		free(lista->version);
		//1) Borramos su lista de oid
		libera_oid (lista->oids);
		prov = lista->next;
		free(lista);
		lista = prov;
	}
}

void libera_oid (st_oid * lista) {
	st_oid * prov;
	while (lista!= NULL) {
		free (lista->name);
		free (lista->oid_name);
		prov = lista->next;
		free (lista);
		lista = prov;
	}
}

void imprime_lista (st_host * lista) {
	st_oid * prov;
	st_host * lista_prov = lista;

	while (lista_prov!= NULL) {
		prov=lista_prov->oids;
		fprintf(stdout,"[%s]->",lista_prov->name);
		while (prov!=NULL) {
			fprintf(stdout,"[%s]->",prov->oid_name);
			prov = prov->next;
		}
		fprintf(stdout,"|NULL\n");
		lista_prov=lista_prov->next;
	}
}

void imprime_cortes (st_host * lista) {
	while (lista!=NULL) {
		fprintf(stdout,"[%s]->",lista->name);
		if (lista->corte == 1)
			fprintf(stdout,"\n");
		lista = lista->next;
	}
	fprintf(stdout,"\n");
}


void lista_cortes (st_host * lista, int host, int resto, int hilos) {

	int llenado = 0;
	int iteracciones = 0;

	if(host!=0)
	{
		while (lista != NULL)
		{
			llenado++;
			if (llenado == host) {
				llenado = 0;
				if (resto > 0) {
					resto--;
					lista = lista->next;
				}
				lista->corte=1;
			}
		lista = lista->next;
		}
	}
	else {
		//Caso de que haya mas hilos que host
		while (lista!=NULL) {
			if (lista->next != NULL)
				lista->corte=1;
			lista=lista->next;
		}
	}
}

void procesa_oid(st_host * lista_host)
{
	st_oid * lista_oid;
	while (lista_host != NULL) {
		lista_oid = lista_host->oids;
		while (lista_oid != NULL) {
			lista_oid->OidLen = sizeof(lista_oid->Oid)/sizeof(lista_oid->Oid[0]);
			if (!read_objid(lista_oid->oid_name, lista_oid->Oid, &lista_oid->OidLen)) {
			      snmp_perror("read_objid");
			      //exit(1);
			    }
			lista_oid = lista_oid->next;
		}
		lista_host = lista_host->next;
	}
}


