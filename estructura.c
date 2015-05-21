#include "estructura.h"
//-------------------------------------------------------------------



st_host * procesa_fichero_host () {


	st_host * lista_host = NULL;
	st_host * nodo_host;
	st_host ** p_lista_host = &lista_host;
	st_oid * lista_oid = NULL;
	st_oid * nodo_oid;
	st_oid ** p_lista_oid = &lista_oid;

	//TODO -> Lectura de fichero
	//TODO -> Puede dar fallo por /0 strcpystrcpy

	//PROVISIONAL: Creo la misma lista de oid para cada nodo
	nodo_oid = (st_oid *)malloc (sizeof(st_oid));
	strcpy(nodo_oid->Name,"IF-MIB::ifDescr.1");
	nodo_oid->next=NULL;
	anida_oid(p_lista_oid,nodo_oid);

	nodo_oid = (st_oid *)malloc (sizeof(st_oid));
	strcpy(nodo_oid->Name,"IF-MIB::ifAdminStatus.1");
	nodo_oid->next=NULL;
	anida_oid(p_lista_oid,nodo_oid);

	//Creo elemento 1 y lo anido
	nodo_host = (st_host *)malloc (sizeof(st_host));
	strcpy(nodo_host->name,"10.0.30.1");
	strcpy(nodo_host->community,"redBorder");
	//Leemos los oid y los anidamos
	nodo_host->oids = lista_oid;
	nodo_host->next = NULL;
	anida_host(p_lista_host,nodo_host);

	lista_oid=NULL;
	//PROVISIONAL: Creo la misma lista de oid para cada nodo
	nodo_oid = (st_oid *)malloc (sizeof(st_oid));
	strcpy(nodo_oid->Name,"IF-MIB::ifMtu.1");
	nodo_oid->next=NULL;
	anida_oid(p_lista_oid,nodo_oid);

	nodo_oid = (st_oid *)malloc (sizeof(st_oid));
	strcpy(nodo_oid->Name,"IF-MIB::ifAdminStatus.1");
	nodo_oid->next=NULL;
	anida_oid(p_lista_oid,nodo_oid);

	//Creo elemento 2 y lo anido
	nodo_host = (st_host *)malloc (sizeof(st_host));
	strcpy(nodo_host->name,"10.0.30.1");
	strcpy(nodo_host->community,"redBorder");
	//Leemos los oid y los anidamos
	nodo_host->oids = lista_oid;
	nodo_host->next = NULL;
	anida_host(p_lista_host,nodo_host);

	lista_oid=NULL;
	//PROVISIONAL: Creo la misma lista de oid para cada nodo
	nodo_oid = (st_oid *)malloc (sizeof(st_oid));
	strcpy(nodo_oid->Name,"IF-MIB::ifMtu.1");
	nodo_oid->next=NULL;
	anida_oid(p_lista_oid,nodo_oid);

	nodo_oid = (st_oid *)malloc (sizeof(st_oid));
	strcpy(nodo_oid->Name,"IF-MIB::ifAdminStatus.1");
	nodo_oid->next=NULL;
	anida_oid(p_lista_oid,nodo_oid);

	//Creo elemento 3 y lo anido
	nodo_host = (st_host *)malloc (sizeof(st_host));
	strcpy(nodo_host->name,"10.0.30.2");
	strcpy(nodo_host->community,"redBorder");
	//Leemos los oid y los anidamos
	nodo_host->oids = lista_oid;
	nodo_host->next = NULL;
	anida_host(p_lista_host,nodo_host);

	lista_oid=NULL;
	//PROVISIONAL: Creo la misma lista de oid para cada nodo
	nodo_oid = (st_oid *)malloc (sizeof(st_oid));
	strcpy(nodo_oid->Name,"IF-MIB::ifMtu.1");
	nodo_oid->next=NULL;
	anida_oid(p_lista_oid,nodo_oid);

	nodo_oid = (st_oid *)malloc (sizeof(st_oid));
	strcpy(nodo_oid->Name,"IF-MIB::ifAdminStatus.1");
	nodo_oid->next=NULL;
	anida_oid(p_lista_oid,nodo_oid);

	//Creo elemento 4 y lo anido
	nodo_host = (st_host *) malloc (sizeof(st_host));
	strcpy(nodo_host->name,"10.0.30.2");
	strcpy(nodo_host->community,"redBorder");
	//Leemos los oid y los anidamos
	nodo_host->oids = lista_oid;
	nodo_host->next = NULL;
	anida_host(p_lista_host,nodo_host);

	return lista_host;
}

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
			fprintf(stdout,"[%s]->",prov->Name);
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
			if (!read_objid(lista_oid->Name, lista_oid->Oid, &lista_oid->OidLen)) {
			      snmp_perror("read_objid");
			      //exit(1);
			    }
			lista_oid = lista_oid->next;
		}
		lista_host = lista_host->next;
	}
}


