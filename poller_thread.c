#include "poller_thread.h"


/*
 * Funcion: void * poller (void * thread_args)
 *
 * 		Funcion ejecutada por hilos. Se encarga del envio de pdu continuo.
 * 		Recibira una lista enlazada de los hosts que tiene que monitorizar
 */
void * poller (void * thread_args)
{

	//Banderas
	int fin_lista = 1;
	int corte = 1;
	int aux_fin = 1;


	//Punteros a la estructura
	st_host * lista_host_prov = (st_host *)thread_args;
	st_oid * lista_oid_prov;

	struct snmp_pdu *pregunta;

/*
 * Abrimos una sesion para cada host, almacenando los punteros
 * en la estructura dinamica
 * Levantamos la bandera fallo_sesion para las sesiones no abiertas.
 * Seran reintentadas posteriormente
 */
	while (corte) {
		  //Almacenamos el puntero en la estructura dinamica
		  snmp_sess_init(&(lista_host_prov->sesion));

		  /*
		   * VERSION 2c de momento
		   * TODO almacenar version en estructura dinamica
		   * Al menos añadir version 1
		   */


		  lista_host_prov->sesion.version = SNMP_VERSION_2c;

		  lista_host_prov->sesion.peername = strdup(lista_host_prov->ip);

		  lista_host_prov->sesion.community = (u_char*)strdup(lista_host_prov->community);
		  lista_host_prov->sesion.community_len = strlen(lista_host_prov->community);

		  //TODO Funcion de callback para cuando llega la pdu respuesta
		  lista_host_prov->sesion.callback = procesa_pdu;

		  lista_host_prov->punt_sesion = snmp_open(&(lista_host_prov->sesion));

		  //Comprobamos que se haya abierto correctamente
		  if (!(lista_host_prov->punt_sesion)) {
			  fprintf(stdout,"fallo sesion\n");
			  //Marcamos este host --> contador de reintentos
			  lista_host_prov->fallo_sesion = ESPERA_REINTENTOS;
		  }
		  else {
			  lista_host_prov->fallo_sesion = 0;
			  sesiones_activas=1;
		  }


		  //Llegamos a corte o a NULL -> paramos el bucle
		  if ((lista_host_prov->corte == 1)||(lista_host_prov->next==NULL))
			  corte = 0;
		  else
			  lista_host_prov = lista_host_prov->next;

	  }

	//Todas las sesiones abiertas o marcadas para reintento

	//BUCLE MONITORIZACION -> Finaliza solo ante Ctrl C
	  while (fin) {
		  lista_host_prov = (st_host *)thread_args;
		  do {
			  //Si hemos llegado al ultimo host, levantamos la bandera
			  if ((lista_host_prov->corte == 1) || (lista_host_prov->next == NULL))
				  fin_lista = 0;

			  //SI SESION ABIERTA -> ENVIO
			  if(lista_host_prov->fallo_sesion == 0) {
				  lista_oid_prov = lista_host_prov->oids;


				  //Lanzamos una pregunta por cada OID
				  while (lista_oid_prov != NULL) {
					  pregunta=NULL;
					  pregunta = snmp_pdu_create(SNMP_MSG_GET);

					  snmp_add_null_var(pregunta, lista_oid_prov->Oid, (int)lista_oid_prov->OidLen);

					  if (!(snmp_send(lista_host_prov->punt_sesion, pregunta))) {
						  snmp_perror("snmp_send");
						  snmp_free_pdu(pregunta);
					  }
				  lista_oid_prov = lista_oid_prov->next;
				  }
				  //OID lanzados
			  }

			  //SI SESION NO ABIERTA -> REINTENTO
			  else {
				  /*
				   * Solo vamos a reintentar cuando se cumplan tantas interacciones
				   * como nos marque el numero de reintentos. Así no saturamos con
				   * un numero excesivo de reintentos
				   *
				   */
				  if (lista_host_prov->fallo_sesion > 1) {
					  lista_host_prov->fallo_sesion--;
				  }
				  else {
					  //Reintentamos abrir la sesion cuando fallo_sesion sea 1
					  snmp_sess_init(&(lista_host_prov->sesion));
					  lista_host_prov->sesion.version = SNMP_VERSION_2c;
					  lista_host_prov->sesion.peername = strdup(lista_host_prov->name);
					  lista_host_prov->sesion.community = (u_char*)strdup(lista_host_prov->community);
					  lista_host_prov->sesion.community_len = strlen(lista_host_prov->community);
					  lista_host_prov->sesion.callback = procesa_pdu;
					  lista_host_prov->punt_sesion = snmp_open(&(lista_host_prov->sesion));

					  	if (!(lista_host_prov->punt_sesion)) {
					  		fprintf(stdout,"fallo sesion\n");
					  		lista_host_prov->fallo_sesion=ESPERA_REINTENTOS;
					  	}
					  	else {
					  		lista_host_prov->fallo_sesion=0;
					  		sesiones_activas=1;
					  	}
				  }
			  }

			  lista_host_prov = lista_host_prov->next;

		  } while (fin_lista);
		  //Acabamos de terminar la lista de host

		  //TODO -> Intervalo de monitorizacion global y desde fichero
		  //Esperamos el tiempo indicado -> INTERVALO MONITORIZACION
		  sleep(t_monitor);

	  }

	  /*
	   * Antes de empezar a cerrar las sesiones, comprobamos que el
	   * hilo de lectura ha finalizado
	   */
	  fin_lista=1;
	  lista_host_prov = (st_host *)thread_args;
	  while (fin_lista) {
		  if (fin_lectura == 0){
			  //Ya ha terminado el hilo de lectura
			  while (fin_lista) {
				  //Cerramos solo las sesiones que estan abiertas
				  if (lista_host_prov->fallo_sesion==0) {
					  snmp_close(lista_host_prov->punt_sesion);
				  }

				  if ((lista_host_prov->corte == 1)||(lista_host_prov->next==NULL))
					  fin_lista = 0;
				  lista_host_prov = lista_host_prov->next;
			  }
		  }
	  }



  return 0;
}


/*
 * Funcion: int procesa_pdu(int operation, struct snmp_session *sp,
 * 	int reqid, struct snmp_pdu *pdu, void *magic)
 *
 * 		Funcion que se definirá de callback en las sesiones snmp.
 * 		Se llamara cada vez que se recibe una trama pdu.
 * 		SU funcion principal sera enviar por kafka los
 * 		datos de la PDU recibida
 *
 * 		Parametros callback:
 * 		1)int operation				--> the possible operations are RECEIVED_MESSAGE and TIMED_OUT
 * 		2)struct snmp_session* session	--> the session that was authenticated using community
 * 		3)int reqid					--> the request ID identifying the transaction within this session. Use 0 for traps
 * 		4)struct snmp_pdu* pdu			--> a pointer to PDU information. You must copy the information because it will be freed elsewhere
 * 		5)void* magic					--> a pointer to the data for callback()
 */
int procesa_pdu(int operation, struct snmp_session *sp, int reqid, struct snmp_pdu *pdu, void *magic) {
	//TODO
	netsnmp_variable_list * vars;

	if (operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
		//Solo cuando hemos recibido pdu snmp
		print_result(STAT_SUCCESS, sp, pdu);
	}
/*
	//Variables para tiempo
	time_t rawtime;
	struct tm * timeinfo;

	int count=1;

	//Variables para los tipos de datos
	unsigned long long * result;

	for(vars = pdu->variables; vars; vars = vars->next_variable) {
		//Antes de imprimir, obtenemos el tiempo
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );

		 char *sp = NULL;
		 sp = malloc(1 + vars->val_len);
		 memcpy(sp, vars->val.string, vars->val_len);
		 sp[vars->val_len] = '\0';
		 fprintf(stdout,"%s \t tipo %d -->  %s\n",asctime (timeinfo), vars->type ,sp);
		 free(sp);
	}
*/
	 return 0;
}

/*
 * Funcion: void * hilo_lectura (void * thread_args)
 *
 * 		Funcion ejecutada en un hilo. Se encargará de recibir las
 * 		respuestas a las pdu enviadas por los poller a traves de la
 * 		funcion send
 */
void * hilo_lectura (void * thread_args) {

	//Variables auxiliares
	int fds = 0;
	int block = 1;
	int aux_fin = 1;
	struct timeval timeout;

	//Leo mientras no reciba Ctrl_C
	while(fin) {
		//Solo entro si hay sesiones activas
		if (sesiones_activas != 0)
		{
			snmp_select_info(&fds, &fdset, &timeout, &block);
			fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);
			if (fds){
				snmp_read(&fdset);
			}
			else {
				/*
				 * Si se produce un timeout es que no hay nada que leer de
				 * un determinado descriptor de fichero
				 */
				snmp_timeout();
			}
		}
	}

	/*
	 * Activo variable global, para saber que se ha terminado leer
	 * y poder cerrar las sesiones
	 */
	fin_lectura = 0;
	return 0;

}

int print_result (int status, struct snmp_session *sp, struct snmp_pdu *pdu)
{
  char buf[1024];
  struct variable_list *vp;
  int ix;
  struct timeval now;
  struct timezone tz;
  struct tm *tm;

  gettimeofday(&now, &tz);
  tm = localtime(&now.tv_sec);
  fprintf(stdout, "%.2d:%.2d:%.2d.%.6d ", tm->tm_hour, tm->tm_min, tm->tm_sec, (int)now.tv_usec);
  switch (status) {
  case STAT_SUCCESS:
    vp = pdu->variables;
    if (pdu->errstat == SNMP_ERR_NOERROR) {
      while (vp) {
        snprint_variable(buf, sizeof(buf), vp->name, vp->name_length, vp);
        fprintf(stdout, "%s: %s\n", sp->peername, buf);
	vp = vp->next_variable;
      }
    }
    else {
      for (ix = 1; vp && ix != pdu->errindex; vp = vp->next_variable, ix++)
        ;
      if (vp) snprint_objid(buf, sizeof(buf), vp->name, vp->name_length);
      else strcpy(buf, "(none)");
      fprintf(stdout, "%s: %s: %s\n",
      	sp->peername, buf, snmp_errstring(pdu->errstat));
    }
    return 1;
  case STAT_TIMEOUT:
    fprintf(stdout, "%s: Timeout\n", sp->peername);
    return 0;
  case STAT_ERROR:
    snmp_perror(sp->peername);
    return 0;
  }
  return 0;
}
