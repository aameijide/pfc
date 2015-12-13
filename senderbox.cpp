#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

#include <unistd.h>
#include <errno.h>

using namespace std;

int channel_to_fx(int ch) {
	switch (ch) {
		case 21:	return 474000000;
				break;
		case 22:	return 482000000;
				break;
		case 23:	return 490000000;
				break;
		case 24:	return 498000000;
				break;
		case 25:	return 506000000;
				break;
		case 26:	return 514000000;
				break;
		case 27:	return 522000000;
				break;
		case 28:	return 530000000;
				break;
		case 29:	return 538000000;
				break;
		case 30:	return 546000000;
				break;
		case 31:	return 554000000;
				break;
		case 32:	return 562000000;
				break;
		case 33:	return 570000000;
				break;
		case 34:	return 578000000;
				break;
		case 35:	return 586000000;
				break;
		case 36:	return 594000000;
				break;
		case 37:	return 602000000;
				break;
		case 38:	return 610000000;
				break;
		case 39:	return 618000000;
				break;
		case 40:	return 626000000;
				break;
		case 41:	return 634000000;
				break;
		case 42:	return 642000000;
				break;
		case 43:	return 650000000;
				break;
		case 44:	return 658000000;
				break;
		case 45:	return 666000000;
				break;
		case 46:	return 674000000;
				break;
		case 47:	return 682000000;
				break;
		case 48:	return 690000000;
				break;
		case 49:	return 698000000;
				break;
		case 50:	return 706000000;
				break;
		case 51:	return 714000000;
				break;
		case 52:	return 722000000;
				break;
		case 53:	return 730000000;
				break;
		case 54:	return 738000000;
				break;
		case 55:	return 746000000;
				break;
		case 56:	return 754000000;
				break;
		case 57:	return 762000000;
				break;
		case 58:	return 770000000;
				break;
		case 59:	return 778000000;
				break;
		case 60:	return 786000000;
				break;
		case 61:	return 794000000;
				break;
		case 62:	return 802000000;
				break;
		case 63:	return 810000000;
				break;
		case 64:	return 818000000;
				break;
		case 65:	return 826000000;
				break;
		case 66:	return 834000000;
				break;
		case 67:	return 842000000;
				break;
		case 68:	return 850000000;
				break;
		case 69:	return 858000000;
				break;
		default:	return ch;
				break;
		
	}
}

void send_by_vlc (string message1, string message2, string message3, int cas){
	
	pid_t pid;
	int response;
	string vlc = "c:/Archiv~1/VideoLAN/VLC/vlc.exe";
	
	pid = fork();
	if (pid < 0) {std::cout << "ERROR: error doing pid!" << endl;}
	if (!pid) {
		if (cas == 1) {
			response = execlp(vlc.c_str(), vlc.c_str(), message1.c_str(), message2.c_str(), " :no-sout-rtp-sap", " :no-sout-standard-sap", " :sout-keep", (char *) 0);
		}
		if (cas == 2) {
		response = execl(vlc.c_str(), vlc.c_str(), message1.c_str(), ":dvb-bandwidth=8",  message2.c_str(), message3.c_str(), " :no-sout-rtp-sap", " :no-sout-standard-sap", " :sout-keep", (char *) 0);
		}
		if (cas == 3) {
			response = execlp(vlc.c_str(), vlc.c_str(), message1.c_str(), message2.c_str(), ":sout-all", ":ts-es-id-pid", (char *) 0);
		}
		if (response != 0) { printf("Error executing VLC: [errno %d : %s ]\n", errno, strerror(errno)); }
	}
}

void send_by_vlc_proxy(string rtp, string uport, string ip, string port) {

	ostringstream off, off2;
	string rtp_source;
	string rtp_destination;
	
	off << "rtp://@" << rtp << ":" << uport;
	rtp_source = off.str();
	
	off2 << ":sout=#rtp{dst=" << ip << ",port=" << port<< ",mux=ts}";
	rtp_destination = off2.str();
	
	send_by_vlc(rtp_source, rtp_destination, "", 1);
}

void send_by_vlc_proxy_udp(string udp, string uport, string ip, string port) {

	ostringstream off, off2;
	string udp_source;
	string udp_destination;
	
	off << "udp://@" << udp << ":" << uport;
	udp_source = off.str();
	
	off2 << ":sout=#udp{dst=" << ip << ":" << port<< "}";
	udp_destination = off2.str();
	
	send_by_vlc(udp_source, udp_destination, "", 3);
}

void send_by_vlc_emulator(int fx, string PID, string ip, string port) {
	
	fx = channel_to_fx(fx);
	
	ostringstream off, off2, off3;
	string source;
	string rtp_destination;
	string program;
	
	off << "dvb-t://frequency=" << fx;
	source = off.str();
	off3 << ":program=" << PID;
	program = off3.str();
	off2 << ":sout=#rtp{dst=" << ip << ",port=" << port<< ",mux=ts}";
	rtp_destination = off2.str();
	send_by_vlc(source, program, rtp_destination, 2);
}

int main (int argc, char* argv[]) {
	// USE:
	// senderbox.exe ip_rtp port_rtp ip_destination port_destination program
	// senderbox.exe frequency PID ip_destination port_destination program
	
	//program tag:
	// 1 -> VLC (proxy)
	// 2 -> VLC (emulator)
	// 3 -> VLC (proxy UDP)
	//more...
	
	int select;
	
	if (argc != 6) {
		std::cout << "ERROR: Correct use is senderbox.exe ip_rtp port_rtp ip_destination port_destination program" << endl <<"or for emulator senderbox.exe frequency PID ip_destination port_destination program" << endl;
		exit(0);
	}else{
		select = atoi(argv[5]);
		
		switch (select) {
			case 1:	send_by_vlc_proxy(argv[1], argv[2], argv[3], argv[4]);
				break;
			case 2:	send_by_vlc_emulator(atoi(argv[1]), argv[2], argv[3], argv[4]);
				break;
			case 3:	send_by_vlc_proxy_udp(argv[1], argv[2], argv[3], argv[4]);
				break;
			default:	send_by_vlc_proxy(argv[1], argv[2], argv[3], argv[4]);
				break;
		}
	}
}
