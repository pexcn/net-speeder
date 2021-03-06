#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <libnet.h>

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 65535
#define SPECIAL_TTL 88
#define ETHERNET_H_LEN_COOKED 16
#define ETHERNET_H_LEN_NORMAL 14

static int ethernet_h_len = -1;

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
void print_usage(void);

/*
 * print help text
 */
void print_usage(void) {
	printf("Usage: %s [mode] <interface> \"filter\"\n", "netspeeder");
	printf("\n");
	printf("Options:\n");
	printf("    mode         Ethernet header length. (auto, normal, cooked)\n");
	printf("                 The default is \"auto\"\n");
	printf("    interface    Listen on <interface> for packets.\n");
	printf("    filter       Rules to filter packets.\n");
	printf("\n");
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	static int count = 1;
	struct libnet_ipv4_hdr *ip;

	libnet_t *libnet_handler = (libnet_t *)args;
	count++;

	ip = (struct libnet_ipv4_hdr*)(packet + ethernet_h_len);

	if(ip->ip_ttl != SPECIAL_TTL) {
		ip->ip_ttl = SPECIAL_TTL;
		ip->ip_sum = 0;
		if(ip->ip_p == IPPROTO_TCP) {
			struct libnet_tcp_hdr *tcp = (struct libnet_tcp_hdr *)((u_int8_t *)ip + ip->ip_hl * 4);
			tcp->th_sum = 0;
			libnet_do_checksum(libnet_handler, (u_int8_t *)ip, IPPROTO_TCP, LIBNET_TCP_H);
		} else if(ip->ip_p == IPPROTO_UDP) {
			struct libnet_udp_hdr *udp = (struct libnet_udp_hdr *)((u_int8_t *)ip + ip->ip_hl * 4);
			udp->uh_sum = 0;
			libnet_do_checksum(libnet_handler, (u_int8_t *)ip, IPPROTO_UDP, LIBNET_UDP_H);
		}
		int len_written = libnet_adv_write_raw_ipv4(libnet_handler, (u_int8_t *)ip, ntohs(ip->ip_len));
		if(len_written < 0) {
			printf("packet len: [%d] actual write: [%d]\n", ntohs(ip->ip_len), len_written);
			printf("err msg: [%s]\n", libnet_geterror(libnet_handler));
		}
	} else {
		// The packet netspeeder sent, nothing todo.
	}
	return;
}

libnet_t* start_libnet(char *dev) {
	char errbuf[LIBNET_ERRBUF_SIZE];
	libnet_t *libnet_handler = libnet_init(LIBNET_RAW4_ADV, dev, errbuf);

	if(NULL == libnet_handler) {
		printf("libnet_init: error %s\n", errbuf);
	}
	return libnet_handler;
}

int main(int argc, char **argv) {
	char *dev = NULL;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle;

	char *filter_rule = NULL;
	struct bpf_program fp;
	bpf_u_int32 net, mask;

	if (argc == 3) {
		dev = argv[1];
		filter_rule = argv[2];
		printf("Device: %s\n", dev);
		printf("Filter rule: %s\n", filter_rule);

		if (strncmp(argv[1], "venet", 5) == 0) {
			ethernet_h_len = ETHERNET_H_LEN_COOKED;
		} else {
			ethernet_h_len = ETHERNET_H_LEN_NORMAL;
		}
	} else if (argc == 4) {
		if (strcmp(argv[1], "auto") == 0) {
			if (strncmp(argv[1], "venet", 5) == 0) {
				ethernet_h_len = ETHERNET_H_LEN_COOKED;
			} else {
				ethernet_h_len = ETHERNET_H_LEN_NORMAL;
			}
		} else if (strcmp(argv[1], "normal") == 0) {
			ethernet_h_len = ETHERNET_H_LEN_NORMAL;
		} else if (strcmp(argv[1], "cooked") == 0) {
			ethernet_h_len = ETHERNET_H_LEN_COOKED;
		} else {
			print_usage();
			return -1;
		}

		dev = argv[2];
		filter_rule = argv[3];
		printf("Device: %s\n", dev);
		printf("Filter rule: %s\n", filter_rule);
	} else {
		print_usage();
		return -1;
	}

	printf("Ethernet header len: [%d] (14:normal, 16:cooked)\n", ethernet_h_len);

	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		printf("Couldn't get netmask for device %s: %s\n", dev, errbuf);
		net = 0;
		mask = 0;
	}

	printf("init pcap\n");
	handle = pcap_open_live(dev, SNAP_LEN, 1, 0, errbuf);
	if(handle == NULL) {
		printf("pcap_open_live dev: [%s] err: [%s]\n", dev, errbuf);
		printf("init pcap failed\n");
		return -1;
	}

	printf("init libnet\n");
	libnet_t *libnet_handler = start_libnet(dev);
	if(NULL == libnet_handler) {
		printf("init libnet failed\n");
		return -1;
	}

	if (pcap_compile(handle, &fp, filter_rule, 0, net) == -1) {
		printf("filter rule err: [%s][%s]\n", filter_rule, pcap_geterr(handle));
		return -1;
	}

	if (pcap_setfilter(handle, &fp) == -1) {
		printf("set filter failed: [%s][%s]\n", filter_rule, pcap_geterr(handle));
		return -1;
	}

	while(1) {
		pcap_loop(handle, 1, got_packet, (u_char *)libnet_handler);
	}

	/* cleanup */
	pcap_freecode(&fp);
	pcap_close(handle);
	libnet_destroy(libnet_handler);
	return 0;
}
