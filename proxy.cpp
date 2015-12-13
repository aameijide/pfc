#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "libhdhomerun_20100213/hdhomerun.h"

#define MAX_LENGTH 400
#define HDHOMERUN_CONTROL_CONNECT_TIMEOUT 2500
#define HDHOMERUN_CONTROL_SEND_TIMEOUT 2500
#define HDHOMERUN_CONTROL_RECV_TIMEOUT 2500
#define HDHOMERUN_CONTROL_UPGRADE_TIMEOUT 20000

#define EXTPROGRAM_RTP_CASE 1
#define EXTPROGRAM_UDP_CASE 3
#define WAIT_TIME 10

using namespace std;

static uint32_t reverse_ip_addr(const char *str) { //Reverse the IP Adress
	unsigned long a[4];
	if (sscanf(str, "%lu.%lu.%lu.%lu", &a[0], &a[1], &a[2], &a[3]) != 4) {
		return 0;
	}

	return (uint32_t)((a[3] << 24) | (a[2] << 16) | (a[1] << 8) | (a[0] << 0));
}

uint32_t reverse_to_char(uint32_t ip) { //Save the adress in char* and reverse it
	const char *sending_ip;
	struct sockaddr_in Reverse;
	
	Reverse.sin_addr.s_addr = htonl((int) ip);
	sending_ip = inet_ntoa(Reverse.sin_addr);
	ip = reverse_ip_addr(sending_ip);
	return ip;
}

uint32_t call_discover(uint32_t *device) { //Find HDHR
	struct hdhomerun_discover_device_t result_list[64];
	uint32_t ip;
	int index, select=1;
	
	int count = hdhomerun_discover_find_devices_custom(0, HDHOMERUN_DEVICE_TYPE_TUNER, HDHOMERUN_DEVICE_ID_WILDCARD, result_list, 64);
	if (count < 0) {
		printf("error sending discover request\n");
		exit(0); //If error, exit program
	}
	if (count == 0) {
		printf("no devices found\n");
		exit(0); //If no devices, exit program
	}

	for (index = 0; index < count; index++) {
		struct hdhomerun_discover_device_t *result = &result_list[index];
		printf("%d: hdhomerun device %08lX found at %u.%u.%u.%u\n", index+1, (unsigned long)result->device_id, (unsigned int)(result->ip_addr >> 24) & 0x0FF, (unsigned int)(result->ip_addr >> 16) & 0x0FF, (unsigned int)(result->ip_addr >> 8) & 0x0FF, (unsigned int)(result->ip_addr >> 0) & 0x0FF);
	}
	
	if (count == 1) {
		struct hdhomerun_discover_device_t *result = &result_list[0];
		ip = result->ip_addr;
		ip = reverse_to_char(ip); //If we don't reverse the IP adress, sending will not work.
		return ip;
	}else{
		printf("Select one HDHR: ");
		scanf("%d", &select);
		while(select > count /*|| select < 0*/) {
			printf("Valid numbers are from 1 to %d, please select one of these: ", count);
			scanf("%d", &select);
		}
		struct hdhomerun_discover_device_t *result = &result_list[select-1];
		ip = result->ip_addr;
		ip = reverse_to_char(ip); //If we don't reverse the IP adress, sending will not work.
		*device = result->device_id;
		return ip;
	}
}

char* get_localhost(int interface) {
	struct ifreq   buffer[32];
	struct ifconf  intfc;
	struct ifreq  *pIntfc;
	int i=0, fd, num_intfc;
	char* ipaddr;
	
	intfc.ifc_len = sizeof(buffer);
	intfc.ifc_buf = (char*) buffer;
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Error to know localhost ip: Can't create socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	if (ioctl(fd, SIOCGIFCONF, &intfc) < 0) {
		printf("Error to know localhost ip: ioctl SIOCGIFCONF failed [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	pIntfc = intfc.ifc_req;
	num_intfc = intfc.ifc_len / sizeof(struct ifreq);
	
	if (num_intfc!=0) {
		struct ifreq *item = &(pIntfc[interface]);
		ipaddr = inet_ntoa(((struct sockaddr_in *)&item->ifr_addr)->sin_addr);
	}else{
		printf("No interficies found\n");
	}
	close(fd);
	return ipaddr + '\0';
}

bool is_rtp (char* text, size_t length) {
	
	int it, valor;
	
	while(it < length) {
		if(text[it] =='r' && text[it+1] =='t' && text[it+2] =='p' && text[it+3] ==':' && text[it+4] =='/' && text[it+5] =='/') {
			std::cout << text[it] << text[it+1] << text[it+2] << text[it+3] << text[it+4] << text[it+5] ;
			return true;
		}else{
			it++;
		}
	}
	
	return false;
}

void know_data_from_rtp_addr (char* text, size_t length, string *ip, string *port) {
	
	int i, valor;
	ostringstream aux, aux2;
	
	i=0;
	
	while(i < length) {
		if(text[i] =='r' && text[i+1] =='t' && text[i+2] =='p' && text[i+3] ==':' && text[i+4] =='/' && text[i+5] =='/') {
			valor = i;
			i = length +2;
		}else{
			i++;
		}
	}
	
	i = valor + 6;
	
	while (text[i] !=':') {
		aux << text[i];
		i++;
	}

	*ip = aux.str();
	
	i++;
	
	while (text[i] =='0' || text[i] =='1' || text[i] =='2' || text[i] =='3' || text[i] =='4' || text[i] =='5' || text[i] =='6' || text[i] =='7' || text[i] =='8' || text[i] =='9') {
		aux2 << text[i];
		i++;
	}
	
	*port = aux2.str();
}

void know_data_from_udp_addr (char* text, size_t length, string *ip, string *port) {
	
	int i, valor;
	ostringstream aux, aux2;
	
	i=0;
	
	while(i < length) {
		if(text[i] =='1' || text[i] =='2' || text[i] =='3' || text[i] =='4' || text[i] =='5' || text[i] =='6' || text[i] =='7' || text[i] =='8' || text[i] =='9') {
			valor = i;
			i = length +2;
		}else{
			i++;
		}
	}
	
	i = valor;
	
	while (text[i] !=':') {
		aux << text[i];
		i++;
	}

	*ip = aux.str();
	
	i++;
	
	while (text[i] =='0' || text[i] =='1' || text[i] =='2' || text[i] =='3' || text[i] =='4' || text[i] =='5' || text[i] =='6' || text[i] =='7' || text[i] =='8' || text[i] =='9') {
		aux2 << text[i];
		i++;
	}
	
	*port = aux2.str();
}

uint16_t search_free_port () {
	
	bool notavaible = true;
	hdhomerun_sock_t sock;
	uint16_t port;
	
	struct sockaddr_in my_addr;

        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(0);
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        memset(&(my_addr.sin_zero), '\0', 8);
	
	sock = hdhomerun_sock_create_udp();
	if (sock == -1) { //Error Check
		printf("Error creating UDP socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	while (notavaible == true) {	
		if (hdhomerun_sock_bind(sock, INADDR_ANY, 0) == -1) { //Port association (bind) to socket and error check
			printf("Error searching a port [errno %d : %s ]\n", errno, strerror(errno));
		}else{
			notavaible = false;
			port = hdhomerun_sock_getsockname_port(sock);
			hdhomerun_sock_destroy(sock);
		}
	}
	return port;
}

bool check_message (char *text, int type) {
	
	switch (type) {
		case 1: if (text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='t' && text[15]=='a' && text[16]=='r' && text[17]=='g' && text[18]=='e' && text[19]=='t' && (int)text[21]==4 && text[25]!='n' && text[26]!='o' && text[27]!='n' && text[28]!='e') {
			//	/tunerX/target !(none), where X can be anything
				return true;
			}else{
				return false;
			}
			break;
			
		case 2: if ((int)text[1]==4 && (int)text[4]==3 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='l' && text[15]=='o' && text[16]=='c' && text[17]=='k' && text[18]=='k' && text[19]=='e' && text[20]=='y') {
			// /tunerX/lockkey, where X can be anything
				return true;
			}else{
				return false;
			}
			break;
			
		case 3: if ((int)text[1]==4 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='p' && text[15]=='r' && text[16]=='o' && text[17]=='g' && text[18]=='r' && text[19]=='a' && text[20]=='m') {
			// /tuner<n>/program <program number>
				return true;
			}else{
				return false;
			}
			break;
		
		default: return false;
	}
}

bool send_by_senderbox(string udp_ip, uint16_t udp_port, string host, string port, int cas) {
	
	pid_t pid;
	ostringstream off, type;
	string uport, caso;
	
	off << udp_port;
	uport = off.str();
	
	type << cas;
	caso = type.str();
	
	pid = fork();
	if (pid < 0) {std::cout << "error doing pid!" << endl;}
	if (!pid) {
		int response = execl("./senderbox.exe", "./senderbox.exe", udp_ip.c_str(), uport.c_str(), host.c_str(), port.c_str(), caso.c_str(), (char *) 0);
		if (response != 0) { printf("Error executing senderbox: [errno %d : %s ]\n", errno, strerror(errno)); }
	
		return true;
	}
}

void print_text(char *text, size_t length, FILE *log) {
	
	int i;
	
	std::cout << "*********************************" << endl;
	fprintf(log, "*********************************\n");
	
	for (i=0; i<length; i++) {
		if (text[i] > 32 && text[i]!=127) {
			printf("%c", text[i]);
			fprintf(log, "%c", text[i]);
		}else{
			if (text[i] == 10) {
				printf("\n");
				fprintf(log, "\n");
			}else{
				printf(".");
				fprintf(log, ".");
			}
		}
		
	}
	
	printf("\n");
	fprintf(log, "\n");
	
	std::cout << "*********************************" << endl;
	fprintf(log, "*********************************\n");
}

uint create_message_to_HDHR(struct hdhomerun_pkt_t *mis, string ip, string type) {
	
	int name_len, value_len;
	uint16_t port;
	
	hdhomerun_pkt_reset(mis);
	
	const char *target = "/tuner0/target";
	
	ostringstream aux;
	port = search_free_port();
	aux << type << ip << ":" << port;
	string rtp = aux.str();

	name_len = (int)strlen(target) + 1;
	hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
	hdhomerun_pkt_write_var_length(mis, name_len);
	hdhomerun_pkt_write_mem(mis, (void *)target, name_len);
		
	value_len = (int)strlen(rtp.c_str()) + 1;
	hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
	hdhomerun_pkt_write_var_length(mis, value_len);
	hdhomerun_pkt_write_mem(mis, (void *)rtp.c_str(), value_len);
	
	hdhomerun_pkt_seal_frame(mis, HDHOMERUN_TYPE_GETSET_REQ);
	
	return port;
}

void create_message_to_program(struct hdhomerun_pkt_t *mis, string ip, string port, string type) {
	
	int name_len, value_len;
	
	const char *target = "/tuner0/target";
	
	ostringstream aux;
	aux << type << ip << ":" << port;
	string rtp = aux.str();

	name_len = (int)strlen(target) + 1;
	hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
	hdhomerun_pkt_write_var_length(mis, name_len);
	hdhomerun_pkt_write_mem(mis, (void *)target, name_len);
		
	value_len = (int)strlen(rtp.c_str()) + 1;
	hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
	hdhomerun_pkt_write_var_length(mis, value_len);
	hdhomerun_pkt_write_mem(mis, (void *)rtp.c_str(), value_len);
	
	hdhomerun_pkt_seal_frame(mis, HDHOMERUN_TYPE_GETSET_REQ);
}

///////////////////////////////////////////
///////////////////MAIN//////////////////
//////////////////////////////////////////

int main (int argc, char* argv[]) {
	
	hdhomerun_sock_t sockPROG, sockHDHR, acc, con;
	struct sockaddr_in Resposta;
	struct sockaddr HDHR, PROG;
	size_t Plength, Mlength, Mlength2, MIlength, OLDlength;
	time_t ltime;
	uint16_t udp_port;
	uint32_t device_id;
	char text_virt[MAX_LENGTH];
	char text_phy[MAX_LENGTH];
	char text_old[MAX_LENGTH];
	socklen_t mres = sizeof(struct sockaddr_in);
	int i, exit=0, j =0, verbose, num = 0, iterator=0;
	
	FILE *log = fopen ("log_proxy.txt", "w");
	bool ready_to_send = false;
	
	if (argc >= 2) {
		verbose = atoi(argv[1]);
		std::cout << "Verbose:";
		fprintf(log, "Verbose:");
		
		switch (verbose) {
			case 1:	std::cout << "1: All messages" << endl;
					fprintf(log, "1: All messages\n");
					break;
			case 2:	std::cout << "2: Send-recive adverts with messages" << endl;
					fprintf(log, "2: Send-recive adverts with messages\n");
					break;
			case 3:	std::cout << "3: Send-recive adverts" << endl;
					fprintf(log, "3: Send-recive adverts\n");
					break;
			case 4:	std::cout << "4: No messages" << endl;
					fprintf(log, "4: No messages\n");
					break;
			default:	verbose = 1;
					std::cout << "Default: All messages" << endl;
					fprintf(log, "Default: All messages\n");
					break;
		}
	}else{
		std::cout << "Verbose not selected: All messages will appear" << endl;
		fprintf(log, "Verbose not selected: All messages will appear\n");
		verbose = 1;
	}
	
	std::cout << endl;
	fprintf(log, "\n");
	
	struct hdhomerun_pkt_t *mis = hdhomerun_pkt_create();
	hdhomerun_pkt_reset(mis);
	if (verbose <= 1) {std::cout << "package mis create!" << endl; fprintf(log, "package mis create!\n");}
	struct hdhomerun_pkt_t *pkt = hdhomerun_pkt_create();
	hdhomerun_pkt_reset(pkt);
	if (verbose <= 1) {std::cout << "package pkt create!" << endl; fprintf(log, "package pkt create!\n");}
	
	string sip, localhost, simhost, simport;
	uint32_t ip;
	
	localhost =string(get_localhost(0)); // It's important know our localhost before we create our self sockets, else programm will not work!!
	simhost =string(get_localhost(2)); // It's important know our localhost before we create our self sockets, else programm will not work!!

	//Socket opening
	sockPROG = hdhomerun_sock_create_tcp();
	if (sockPROG == -1) { //Error Check
		printf("ERROR: Error creating client TCP socket [errno %d : %s ]\n", errno, strerror(errno));
		fprintf(log, "ERROR: Error creating client TCP socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	if (verbose <= 1) {std::cout << "Created socket to Program" << endl; fprintf(log, "Created socket to Program\n");}

	//Port association (bind) to socket and error check
	if (hdhomerun_sock_bind(sockPROG, INADDR_ANY, HDHOMERUN_CONTROL_TCP_PORT) == -1) {
		printf("ERROR: Error binding client TCP socket [errno %d : %s ]\n", errno, strerror(errno));
		fprintf(log, "ERROR: Error binding client TCP socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	if (verbose <= 1) {std::cout << "Binded socket to Program" << endl; fprintf(log, "Binded socket to Program\n");}
	
	//Socket opening
	sockHDHR = hdhomerun_sock_create_tcp();
	if (sockHDHR == -1) { //Error Check
		printf("ERROR: Error creating HDHR TCP socket [errno %d : %s ]\n", errno, strerror(errno));
		fprintf(log, "ERROR: Error creating HDHR TCP socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	if (verbose <= 1) {std::cout << "Created socket to HDHR" << endl; fprintf(log, "Created socket to HDHR\n");}

	//Port association (bind) to socket and error check
	if (hdhomerun_sock_bind(sockHDHR, INADDR_ANY, 0) == -1) {
		printf("ERROR: Error binding HDHR TCP socket [errno %d : %s ]\n", errno, strerror(errno));
		fprintf(log, "ERROR: Error binding HDHR TCP socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	if (verbose <= 1) {std::cout << "Binded socket to HDHR" << endl; fprintf(log, "Binded socket to HDHR\n");}
	
	if (argc >= 3) {
		ip =reverse_ip_addr(argv[2]);
	}else{
		ip = call_discover(&device_id);
	}
	
	Resposta.sin_addr.s_addr = htonl((int) ip); //Only need it to convert to char*
	sip = inet_ntoa(Resposta.sin_addr);
	
	//Start listening, with 5 in queue
	listen(sockPROG, 5);
	
	if (verbose <= 1) {std::cout << "Socket Program listening!" << endl << endl; fprintf(log, "Socket Program listening!\n");}
	
	std::cout << endl << "Proxy active! Listening at " << simhost << ":" << hdhomerun_sock_getsockname_port(sockPROG) << " Sending through: " << localhost << ":" << hdhomerun_sock_getsockname_port(sockHDHR) << endl;
	fprintf(log, "\nProxy active! Listening at %s:%i Sending through: %s:%i\n", simhost.c_str(), hdhomerun_sock_getsockname_port(sockPROG), localhost.c_str(), hdhomerun_sock_getsockname_port(sockHDHR));
	
	while(1) {
		acc = accept(sockPROG, (struct sockaddr *) &Resposta, &mres );
		if ( acc > 0 ) {
			if (verbose <=3) { std::cout << "ACCEPTED TCP connection" << endl; fprintf(log, "ACCEPTED TCP connection\n"); }
			con = hdhomerun_sock_connect(sockHDHR, inet_addr(sip.c_str()), HDHOMERUN_CONTROL_TCP_PORT, 40);
			if (con > 0) {
				if (verbose <=3) { std::cout << "CONNECTED with HDHR" << endl; fprintf(log, "CONNECTED with HDHR\n"); }
				while(1) {
					Mlength=MAX_LENGTH;
					Mlength2=MAX_LENGTH;
					text_virt[0]='\0';
					text_phy[0]='\0';
					
					if (hdhomerun_sock_recv(acc, &text_virt, &Mlength, HDHOMERUN_CONTROL_SEND_TIMEOUT) <= 0 ) 
					{
							if (verbose <=3) { std::cout << "BREAK;" << endl; fprintf(log, "BREAK;\n"); } //If we didn't recive, maybe need connect again
							break;
					}else{
							
						if (verbose <=3) { 
							std::cout << "<<< RECIVE from program with length " << Mlength << endl;
							fprintf(log, "<<< RECIVE from program with length %ud\n", Mlength);
							if (verbose <=2) { print_text(text_virt, Mlength, log); }
						}
					}
					
					iterator=0;
					while (iterator < Mlength) {
						text_old[iterator] = text_virt[iterator];
						iterator++;
					}
					OLDlength = Mlength;
					
					if (check_message (text_virt, 1)) { //	/tuner	/target !(none)
						if (verbose <=3) { std::cout << "Changed address from packet" << endl; fprintf(log, "Changed address from packet\n"); }

						if (is_rtp(text_virt, Mlength)) {
							know_data_from_rtp_addr(text_virt, Mlength, &simhost, &simport); //Parsing message for IP and port
							udp_port = create_message_to_HDHR(mis, localhost, "rtp://"); //Creating message to send to HDHR (Also check a free port)
							
							ready_to_send = send_by_senderbox(localhost, udp_port, simhost, simport, EXTPROGRAM_RTP_CASE); //Start sender program
							ltime = time(NULL);
							create_message_to_program(pkt, simhost, simport, "rtp://"); //Create message to send to Program
						}else{
							know_data_from_udp_addr(text_virt, Mlength, &simhost, &simport); //Parsing message for IP and port
							udp_port = create_message_to_HDHR(mis, localhost, ""); //Creating message to send to HDHR (Also check a free port)
							
							ready_to_send = send_by_senderbox(localhost, udp_port, simhost, simport, EXTPROGRAM_UDP_CASE); //Start sender program
							ltime = time(NULL);
							create_message_to_program(pkt, simhost, simport, "udp://"); //Create message to send to Program
						}
						
						MIlength = mis->end - mis->start;
						
						if (verbose <=3) { std::cout << ">>> Message to SEND to HDHR with length " << MIlength << " SAVED, but not send" << endl; fprintf(log, ">>> Message to SEND to HDHR with length %ud SAVED, but not send\n", MIlength);}
						
					}else{
						while (hdhomerun_sock_send(sockHDHR, text_virt, Mlength, HDHOMERUN_CONTROL_SEND_TIMEOUT) <= 0 ) {}
						if (verbose <=3) { std::cout << ">>> SEND to HDHR with length " << Mlength << endl; fprintf(log, ">>> SEND to HDHR with length %ud\n", Mlength);}
					}
					
					if (check_message (text_virt, 1)) { //	/tuner	/target !(none)
						if (verbose <=3) { std::cout << "WARNING: Nothing to RECIVE from HDHR" << endl; fprintf(log, "WARNING: Nothing to RECIVE from HDHR\n"); } //Send it again
					}else{
						while( hdhomerun_sock_recv(sockHDHR, &text_phy, &Mlength2, HDHOMERUN_CONTROL_SEND_TIMEOUT) <= 0 ) {
							if (verbose <=3) { std::cout << "ERROR: Error reciving from HDHR, sending data again" << endl; fprintf(log, "ERROR: Error reciving from HDHR, sending data again\n"); } //Send it again
							while (hdhomerun_sock_send(sockHDHR, text_virt, Mlength, HDHOMERUN_CONTROL_SEND_TIMEOUT) <= 0 ) {}
						}
					
						if (verbose <=3) { 
							std::cout << "<<< RECIVE from HDHR with length " << Mlength2 << endl;
							fprintf(log, "<<< RECIVE from HDHR with length %ud\n", Mlength2);
							if (verbose <=2) { print_text(text_phy, Mlength2, log); }
						}
					}
					
					if (check_message (text_virt, 1)) { //	/tuner	/target !(none)
						Plength = pkt->end - pkt->start;
						while (hdhomerun_sock_send(acc, pkt->start, Plength, HDHOMERUN_CONTROL_SEND_TIMEOUT) <= 0 ) {}
						if (verbose <=3) { std::cout << ">>> SEND to program with length " << Plength << endl; fprintf(log, ">>> SEND to program with length %ud\n", Plength);}
					}else{
						while (hdhomerun_sock_send(acc, text_phy, Mlength2, HDHOMERUN_CONTROL_SEND_TIMEOUT) <= 0 ) {}
						if (verbose <=3) { std::cout << ">>> SEND to program with length " << Mlength2 << endl; fprintf(log, ">>> SEND to program with length %ud\n", Mlength2);}
					}
					
					if (Mlength == 26) { // If program send a lockkey, we need to connect again
						if (check_message (text_virt, 2)) { // /tunerX/lockkey, where X can be anything
							if (verbose <=3) { std::cout << "Lock key break;" << endl; fprintf(log, "Lock key break;\n");}
							break;
						}
					}
					
					if (ready_to_send == true) {
						std::cout << "WARNING: MESSAGE IS WAITING TO BE SEND. It will be send in " << WAIT_TIME - (time(NULL) - ltime) << " seconds" << endl;
						//~ fprintf(log, "WARNING: MESSAGE IS WAITING TO BE SEND. It will be send in %s secons",asctime( localtime(&(time(NULL) - ltime)));
						if (time(NULL)>=ltime+WAIT_TIME) {
							while (hdhomerun_sock_send(sockHDHR, mis->start, MIlength, HDHOMERUN_CONTROL_SEND_TIMEOUT) <= 0 ) {}
							if (verbose <=3) { std::cout << "SEND SAVED message to HDHR with length " << MIlength << endl; fprintf(log, "SEND SAVED message to HDHR with length %ud\n", MIlength);}
							while( hdhomerun_sock_recv(sockHDHR, &text_phy, &Mlength2, HDHOMERUN_CONTROL_SEND_TIMEOUT) <= 0 ) {
								if (verbose <=3) { std::cout << "ERROR: Error reciving from HDHR, sending data again" << endl; fprintf(log, "ERROR: Error reciving from HDHR, sending data again\n"); } //Send it again
								while (hdhomerun_sock_send(sockHDHR, mis->start, MIlength, HDHOMERUN_CONTROL_SEND_TIMEOUT) <= 0 ) {}
							}
							
							if (verbose <=3) { 
								std::cout << "RECIVE from HDHR with length " << Mlength2 << endl;
								fprintf(log, "RECIVE from HDHR with length %ud\n", Mlength2);
								if (verbose <=2) { print_text(text_phy, Mlength2, log); }
							}
							
							ready_to_send = false;
						}
					}
				}
			}
		}
	}
}
