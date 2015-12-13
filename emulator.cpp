#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <sstream>

#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>

#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include "libhdhomerun_20100213/hdhomerun.h"

#define HDHOMERUN_CONTROL_SEND_TIMEOUT 2500
#define HDHOMERUN_CONTROL_RECV_TIMEOUT 2500

using namespace std;

int Nchannel;
int Nprogram;

string Gip;
string Gport;
string Grtp;

struct RTP_data {
	string ip;
	string port;
	string rtp;
};
	

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

void print_msg (int type, size_t length, FILE *log) {
	
	switch (type) {
		case 0:	std::cout << ">>> Tuner ERROR target message send with length " << length << endl;
				fprintf(log, ">>> Tuner ERROR target message send with length %ud\n", length);
				break;
		case 1:	std::cout << ">>> Tuner NONE target message send with length " << length << endl;
				fprintf(log, ">>> Tuner NONE target message send with length %ud\n", length);
				break;
		case 2:	std::cout << ">>> Message /sys/version reply send by TCP with length " << length << endl;
				fprintf(log, ">>> Message /sys/version reply send by TCP with length %ud\n", length);
				break;
		case 3:	std::cout << ">>> Message /sys/model hdhomerun dvbt send by TCP with length " << length << endl;
				fprintf(log, ">>> Message /sys/model hdhomerun dvbt send by TCP with length %ud\n", length);
				break;
		case 4:	std::cout << ">>> Status message send by TCP with length " << length << endl;
				fprintf(log, ">>> Status message send by TCP with length %ud\n", length);
				break;
		case 5:	std::cout << ">>> Streaminfo message send by TCP with length " << length << endl;
				fprintf(log, ">>> Streaminfo message send by TCP with length %ud\n", length);
				break;
		case 6:	std::cout << ">>> Channel map message send by TCP with length " << length << endl;
				fprintf(log, ">>> Channel map message send by TCP with length %ud\n", length);
				break;
		case 7:	std::cout << ">>> Lockkey message send by TCP with length " << length << endl;
				fprintf(log, ">>> Lockkey message send by TCP with length %ud\n", length);
				break;
		case 8:	std::cout << ">>> Program message send by TCP with length " << length << endl;
				fprintf(log, ">>> Program message send by TCP with length %ud\n", length);
				break;
		case 9:	std::cout << ">>> Change channel message send by TCP with length " << length << endl;
				fprintf(log, ">>> Change channel message send by TCP with length %ud\n", length);
				break;
		case 10:	std::cout << ">>> Tuner RTP target message send with length " << length << endl;
				fprintf(log, ">>> Tuner RTP target message send with length %ud\n", length);
				break;
		case 11:	std::cout << ">>> Tuner FILTER message send with length " << length << endl;
				fprintf(log, ">>> Tuner FILTER message send with length %ud\n", length);
				break;
		case 12:	std::cout << ">>> Tuner DEBUG message send with length " << length << endl;
				fprintf(log, ">>> Tuner DEBUG message send with length %ud\n", length);
				break;
		case 13:	std::cout << ">>> IR target message send with length " << length << endl;
				fprintf(log, ">>> IR target message send with length %ud\n", length);
				break;
		case 14:	std::cout << ">>> Lineup Location message send with length " << length << endl;
				fprintf(log, ">>> Lineup Location message send with length %ud\n", length);
				break;
		case 15:	std::cout << ">>> sys FEATURES message send with length " << length << endl;
				fprintf(log, ">>> sys FEATURES message send with length %ud\n", length);
				break;
		case 16:	std::cout << ">>> sys COPYRIGHT message send with length " << length << endl;
				fprintf(log, ">>> sys COPYRIGHT message send with length %ud\n", length);
				break;
		case 17:	std::cout << ">>> sys DEBUG message send with length " << length << endl;
				fprintf(log, ">>> sys DEBUG message send with length %ud\n", length);
				break;
		default:	std::cout << ">>> WARNING: Message not found " << endl;
				fprintf(log, ">>> WARNING: Message not found \n");
		}
}

int check_message (char *text) {
	
	if ((int)text[1]==4 && (int)text[4]==3 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && (int)text[13]=='/' && text[14]=='t' && text[15]=='a' && text[16]=='r' && text[17]=='g' && text[18]=='e' && text[19]=='t' ) {
		// /tuner<n>/target
		return 1;
	}
	
	if ((int)text[1]==4 && (int)text[6]==47 && (int)text[7]==115 && (int)text[8]==121 && (int)text[9]==115 && (int)text[10]==47 && (int)text[11]==118 && (int)text[12]==101 && (int)text[13]==114 && (int)text[14]==115 && (int)text[15]==105 && (int)text[16]==111 && (int)text[17]==110) {
		// /sys/version
		return 2;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='s' && text[8]=='y' && text[9]=='s' && text[10]=='/' && text[11]=='m' && text[12]=='o' && text[13]=='d' && text[14]=='e' && text[15]=='l') {
		// /sys/model
		return 3;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='s' && text[15]=='t' && text[16]=='a' && text[17]=='t' && text[18]=='u' && text[19]=='s') {
		// /tuner<n>/status
		return 4;
	}
	
	if ((int)text[1]==4 && (int)text[4]==3 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[12]=='0' && text[13]=='/' && text[14]=='s' && text[15]=='t' && text[16]=='r' && text[17]=='e' && (int)text[18]=='a' && text[19]=='m' && text[20]=='i' && text[21]=='n' && text[22]=='f' && (int)text[23]=='o' ) {
		// /tuner<n>/streaminfo
		return 5;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='c' && text[15]=='h' && text[16]=='a' && text[17]=='n' && text[18]=='n' && text[19]=='e' && text[20]=='l' && text[21]=='m' && text[22]=='a' && text[23]=='p') {
		// /tuner<n>/channelmap <channel map>
		return 6;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='l' && text[15]=='o' && text[16]=='c' && text[17]=='k' && text[18]=='k' && text[19]=='e' && text[20]=='y') {
		// /tuner<n>/lockkey
		return 7;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='p' && text[15]=='r' && text[16]=='o' && text[17]=='g' && text[18]=='r' && text[19]=='a' && text[20]=='m') {
		// /tuner<n>/program <program number>
		return 8;
	}
	
	//~ if ((int)text[1]==4 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='c' && text[15]=='h' && text[16]=='a' && text[17]=='n' && text[18]=='n' && text[19]=='e' && text[20]=='l' && text[24]=='a' && text[25]=='u' && text[26]=='t' && text[27]=='o'  && text[28]==':') {
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='c' && text[15]=='h' && text[16]=='a' && text[17]=='n' && text[18]=='n' && text[19]=='e' && text[20]=='l') {
		// /tuner<n>/channel <modulation>:<freq|ch>
		return 9;
	}
	
	// return 10; // /tuner<n>/target <ip>:<port> //No implementation, RTP message
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='f' && text[15]=='i' && text[16]=='l' && text[17]=='t' && text[18]=='e' && text[19]=='r') {
		// /tuner<n>/filter 0x<nnnn>-0x<nnnn>
		return 11;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='t' && text[8]=='u' && text[9]=='n' && text[10]=='e' && text[11]=='r' && text[13]=='/' && text[14]=='d' && text[15]=='e' && text[16]=='b' && text[17]=='u' && text[18]=='g') {
		// /tuner<n>/debug
		return 12;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='i' && text[8]=='r' && text[9]=='/' && text[10]=='t' && text[11]=='a' && text[12]=='r' && text[13]=='g' && text[14]=='e' && text[15]=='t') {
		// /ir/target <ip>:<port>
		return 13;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='l' && text[8]=='i' && text[9]=='n' && text[10]=='e' && text[11]=='u' && text[12]=='p' && text[13]=='/' && text[14]=='l' && text[15]=='o' && text[16]=='c' && text[17]=='a' && text[18]=='t' && text[19]=='i' && text[20]=='o' && text[21]=='n') {
		// /lineup/location <countrycode>:<postcode>
		// /lineup/location disabled
		return 14;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='s' && text[8]=='y' && text[9]=='s' && text[10]=='/' && text[11]=='f' && text[12]=='e' && text[13]=='a' && text[14]=='t' && text[15]=='u' && text[16]=='r' && text[17]=='e' && text[18]=='s') {
		// /sys/features
		return 15;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='s' && text[8]=='y' && text[9]=='s' && text[10]=='/' && text[11]=='c' && text[12]=='o' && text[13]=='p' && text[14]=='y' && text[15]=='r' && text[16]=='i' && text[17]=='g' && text[18]=='h' && text[19]=='t') {
		// /sys/copyright
		return 16;
	}
	
	if ((int)text[1]==4 && text[6]=='/' && text[7]=='s' && text[8]=='y' && text[9]=='s' && text[10]=='/' && text[11]=='d' && text[12]=='e' && text[13]=='b' && text[14]=='u' && text[15]=='g') {
		// /sys/debug
		return 17;
	}
	
	return -1;
}

void know_data_from_rtp_addr (char* text, size_t length) {
	
	int i, valor;
	ostringstream aux, aux2, aux3;
	struct RTP_data data;
	
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
		if (i>length) {
			std::cout << "ERROR: NO IP in message!";
			exit(0);
		}
	}
	
	Gip = aux.str();
	i++;
	
	while (text[i] =='0' || text[i] =='1' || text[i] =='2' || text[i] =='3' || text[i] =='4' || text[i] =='5' || text[i] =='6' || text[i] =='7' || text[i] =='8' || text[i] =='9') {
		aux2 << text[i];
		i++;
	}
	
	Gport = aux2.str();

	aux3 << "rtp://" << Gip.c_str() << ":" << Gport.c_str();
	Grtp = aux3.str();
}

void send_by_senderbox(string ip, string port) {
	
	pid_t pid;
	ostringstream off, off2;
	string channel, program;
	
	off << Nchannel;
	channel = off.str();
	
	off2 << Nprogram;
	program = off2.str();
	
	pid = fork();
	if (pid < 0) {std::cout << "error doing pid!" << endl;}
	if (!pid) {
		int response = execl("./senderbox.exe", "./senderbox.exe", channel.c_str(), program.c_str(), ip.c_str(), port.c_str(), "2", (char *) 0);
		if (response != 0) { printf("Error executing senderbox: [errno %d : %s ]\n", errno, strerror(errno)); }
	}
}

char* give_a_ch_prog(char *input, int a, int b, int c, int d, int e) {
	
	bool end = false;
	char number[7];
	char poszero;
	while (end == false) {
	
		poszero = input[a];
		number[0] = poszero;
		
		if (input[b] >= '0' && input[b] <= '9') {
			number[1] = input[b];
		}else{
			number[1] = '\0';
			end = true;
		}
		
		if (input[c] >= '0' && input[c] <= '9') {
			number[2] = input[c];
		}else{
			number[2] = '\0';
			end = true;
		}
		
		if (input[d] >= '0' && input[d] <= '9') {
			number[3] = input[d];
		}else{
			number[3] = '\0';
			end = true;
		}
		
		if (input[e] >= '0' && input[e] <= '9') {
			number[4] = input[e];
		}else{
			number[4] = '\0';
			end = true;
		}
		
		number[5] = '\0';
		end = true;
			
	}
	
	return number;
}

void create_message_TCP(struct hdhomerun_pkt_t *mis, int cas, string input) {
	
	ostringstream auxi;
	
	hdhomerun_pkt_reset(mis);
	
	char *aux = "aaa";
	char *aux2 = "aaa";
	strcpy(aux, input.c_str());
	
	const char *channelmap = "/tuner0/channelmap";
	const char *debug = "/tuner0/debug";
	const char *filter = "/tuner0/filter";
	const char *lockkey = "/tuner0/lockkey";
	const char *ir = "/ir/target";
	const char *lineup = "/lineup/location";
	const char *features = "/sys/features";
	const char *copyright = "/sys/copyright";
	const char *sdebug = "/sys/debug";
	const char *model = "/sys/model";
	const char *version = "/sys/version";
	const char *program = "/tuner0/program";
	const char *status = "/tuner0/status";
	const char *streaminfo = "/tuner0/streaminfo";
	const char *target = "/tuner0/target";
	
	char *autoYY = "aaa";
	string autoY;
	
	if (cas == 9) {
		auxi << "auto:" << input;
		autoY = auxi.str();
		//~ Nchannel = atoi(input.c_str());
		strcpy(autoYY, autoY.c_str());
		
		//Cleaning auxiliar value
		auxi.clear();
		auxi.str("");
	}
	
	const char *data = "20100213";
	const char *debugmsg = "tun: ch=none lock=none ss=0 snq=0 seq=0 dbg=0\ndev: resync=0 overflow=0\n ts: bps=0 ut=0 te=0 miss=0 crc=0\nflt: bps=0\nnet: pps=0 err=0 stop=0\n";
	const char *sdebugmsg = "mem: nbm=106/103 qwn=4 npf=611 dmf=236\nloop: pkt=0\nt0:pt=9 cal=16589-12782 bsw=155/435\nt1:pt=9 cal=16181-12777 bsw=155/435\neth: link=100f\n";
	const char *europe = "eu-bcast";
	//~ const char *copyinfo = "Copyright © 2005-2010 Silicondust USA Inc. <www.silicondust.com>. All rights reserved.";
	char *copyinfo = "Copyright © 2005-2010 Silicondust USA Inc. <www.silicondust.com>. All rights reserved. Emulator made by Adri� Ameijide and PFC supervised by Dani Soto";
	const char *featmsg = "channelmap: eu-bcast eu-cable au-bcast au-cable tw-bcast tw-cable\nmodulation: t8qam64 t8qam16 t8qpsk t7qam64 t7qam16 t7qpsk t6qam64 t6qam16 t6qpsk a8qam256-* a8qam128-* a8qam64-* a7qam256-* a7qam128-* a7qam64-* a6qam256-* a6qam128-* a6qam64-*\nauto-modulation: auto auto8t auto7t auto6t auto8c auto7c auto6c\n";
	const char *filt = "0x000-0x1FFF";
	const char *location = "ES:08018";
	const char *malformed = "ERROR: malformed getset request";
	const char *miss = "ERROR: unknown getset variable";
	const char *none = "none";
	char *rtp = "aaa";
	if (cas == 10) {
		strcpy(rtp, input.c_str());
	}
	
	string StatCh;
	
	ostringstream txt, file_read;
	string text, read_file;
	
	txt << Nchannel << ".txt";
	text = txt.str();
	
	char c;
	int it=0;
	FILE *f = fopen (text.c_str(), "r");
	if (f!=NULL)
	{
		auxi << "ch=auto:" << Nchannel << " lock=t8qam64 ss=90 snq=80 seq=100 0 bps=19908448 p ps=0";
		while ((c=getc(f)) !=EOF) {
			file_read << c;
			it++;
		}
		read_file = file_read.str();
		fclose(f);
	}else{
		auxi << "ch=auto:" << Nchannel << " lock=none ss=10 snq=0 seq=0 bps=0 pps=0";
		read_file = "none";
	}
	StatCh = auxi.str();
	const char *status_ch = StatCh.c_str();
	
	const char *channelF = "/tuner0/channel";

	const char *stinfo = read_file.c_str();
	
	//Cleaning auxiliar values
	file_read.clear();
	file_read.str("");
	auxi.clear();
	auxi.str("");

	//~ const char *stinfo = "100: 0 Teledeporte\n110: 0 Canal Ingenieria (control)\n260: 0 VEO7\n261: 0 AXN (encrypted) \n262: 0 Tienda en VEO\n269: 0 GUIDE PLUS+ (control)\n271: 0 RADIO MARCA\n272: 0 esRadio\n273: 0 Vaughan Radio\n300: 0 Intereconom￿a\n303: 0 Radio Interecon\n65534: 0 (control)\ntsid=0xA";
	const char *value = "hdhomerun_dvbt";
	
	int autoYY_len = (int)strlen(autoYY) + 1;
	int channel_len = (int)strlen(channelF) + 1;
	int channelmap_len = (int)strlen(channelmap) + 1;
	int debug_len = (int)strlen(debug) + 1;
	int copyright_len = (int)strlen(copyright) + 1;
	int features_len = (int)strlen(features) + 1;
	int filter_len = (int)strlen(filter) + 1;
	int ir_len = (int)strlen(ir) + 1;
	int lineup_len = (int)strlen(lineup) + 1;
	int lockkey_len = (int)strlen(lockkey) + 1;
	int sdebug_len = (int)strlen(sdebug) + 1;
	int model_len = (int)strlen(model) + 1;
	int program_len = (int)strlen(program) + 1;
	int status_len = (int)strlen(status) + 1;
	int streaminfo_len = (int)strlen(streaminfo) + 1;
	int target_len = (int)strlen(target) + 1;
	int aux_len = (int)strlen(aux) + 1;
	int data_len = (int)strlen(data) + 1;
	int debugmsg_len = (int)strlen(debugmsg) + 1;
	int sdebugmsg_len = (int)strlen(sdebugmsg) + 1;
	int europe_len = (int)strlen(europe) + 1;
	int copyinfo_len = (int)strlen(copyinfo) + 1;
	int featmsg_len = (int)strlen(featmsg) + 1;
	int filt_len = (int)strlen(filt) + 1;
	int location_len = (int)strlen(location) + 1;
	int malformed_len = (int)strlen(malformed) + 1;
	int miss_len = (int)strlen(miss) + 1;
	int none_len = (int)strlen(none) + 1;
	//~ int Nprogram_len = (int)strlen(Nprogram) + 1;
	int Nprogram_len = 2 + 1;
	int rtp_len = (int)strlen(rtp) + 1;
	int status_ch_len = (int)strlen(status_ch) + 1;
	int stinfo_len = (int)strlen(stinfo) + 1;
	int value_len = (int)strlen(value) + 1;
	int version_len = (int)strlen(version) + 1;
	
	switch (cas) {
		case 0: // ERROR: unknow getset variable (no more HDHR)
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_ERROR_MESSAGE);
			hdhomerun_pkt_write_var_length(mis, miss_len);
			hdhomerun_pkt_write_mem(mis, (void *)miss, miss_len);
			
			break;
		
		case 1: // /tuner0/target NONE Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, target_len);
			hdhomerun_pkt_write_mem(mis, (void *)target, target_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, none_len);
			hdhomerun_pkt_write_mem(mis, (void *)none, none_len);
		
			break;
		
		case 2: // /sys/version Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, version_len);
			hdhomerun_pkt_write_mem(mis, (void *)version, version_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, data_len);
			hdhomerun_pkt_write_mem(mis, (void *)data, data_len);
		
			break;
		
		case 3: // /sys/model Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, model_len);
			hdhomerun_pkt_write_mem(mis, (void *)model, model_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, value_len);
			hdhomerun_pkt_write_mem(mis, (void *)value, value_len);
		
			break;
		
		case 4: // /sys/status Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, status_len);
			hdhomerun_pkt_write_mem(mis, (void *)status, status_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, status_ch_len);
			hdhomerun_pkt_write_mem(mis, (void *)status_ch, status_ch_len);
		
			break;
		
		case 5: // /tuner0/streaminfo Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, streaminfo_len);
			hdhomerun_pkt_write_mem(mis, (void *)streaminfo, streaminfo_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, stinfo_len);
			hdhomerun_pkt_write_mem(mis, (void *)stinfo, stinfo_len);
		
			break;
		
		case 6: // /tuner0/channelmap Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, channelmap_len);
			hdhomerun_pkt_write_mem(mis, (void *)channelmap, channelmap_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, europe_len);
			hdhomerun_pkt_write_mem(mis, (void *)europe, europe_len);
		
			break;
		
		case 7: // /tuner0/lockkey Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, lockkey_len);
			hdhomerun_pkt_write_mem(mis, (void *)lockkey, lockkey_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, none_len);
			hdhomerun_pkt_write_mem(mis, (void *)none, none_len);
		
			break;
		
		case 8: // /tuner0/program Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, program_len);
			hdhomerun_pkt_write_mem(mis, (void *)program, program_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, Nprogram_len);
			hdhomerun_pkt_write_mem(mis, (void *)aux, Nprogram_len);
		
			break;
		
		case 9: // /tuner0/channel auto:YY Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, channel_len);
			hdhomerun_pkt_write_mem(mis, (void *)channelF, channel_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, autoYY_len);
			hdhomerun_pkt_write_mem(mis, (void *)autoYY, autoYY_len);
		
			break;
		
		case 10: // /tuner0/target RTP Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, target_len);
			hdhomerun_pkt_write_mem(mis, (void *)target, target_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, rtp_len);
			hdhomerun_pkt_write_mem(mis, (void *)rtp, rtp_len);
		
			break;
		
		case 11: // /tuner0/filter 0xNNNN-0xNNNN Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, filter_len);
			hdhomerun_pkt_write_mem(mis, (void *)filter, filter_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, filt_len);
			hdhomerun_pkt_write_mem(mis, (void *)filt, filt_len);
		
			break;
		
		case 12: // /tuner0/debug Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, debug_len);
			hdhomerun_pkt_write_mem(mis, (void *)debug, debug_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, debugmsg_len);
			hdhomerun_pkt_write_mem(mis, (void *)debugmsg, debugmsg_len);
		
			break;
		
		case 13: // /ir/target Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, ir_len);
			hdhomerun_pkt_write_mem(mis, (void *)ir, ir_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, none_len);
			hdhomerun_pkt_write_mem(mis, (void *)none, none_len);
		
			break;
		
		case 14: // /lineup/location Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, lineup_len);
			hdhomerun_pkt_write_mem(mis, (void *)lineup, lineup_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, location_len);
			hdhomerun_pkt_write_mem(mis, (void *)location, location_len);
		
			break;
		
		case 15: // /sys/features Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, features_len);
			hdhomerun_pkt_write_mem(mis, (void *)features, features_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, featmsg_len);
			hdhomerun_pkt_write_mem(mis, (void *)featmsg, featmsg_len);
		
			break;
		
		case 16: // /sys/copyright Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, copyright_len);
			hdhomerun_pkt_write_mem(mis, (void *)copyright, copyright_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, copyinfo_len);
			hdhomerun_pkt_write_mem(mis, (void *)copyinfo, copyinfo_len);
		
			break;
		
		case 17: // /sys/debug Reply
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_NAME);
			hdhomerun_pkt_write_var_length(mis, sdebug_len);
			hdhomerun_pkt_write_mem(mis, (void *)sdebug, sdebug_len);

			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_GETSET_VALUE);
			hdhomerun_pkt_write_var_length(mis, sdebugmsg_len);
			hdhomerun_pkt_write_mem(mis, (void *)sdebugmsg, sdebugmsg_len);
		
			break;
		
		default: // ERROR: malformed package
			hdhomerun_pkt_write_u8(mis, HDHOMERUN_TAG_ERROR_MESSAGE);
			hdhomerun_pkt_write_var_length(mis, malformed_len);
			hdhomerun_pkt_write_mem(mis, (void *)malformed, malformed_len);
			
			break;
	}

	hdhomerun_pkt_seal_frame(mis, HDHOMERUN_TYPE_GETSET_RPY);
}

///////////////////////////////////////////
///////////////////MAIN//////////////////
//////////////////////////////////////////

int main (int argc, char* argv[]) {
	
	hdhomerun_sock_t sockTCP;
	struct sockaddr_in Adreca, Resposta;
	char text[40];
	
	Nchannel = 2;
	Nprogram = 0;
	
	char *buff = "aaa";
	string buffer, ip, port;
	
	FILE *log = fopen ("log_emulator.txt", "w");
	
	struct hdhomerun_pkt_t *mis = hdhomerun_pkt_create();
	hdhomerun_pkt_reset(mis);
	
	int i, cas=0, verbose=1;
	
	uint32_t remote_addr;
	uint16_t remote_port;
	
	socklen_t mres = sizeof(struct sockaddr_in);
	
	size_t lengthResp = 3000;
	size_t length;
	
	if (argc == 2) {
		verbose = atoi(argv[1]);
		std::cout << "Verbose ";
		fprintf(log, "Verbose ");
		
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
			default:	std::cout << "Default: All messages" << endl;
					fprintf(log, "Default: All messages\n");
					break;
		}
	}else{
		std::cout << "Verbose not selected: All messages will appear" << endl;
		fprintf(log, "Verbose not selected: All messages will appear\n");
	}
	
	std::cout << endl;
	fprintf(log, "\n");
	
	//Opening socket
	sockTCP = hdhomerun_sock_create_tcp();
	if (sockTCP == -1) { //Error check
		printf("ERROR: Error creating TCP socket [errno %d : %s ]\n", errno, strerror(errno));
		fprintf(log, "ERROR: Error creating TCP socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	if (verbose <= 1) {std::cout << "Created socket TCP" << endl; fprintf(log, "Created socket TCP\n");}
	
	//Port association (bind) to socket and error check
	if (hdhomerun_sock_bind(sockTCP, INADDR_ANY, HDHOMERUN_CONTROL_TCP_PORT) == -1) {
		printf("ERROR: Error binding client TCP socket [errno %d : %s ]\n", errno, strerror(errno));
		fprintf(log, "ERROR: Error binding client TCP socket [errno %d : %s ]\n", errno, strerror(errno));
	}
	
	if (verbose <= 1) {std::cout << "Binded socket TCP" << endl;fprintf(log, "Binded socket TCP\n");}
	
	//Start listening, with 5 in queue
	listen(sockTCP, 5);
	
	if (verbose <= 1) {std::cout << "Listening socket!" << endl << "-----" << endl; fprintf(log, "Listening socket!\n-----\n");}
	
	struct RTP_data data;
	
	std::cout << "Emulator active!" << endl;
	fprintf (log, "Emulator active!\n");
	
	while(1) {
		
		int acc = accept(sockTCP, (struct sockaddr *) &Resposta, &mres );
		if ( acc > 0 ) {
			if (verbose <=3) { printf("ACCEPTED TCP connection\n"); fprintf(log, ">>> ACCEPTED TCP connection\n"); }
			
			while(1) {
				for(i=0; i < 40; i++) { text[i]==-1; } // Reset text
				lengthResp = 3000;
				
				if (hdhomerun_sock_recv(acc, &text, &lengthResp, HDHOMERUN_CONTROL_RECV_TIMEOUT) > 0 ) {
					if (verbose <=3) { 
						std::cout << "<<< TCP message recived! from IP " << inet_ntoa(Resposta.sin_addr) << " Port: " << ntohs(Resposta.sin_port) << endl;
						fprintf(log, "<<< TCP message recived! from IP %s Port: %d\n", inet_ntoa(Resposta.sin_addr), ntohs(Resposta.sin_port));
						if (verbose <=2) { print_text(text, lengthResp, log); }
					}
					
					cas = check_message(text);
					
					switch(cas) {
						case 1:	if ((int)text[12]!=48) { // X != 0
								std::cout << "1" << endl;
									cas = 0;
									create_message_TCP(mis, cas, "");
								}else{
									if (text[23]=='r') {  // /tunerX/target rtp://... , where X can be anything
										know_data_from_rtp_addr (text, lengthResp);
										cas = 10;
										send_by_senderbox(Gip, Gport); //Start sender program
										create_message_TCP(mis, cas, Grtp);
									}else{
										create_message_TCP(mis, cas, "");
									}
								}
								
								break;
						
						case 8:	//Program
								buff = give_a_ch_prog(text, 24, 25, 26, 27, 28);
								buffer = buff;
								Nprogram = atoi(buffer.c_str());
								create_message_TCP(mis, cas, buffer);
								break;
						
						case 9:	//Channel
								buff = give_a_ch_prog(text, 29, 30, '\0', '\0', '\0');
								buffer = buff;
								Nchannel = atoi(buffer.c_str());
								create_message_TCP(mis, cas, buffer);
								break;
						
						default:	create_message_TCP(mis, cas, "");
								break;
					}
					
					length = mis->end - mis->start;
					if ( hdhomerun_sock_send(acc, mis->start, length, HDHOMERUN_CONTROL_SEND_TIMEOUT) ) {
						if (verbose <=3) { 
							print_msg(cas, length, log);
						}
						
						if (cas==7) {
							if (verbose <=3) { std::cout << "LOCKKEY BREAK;" << endl; fprintf(log, "LOCKKEY BREAK;\n"); }
							break;
						}
					}
				}else{
					if (verbose <=3) { printf("BREAK;\n"); fprintf(log, "BREAK;\n"); } //If we didn't recive, maybe need connect again
					break;
				}
			}
		}
	}
}
