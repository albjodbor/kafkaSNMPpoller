#include "conf.h"
#include "logger.h"


/*
 * Funcion: void carga_monitor ()
 * Parseara el fichero de monitorizacion
 */
void carga_monitor () {
	//TODO por linea de comandos
	char rutaconf [] = "./monitor.json";
	json_error_t error;

	//Configuracion sensores
	json_t *sensores;
	json_t *sensor;
	json_t *sensor_name,*sensor_ip,*sensor_community,*sensor_snmp_version;
	const char *nombre_sensor;
	const char *ip_sensor;
	const char *comunidad_sensor;
	const char *version_sensor;

	int tam_nombre_sensor;
	int tam_ip_sensor;
	int tam_comunidad_sensor;
	int tam_version_sensor;


	//Configuracion OID
	json_t *oids;
	json_t *oid;
	json_t *oid_name, *oid_oid;
	const char *nombre_oid;
	const char *id_oid;

	int tam_nombre_oid;
	int tam_id_oid;

	//Variables auxiliares
	int i;
	int j;
	int error_for = 1;
	int error_for2 = 1;

	int error_host=0;
	int error_oid=0;

	st_host * nodo_prov=NULL;
	st_oid * oid_prov=NULL;
	st_oid * lista_oid=NULL;

	/*
	 * Vamos a liberar la lista en el caso de que la hubiera
	 * Cuando recargamos la configuracion, creamos la lista de nuevo
	 */
	libera_host(lista_hosts);
	lista_hosts = NULL;


	/* parse text into JSON structure */
	sensores = json_load_file(rutaconf,0,&error);


	if (!sensores) {
		LOG_PRINT("error: on line %d:",error.line);
		LOG_PRINT("error: %s",error.text);
		error_estdin = 1;
		}
	else {
		//Primero comprobamos que es un array
		if (!(json_is_array(sensores))) {
			LOG_PRINT("error: El documento no es un array");
			error_estdin = 1;

		}
		else {
			for(i = 0; (i < json_array_size(sensores))&&(error_for); i++) {

				sensor = json_array_get(sensores,i);
					if (!json_is_object(sensor)) {
						LOG_PRINT("error: Datos de sensor no son un objeto");
						error_for = 0;
					}
					else {

						sensor_name= json_object_get(sensor,"sensor_name");
						sensor_ip= json_object_get(sensor,"sensor_ip");
						sensor_community= json_object_get(sensor,"community");
						sensor_snmp_version= json_object_get(sensor,"snmp_version");

						if(!json_is_string(sensor_name)) {
							LOG_PRINT("error: monitor.json: sensor_name no es un string");
							error_for = 0;
						}
						else if(!json_is_string(sensor_ip)) {
							LOG_PRINT("error: monitor.json: sensor_ip no es un string");
							error_for = 0;
						}
						else if(!json_is_string(sensor_community)) {
							LOG_PRINT("error: monitor.json: sensor_community no es un string");
							error_for = 0;
						}
						else if(!json_is_string(sensor_snmp_version)) {
							LOG_PRINT("error: monitor.json: sensor_snmp_version no es un string");
							error_for = 0;
						}
						else {

							//Creamos un nodo
							nodo_prov = (st_host *) malloc (sizeof(st_host));
							nodo_prov->next=NULL;
							nodo_prov->oids=NULL;

							if (nodo_prov != NULL) {
								//Llenamos con los datos

								//-----------NOMBRE-----------
								nombre_sensor = json_string_value(sensor_name);
								tam_nombre_sensor = strlen (nombre_sensor);
								nodo_prov->name = (char *)calloc(tam_nombre_sensor,sizeof(char));
								if (nodo_prov->name!=NULL)
									strcpy(nodo_prov->name,nombre_sensor);
								else {
									error_host = 1;
									LOG_PRINT("error: monitor.json: fallo memoria en host: name");
								}

								//-----------IP-----------
								if (error_host == 0) {

									ip_sensor = json_string_value(sensor_ip);
									tam_ip_sensor = strlen (ip_sensor);
									nodo_prov->ip = (char *)calloc(tam_ip_sensor,sizeof(char));
									if (nodo_prov->ip != NULL)
										strcpy(nodo_prov->ip,ip_sensor);
									else {
										error_host = 1;
										LOG_PRINT("error: monitor.json: fallo memoria en host: ip");
									}
								}

								//-----------COMUNIDAD-----------
								if (error_host == 0) {
									comunidad_sensor = json_string_value(sensor_community);
									tam_comunidad_sensor = strlen (comunidad_sensor);
									nodo_prov->community = (char *)calloc(tam_comunidad_sensor,sizeof(char));
									if (nodo_prov->community!=NULL)
										strcpy(nodo_prov->community,comunidad_sensor);
									else {
										error_host = 1;
										LOG_PRINT("error: monitor.json: fallo memoria en host: comunidad");
									}

								}

								//-----------VERSION-----------
								if (error_host == 0) {
									version_sensor = json_string_value(sensor_snmp_version);
									tam_version_sensor = strlen (version_sensor);
									nodo_prov->version = (char *)calloc(tam_version_sensor,sizeof(char));
									if(nodo_prov->version)
										strcpy(nodo_prov->version,version_sensor);
									else {
										error_host = 1;
										LOG_PRINT("error: monitor.json: fallo memoria en host: version");
									}

								}

								//-----------NEXT-----------
								nodo_prov->next = NULL;

								if (error_host == 1) {
									//Ha habido fallo de memoria
									//Libero el nodo y paso al siguiente
									if (nodo_prov->name != NULL) {
										free(nodo_prov->name);
										if (nodo_prov->ip != NULL) {
											free(nodo_prov->ip);
											if (nodo_prov->community != NULL) {
												free (nodo_prov->community);
												if (nodo_prov->version!=NULL) {
													free(nodo_prov->version);
												}
											}
										}
									}

									//Libero el nodo
									free(nodo_prov);
									//Dejo un mensaje en el log
									LOG_PRINT("error: monitor.json: Nodo %d no incluido en la estructura",i+1);
								}
							}
						}
						if (nodo_prov == NULL) {
							LOG_PRINT("error: monitor.json: no se ha podido crear nodo %d",i+1);
							error_for=0;
						}
						//Si ha habido algun error, salgo
						if (error_for != 0) {

							//Obtenemos la lista de OID
							oids = json_object_get(sensor,"monitors");

							if(!json_is_array(oids)) {
								error_for = 0;
								LOG_PRINT("error: monitor.json: monitors no es un array");
							}

							else {

								for(j = 0; (j < json_array_size(oids))&&(error_for2); j++) {
									oid = json_array_get(oids,j);
									if (!json_is_object(oid)) {
										LOG_PRINT("error: monitor.json: Datos de OID no es un objeto");
										error_for2 = 0;
									}
									else {
										oid_name= json_object_get(oid,"name");
										oid_oid= json_object_get(oid,"oid");
										if(!json_is_string(oid_name)) {
											LOG_PRINT("error: monitor.json: oid_name no es un string");
											error_for2 = 0;
										}
										else if(!json_is_string(oid_oid)) {
											LOG_PRINT("error: monitor.json: oid_oid no es un string");
											error_for2 = 0;
										}
										else {
											//Creamos un oid
											oid_prov = (st_oid *) malloc (sizeof(st_oid));
											oid_prov->next=NULL;
											if (oid_prov != NULL) {
												//TODO cancelar llenado si fallo de memoria
												//Llenamos con los datos
												//-----------NOMBRE-----------
												nombre_oid = json_string_value(oid_name);
												tam_nombre_oid = strlen (nombre_oid);
												oid_prov->name = (char *)calloc(tam_nombre_oid,sizeof(char));
												if (oid_prov->name!=NULL)
													strcpy(oid_prov->name,nombre_oid);
												else {
													error_oid = 1;
													LOG_PRINT("error: monitor.json: fallo memoria en oid: name");
												}
												//-----------OID-----------
												if (error_oid != 1) {
													id_oid = json_string_value(oid_oid);
													tam_id_oid = strlen (id_oid);
													oid_prov->oid_name = (char *)calloc(tam_id_oid,sizeof(char));
													if (oid_prov->oid_name!=NULL)
														strcpy(oid_prov->oid_name,id_oid);
													else {
														error_oid = 1;
														LOG_PRINT("error: monitor.json: fallo memoria en oid: oid_name");
													}
												}
												if (error_oid == 1) {
													//Ha habido fallo de memoria, libero el oid
													if (oid_prov->name!= NULL) {
														free(oid_prov->name);
														if(oid_prov->oid_name!=NULL) {
															free(oid_prov->oid_name);
														}
													}
													//Libero el oid
													free(oid_prov);
													//Dejo un mensaje en el log
													LOG_PRINT("error: monitor.json: Oid %d no incluido en la estructura",j+1);
												}

											}

											if (oid_prov == NULL) {
												LOG_PRINT("error: monitor.json: error memoria en oid");
												error_for2 = 0;
											}
											else {
												anida_oid(&nodo_prov->oids,oid_prov);
											}

										}
									}
								}
								if (error_for2 == 0) {
									error_for = 0;
								}
								else {
									anida_host(&lista_hosts,nodo_prov);
								}

							}
						}
					}
			}
			if (error_for == 0) {
				libera_host(lista_hosts);
				lista_hosts = NULL;
				error_estdin = 1;
			}
		}
	}
	json_decref(sensores);
}

/*
 * Funcion: void carga_conf ()
 * Parseara el fichero de configuracion
 */
void carga_conf () {

	/*
	 * Inicializa los valores de configuracion general:
	 * - broker
	 * - topic
	 * - max_hilos
	 */
	char rutaconf [] = "./config.json";
	json_error_t error;


	//Configuracion general
	json_t *conf;
	json_t *num_hilos,*broker,*topic,*t_monit;

	json_int_t thread;
	json_int_t tmp_monitor;
	const char *kafka_broker_json;
	const char *kafka_topic_json;

	int tam_broker;
	int tam_topic;

	/* parse text into JSON structure */
	json_t *root = json_load_file(rutaconf,0,&error);

	if (!root) {
		error_confic = 1;
		LOG_PRINT("error: on line %d:",error.line);
		LOG_PRINT("error: %s",error.text);
	}
	else
	{
		//Primero comprobamos que es un objeto
		if (!(json_is_array(root))) {
			error_confic = 1;
			LOG_PRINT("error: El documento no es un array");
		}
		else {
			//Si es un array, continuamos
			conf = json_array_get(root,0);
			if (!json_is_object(conf)) {
				error_confic = 1;
				LOG_PRINT("error: config.json: Datos conf no es un objeto");
			}
			else {
				//Si es un objeto, continuamos
				num_hilos = json_object_get(conf,"threads");
				broker = json_object_get(conf,"kafka_broker");
				topic = json_object_get(conf,"kafka_topic");
				t_monit = json_object_get(conf,"t_monit");


			    if(!json_is_integer(num_hilos))
			    {
			    	error_confic = 1;
			    	LOG_PRINT("error: config.json: thread no es un numero");
			    }
			    else if(!json_is_string(broker)) {
			    	error_confic = 1;
			    	LOG_PRINT("error: config.json: broker no es un string");
			    }
			    else if(!json_is_string(topic)) {
			    	error_confic = 1;
			    	LOG_PRINT("error: config.json: topic no es un string");
			    }
			    else if(!json_is_integer(t_monit)) {
			    	error_confic = 1;
			    	LOG_PRINT("error: config.json: t_monitor no es un numero");
			    }
			    else {

			    	//----------HILOS----------
			    	thread = json_integer_value(num_hilos);
			    	max_hilos = (int) thread;
			    	//----------T_MONITOR----------
			    	tmp_monitor = json_integer_value(t_monit);
			    	t_monitor = (int) tmp_monitor;
			    	//----------BROKER----------
			    	kafka_broker_json= json_string_value(broker);
			    	tam_broker = strlen (kafka_broker_json);
			    	brokers = (char *)calloc(tam_broker,sizeof(char));
			    	//Copia kafka_broker_json en broker
			    	strcpy(brokers,kafka_broker_json);
			    	//----------TOPIC----------
			    	kafka_topic_json= json_string_value(topic);
			    	tam_topic = strlen(kafka_topic_json);
			    	topics = (char *)calloc(tam_topic,sizeof(char));
			    	//Copia kafka_topic_json en broker
			    	strcpy(topics,kafka_topic_json);

			    }
			}
		}
	}
	json_decref(root);
}

