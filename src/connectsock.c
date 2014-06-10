/* connectsock.c - connectsock */

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */

extern int errno;
extern char *sys_errlist[];

int errexit(char *format, ...);

/*
u_short htons();
u_long inet_addr();
*/

/*------------------------------------------------------------------------
 * connectsock - allocate & connect a socket using TCP or UDP
 *------------------------------------------------------------------------
 */
int
connectsock(host, service, protocol)
char *host; /* name of host to which connection is desired */
char *service; /* service associated with the desired port */
char *protocol; /* name of protocol to use ("tcp" or "udp") */
{
  struct hostent *phe; /* pointer to host information entry */
  struct servent *pse; /* pointer to service information entry */
  struct protoent *ppe; /* pointer to protocol information entry */
  struct sockaddr_in sin; /* an Internet endpoint address */
  int s, type; /* socket descriptor and socket type */

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;

  /* Map service name to port number */
  pse = getservbyname(service, protocol);
  if (pse) {
    sin.sin_port = pse->s_port;
  } else {
    sin.sin_port = htons((u_short)atoi(service));
    if (sin.sin_port == 0) {
      errexit("can't get \"%s\" service entry\n", service);
    }
  }

  /* Map host name to IP address, allowing for dotted decimal */
  phe = gethostbyname(host);
  if (phe) {
    bcopy(phe->h_addr, (char *)&sin.sin_addr, phe->h_length);
  } else {
    sin.sin_addr.s_addr = inet_addr(host);
    if (sin.sin_addr.s_addr == INADDR_NONE) {
      errexit("can't get \"%s\" host entry\n", host);
    }
  }

  /* Map protocol name to protocol number */
  ppe = getprotobyname(protocol);
  if (ppe == 0) {
    errexit("can't get \"%s\" protocol entry\n", protocol);
  }

  /* Use protocol to choose a socket type */
  if (strcmp(protocol, "udp") == 0) {
    type = SOCK_DGRAM;
  } else {
    type = SOCK_STREAM;
  }

  /* Allocate a socket */
  s = socket(PF_INET, type, ppe->p_proto);
  if (s < 0) {
    errexit("can't create socket: %s\n", sys_errlist[errno]);
  }

  /* Connect the socket */
  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    errexit("can't connect to %s.%s: %s\n", host, service, sys_errlist[errno]);
  }
  return s;
}
