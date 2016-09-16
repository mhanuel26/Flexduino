#ifndef __NET_IN_H__
#define __NET_IN_H__

#if defined(WINC1500)
  struct hostent {
      char  *h_name;            /* official name of host */
      char  **h_aliases;        /* alias list */
      int   h_addrtype;         /* hostaddrtype(e.g.,AF_INET6) */
      int   h_length;           /* length of address */
      char  **h_addr_list;      /* list of addrs, null terminated */
  };
  /*1st addr, net byte order*/
  #define h_addr h_addr_list[0]

#endif

#define IPPROTO_IP		1
#define IPPROTO_TCP		6
#define IPPROTO_UDP		17

#define INET_ADDRSTRLEN         (16)
#define INET6_ADDRSTRLEN        (48)

#define SOMAXCONN	0		// just saw winc1500 fw always call listen with second param equal 0

#define INADDR_NONE             ((unsigned long int) 0xffffffff)

#endif
