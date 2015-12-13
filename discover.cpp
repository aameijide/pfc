#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include <sys/socket.h>

#include <sys/types.h>

#include "libhdhomerun_20100213/hdhomerun.h"

using namespace std;

void create_message_UDP(struct hdhomerun_pkt_t *mis) {
	
	//~ uint32_t device_id = 0x2E0C19AF; //This number is false
	uint32_t device_id = 0x121097E9; //This number is the placed at UPF machine number
	uint32_t device_type = HDHOMERUN_DEVICE_TYPE_TUNER;
	
	hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_DEVICE_ID);
	hdhomerun_pkt_write_var_length(mis, 4);
	hdhomerun_pkt_write_u32(mis, device_id);
	hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_DEVICE_TYPE);
	hdhomerun_pkt_write_var_length(mis, 4);
	hdhomerun_pkt_write_u32(mis, device_type);
	hdhomerun_pkt_seal_frame(mis, HDHOMERUN_TYPE_DISCOVER_RPY);
}

///////////////////////////////////////////
///////////////////MAIN//////////////////
//////////////////////////////////////////

int main () {
	
	hdhomerun_sock_t sockUDP, sockTCP;
	struct sockaddr_in Adreca, Resposta;
	char text[40];

	// We create discovery message for send it through UDP
	struct hdhomerun_pkt_t *mis = hdhomerun_pkt_create();
	hdhomerun_pkt_reset(mis);
	
	create_message_UDP(mis);
	
	struct hdhomerun_pkt_t *mis2 = hdhomerun_pkt_create();
	hdhomerun_pkt_reset(mis2);

	//Socket opening
	sockUDP = hdhomerun_sock_create_udp();
	if (sockUDP == -1) { //Error Check
		printf("Error creating UDP socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	//Port association (bind) to socket and error check
	if (hdhomerun_sock_bind(sockUDP, INADDR_ANY, HDHOMERUN_DISCOVER_UDP_PORT) == -1) {
		printf("Error binding UDP socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	int i;
	
	uint32_t remote_addr;
	uint16_t remote_port;
	
	socklen_t mres = sizeof(struct sockaddr_in);
	
	size_t lengthResp;
	size_t length = mis->end - mis->start;
	size_t length2;
	
	std::cout << "Discovering active!" << endl;
	
	while(1) {
		
		//Recived from PC with error check
		if (hdhomerun_sock_recvfrom(sockUDP, &remote_addr, &remote_port, &text, &length, 40) > 0 ) {
			Adreca.sin_addr.s_addr = htonl(remote_addr); //We use it only for print IP
			std::cout << "UDP message recived! from IP " << inet_ntoa(Adreca.sin_addr) << " Port: " << remote_port << endl;
			if ((int)text[1]==2 && (int)text[4]==1 && (int)text[10]==2) { //Send HDHR ID
				hdhomerun_sock_sendto(sockUDP, remote_addr, remote_port, mis->start, length, 40);
				std::cout << "Discovery message sended! With length " << length << endl;
			}
		}
		
		for(i=0; i < 40; i++) { // Reset text
			text[i]==-1;
		}
		
	}
}
