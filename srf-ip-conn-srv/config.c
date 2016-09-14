#include "config.h"

#include <json.h>

#include <string.h>
#include <syslog.h>
#include <stdlib.h>

uint16_t config_port = 65100;
flag_t config_ipv4_only = 1;
char config_bind_ip_str[INET6_ADDRSTRLEN] = {0,};
uint16_t config_max_clients = 1000;
uint16_t config_client_login_timeout_sec = 10;
uint16_t config_client_timeout_sec = 30;
char config_server_password_str[SRF_IP_CONN_MAX_PASSWORD_LENGTH] = {0,};
uint16_t config_auth_fail_ip_ignore_sec = 5;
char config_pidfile_str[255] = {0,};

flag_t config_read(char *filename) {
	FILE *f;
	long fsize;
	char *buf;
	jsmn_parser json_parser;
	jsmntok_t tok[20];
	int json_entry_count;
	int i;
	char port_str[6] = {0,};
	char ipv4_only_str[2] = {0,};
	char bind_ip_str[INET6_ADDRSTRLEN] = {0,};
	char max_clients_str[6] = {0,};
	char client_login_timeout_sec_str[6] = {0,};
	char client_timeout_sec_str[6] = {0,};
	char server_password_str[SRF_IP_CONN_MAX_PASSWORD_LENGTH] = {0,};
	char auth_fail_ip_ignore_sec_str[6] = {0,};
	char pidfile_str[255] = {0,};

	f = fopen(filename, "r");
	if (f == NULL) {
		syslog(LOG_ERR, "config: can't open config file for reading: %s\n", filename);
		return 0;
	}

	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	rewind(f);

	buf = (char *)malloc(fsize);
	if (buf == NULL) {
		fclose(f);
		syslog(LOG_ERR, "config: can't allocate memory for config file reading: %s\n", filename);
		return 0;
	}

	if (fread(buf, 1, fsize, f) != fsize) {
		fclose(f);
		free(buf);
		syslog(LOG_ERR, "config: error reading file: %s\n", filename);
	}
	fclose(f);

	jsmn_init(&json_parser);
	json_entry_count = jsmn_parse(&json_parser, buf, fsize, tok, sizeof(tok) / sizeof(tok[0]));
	if (json_entry_count < 1 || tok[0].type != JSMN_OBJECT) {
		free(buf);
		syslog(LOG_ERR, "config: error parsing file: %s\n", filename);
		return 0;
	}

	for (i = 1; i < json_entry_count-1; i++) {
		if (json_compare_tok_key(buf, &tok[i], "port")) {
			json_get_value(buf, &tok[i+1], port_str, sizeof(port_str));
			i++;
		} else if (json_compare_tok_key(buf, &tok[i], "ipv4-only")) {
			json_get_value(buf, &tok[i+1], ipv4_only_str, sizeof(ipv4_only_str));
			i++;
		} else if (json_compare_tok_key(buf, &tok[i], "bind-ip")) {
			json_get_value(buf, &tok[i+1], bind_ip_str, sizeof(bind_ip_str));
			i++;
		} else if (json_compare_tok_key(buf, &tok[i], "max-clients")) {
			json_get_value(buf, &tok[i+1], max_clients_str, sizeof(max_clients_str));
			i++;
		} else if (json_compare_tok_key(buf, &tok[i], "client-login-timeout-sec")) {
			json_get_value(buf, &tok[i+1], client_login_timeout_sec_str, sizeof(client_login_timeout_sec_str));
			i++;
		} else if (json_compare_tok_key(buf, &tok[i], "client-timeout-sec")) {
			json_get_value(buf, &tok[i+1], client_timeout_sec_str, sizeof(client_login_timeout_sec_str));
			i++;
		} else if (json_compare_tok_key(buf, &tok[i], "server-password")) {
			json_get_value(buf, &tok[i+1], server_password_str, sizeof(server_password_str));
			i++;
		} else if (json_compare_tok_key(buf, &tok[i], "auth-fail-ip-ignore-sec")) {
			json_get_value(buf, &tok[i+1], auth_fail_ip_ignore_sec_str, sizeof(auth_fail_ip_ignore_sec_str));
			i++;
		} else if (json_compare_tok_key(buf, &tok[i], "pidfile")) {
			json_get_value(buf, &tok[i+1], pidfile_str, sizeof(pidfile_str));
			i++;
		} else {
			free(buf);
			syslog(LOG_ERR, "config: unexpected key at %u\n", tok[i].start);
			return 0;
		}
	}

	free(buf);

	if (port_str[0])
		config_port = atoi(port_str);
	if (ipv4_only_str[0])
		config_ipv4_only = (ipv4_only_str[0] == '1');
	if (bind_ip_str[0])
		strncpy(config_bind_ip_str, bind_ip_str, sizeof(config_bind_ip_str));
	if (max_clients_str[0])
		config_max_clients = atoi(max_clients_str);
	if (client_login_timeout_sec_str[0])
		config_client_login_timeout_sec = atoi(client_login_timeout_sec_str);
	if (client_timeout_sec_str[0])
		config_client_timeout_sec = atoi(client_timeout_sec_str);
	if (server_password_str[0])
		strncpy(config_server_password_str, server_password_str, sizeof(config_server_password_str));
	if (auth_fail_ip_ignore_sec_str[0])
		config_auth_fail_ip_ignore_sec = atoi(auth_fail_ip_ignore_sec_str);
	if (pidfile_str[0])
		strncpy(config_pidfile_str, pidfile_str, sizeof(config_pidfile_str));

	return 1;
}