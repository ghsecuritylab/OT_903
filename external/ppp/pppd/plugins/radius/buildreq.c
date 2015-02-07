

#include <includes.h>
#include <radiusclient.h>

unsigned char rc_get_seqnbr(void);


int rc_get_nas_id(VALUE_PAIR **sendpairs)
{
	UINT4		client_id;
	char *nasid;

	nasid = rc_conf_str("nas_identifier");
	if (strlen(nasid)) {
		/*
		 * Fill in NAS-Identifier
		 */
		if (rc_avpair_add(sendpairs, PW_NAS_IDENTIFIER, nasid, 0,
				  VENDOR_NONE) == NULL)
			return (ERROR_RC);

		return (OK_RC);
	  
	} else {
		/*
		 * Fill in NAS-IP-Address
		 */
		if ((client_id = rc_own_ipaddress()) == 0)
			return (ERROR_RC);

		if (rc_avpair_add(sendpairs, PW_NAS_IP_ADDRESS, &client_id,
				  0, VENDOR_NONE) == NULL)
			return (ERROR_RC);
	}

	return (OK_RC);
}


void rc_buildreq(SEND_DATA *data, int code, char *server, unsigned short port,
		 int timeout, int retries)
{
	data->server = server;
	data->svc_port = port;
	data->seq_nbr = rc_get_seqnbr();
	data->timeout = timeout;
	data->retries = retries;
	data->code = code;
}


static unsigned char rc_guess_seqnbr(void)
{
	return (unsigned char)(magic() & UCHAR_MAX);
}


unsigned char rc_get_seqnbr(void)
{
	FILE *sf;
	int tries = 1;
	int seq_nbr, pos;
	char *seqfile = rc_conf_str("seqfile");

	if ((sf = fopen(seqfile, "a+")) == NULL)
	{
		error("rc_get_seqnbr: couldn't open sequence file %s: %s", seqfile, strerror(errno));
		/* well, so guess a sequence number */
		return rc_guess_seqnbr();
	}

	while (do_lock_exclusive(fileno(sf))!= 0)
	{
		if (errno != EWOULDBLOCK) {
			error("rc_get_seqnbr: flock failure: %s: %s", seqfile, strerror(errno));
			fclose(sf);
			return rc_guess_seqnbr();
		}
		tries++;
		if (tries <= 10)
			rc_mdelay(500);
		else
			break;
	}

	if (tries > 10) {
		error("rc_get_seqnbr: couldn't get lock after %d tries: %s", tries-1, seqfile);
		fclose(sf);
		return rc_guess_seqnbr();
	}

	pos = ftell(sf);
	rewind(sf);
	if (fscanf(sf, "%d", &seq_nbr) != 1) {
		if (pos != ftell(sf)) {
			/* file was not empty */
			error("rc_get_seqnbr: fscanf failure: %s", seqfile);
		}
		seq_nbr = rc_guess_seqnbr();
	}

	rewind(sf);
	ftruncate(fileno(sf),0);
	fprintf(sf,"%d\n", (seq_nbr+1) & UCHAR_MAX);

	fflush(sf); /* fflush because a process may read it between the do_unlock and fclose */

	if (do_unlock(fileno(sf)) != 0)
		error("rc_get_seqnbr: couldn't release lock on %s: %s", seqfile, strerror(errno));

	fclose(sf);

	return (unsigned char)seq_nbr;
}


int rc_auth(UINT4 client_port, VALUE_PAIR *send, VALUE_PAIR **received,
	    char *msg, REQUEST_INFO *info)
{
    SERVER *authserver = rc_conf_srv("authserver");

    if (!authserver) {
	return (ERROR_RC);
    }
    return rc_auth_using_server(authserver, client_port, send, received,
				msg, info);
}


int rc_auth_using_server(SERVER *authserver,
			 UINT4 client_port,
			 VALUE_PAIR *send,
			 VALUE_PAIR **received,
			 char *msg, REQUEST_INFO *info)
{
	SEND_DATA       data;
	int		result;
	int		i;
	int		timeout = rc_conf_int("radius_timeout");
	int		retries = rc_conf_int("radius_retries");

	data.send_pairs = send;
	data.receive_pairs = NULL;

	/*
	 * Fill in NAS-IP-Address or NAS-Identifier
	 */

	if (rc_get_nas_id(&(data.send_pairs)) == ERROR_RC)
	    return (ERROR_RC);

	/*
	 * Fill in NAS-Port
	 */

	if (rc_avpair_add(&(data.send_pairs), PW_NAS_PORT, &client_port, 0, VENDOR_NONE) == NULL)
		return (ERROR_RC);

	result = ERROR_RC;
	for(i=0; (i<authserver->max) && (result != OK_RC) && (result != BADRESP_RC)
		; i++)
	{
		if (data.receive_pairs != NULL) {
			rc_avpair_free(data.receive_pairs);
			data.receive_pairs = NULL;
		}
		rc_buildreq(&data, PW_ACCESS_REQUEST, authserver->name[i],
			    authserver->port[i], timeout, retries);

		result = rc_send_server (&data, msg, info);
	}

	*received = data.receive_pairs;

	return result;
}


int rc_auth_proxy(VALUE_PAIR *send, VALUE_PAIR **received, char *msg)
{
	SEND_DATA       data;
	int		result;
	int		i;
	SERVER		*authserver = rc_conf_srv("authserver");
	int		timeout = rc_conf_int("radius_timeout");
	int		retries = rc_conf_int("radius_retries");

	data.send_pairs = send;
	data.receive_pairs = NULL;

	result = ERROR_RC;
	for(i=0; (i<authserver->max) && (result != OK_RC) && (result != BADRESP_RC)
		; i++)
	{
		if (data.receive_pairs != NULL) {
			rc_avpair_free(data.receive_pairs);
			data.receive_pairs = NULL;
		}
		rc_buildreq(&data, PW_ACCESS_REQUEST, authserver->name[i],
			    authserver->port[i], timeout, retries);

		result = rc_send_server (&data, msg, NULL);
	}

	*received = data.receive_pairs;

	return result;
}



int rc_acct_using_server(SERVER *acctserver,
			 UINT4 client_port,
			 VALUE_PAIR *send)
{
	SEND_DATA       data;
	VALUE_PAIR	*adt_vp;
	int		result;
	time_t		start_time, dtime;
	char		msg[4096];
	int		i;
	int		timeout = rc_conf_int("radius_timeout");
	int		retries = rc_conf_int("radius_retries");

	data.send_pairs = send;
	data.receive_pairs = NULL;

	/*
	 * Fill in NAS-IP-Address or NAS-Identifier
	 */

	if (rc_get_nas_id(&(data.send_pairs)) == ERROR_RC)
	    return (ERROR_RC);

	/*
	 * Fill in NAS-Port
	 */

	if (rc_avpair_add(&(data.send_pairs), PW_NAS_PORT, &client_port, 0, VENDOR_NONE) == NULL)
		return (ERROR_RC);

	/*
	 * Fill in Acct-Delay-Time
	 */

	dtime = 0;
	if ((adt_vp = rc_avpair_add(&(data.send_pairs), PW_ACCT_DELAY_TIME, &dtime, 0, VENDOR_NONE)) == NULL)
		return (ERROR_RC);

	start_time = time(NULL);
	result = ERROR_RC;
	for(i=0; (i<acctserver->max) && (result != OK_RC) && (result != BADRESP_RC)
		; i++)
	{
		if (data.receive_pairs != NULL) {
			rc_avpair_free(data.receive_pairs);
			data.receive_pairs = NULL;
		}
		rc_buildreq(&data, PW_ACCOUNTING_REQUEST, acctserver->name[i],
			    acctserver->port[i], timeout, retries);

		dtime = time(NULL) - start_time;
		rc_avpair_assign(adt_vp, &dtime, 0);

		result = rc_send_server (&data, msg, NULL);
	}

	rc_avpair_free(data.receive_pairs);

	return result;
}


int rc_acct(UINT4 client_port, VALUE_PAIR *send)
{
    SERVER *acctserver = rc_conf_srv("acctserver");
    if (!acctserver) return (ERROR_RC);

    return rc_acct_using_server(acctserver, client_port, send);
}


int rc_acct_proxy(VALUE_PAIR *send)
{
	SEND_DATA       data;
	int		result;
	char		msg[4096];
	int		i;
	SERVER		*acctserver = rc_conf_srv("authserver");
	int		timeout = rc_conf_int("radius_timeout");
	int		retries = rc_conf_int("radius_retries");

	data.send_pairs = send;
	data.receive_pairs = NULL;

	result = ERROR_RC;
	for(i=0; (i<acctserver->max) && (result != OK_RC) && (result != BADRESP_RC)
		; i++)
	{
		if (data.receive_pairs != NULL) {
			rc_avpair_free(data.receive_pairs);
			data.receive_pairs = NULL;
		}
		rc_buildreq(&data, PW_ACCOUNTING_REQUEST, acctserver->name[i],
			    acctserver->port[i], timeout, retries);

		result = rc_send_server (&data, msg, NULL);
	}

	rc_avpair_free(data.receive_pairs);

	return result;
}


int rc_check(char *host, unsigned short port, char *msg)
{
	SEND_DATA       data;
	int		result;
	UINT4		service_type;
	int		timeout = rc_conf_int("radius_timeout");
	int		retries = rc_conf_int("radius_retries");

	data.send_pairs = data.receive_pairs = NULL;

	/*
	 * Fill in NAS-IP-Address or NAS-Identifier,
         * although it isn't neccessary
	 */

	if (rc_get_nas_id(&(data.send_pairs)) == ERROR_RC)
	    return (ERROR_RC);

	/*
	 * Fill in Service-Type
	 */

	service_type = PW_ADMINISTRATIVE;
	rc_avpair_add(&(data.send_pairs), PW_SERVICE_TYPE, &service_type, 0, VENDOR_NONE);

	rc_buildreq(&data, PW_STATUS_SERVER, host, port, timeout, retries);
	result = rc_send_server (&data, msg, NULL);

	rc_avpair_free(data.receive_pairs);

	return result;
}
