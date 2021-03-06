/* passivesock.c - passivesock */

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

extern int errno;
extern char *sys_errlist[];

int errexit(char *format, ...);

/*
u_short htons(), ntohs();
*/

u_short portbase = 0; /* port base, for non-root servers */

/*------------------------------------------------------------------------
 * passivesock - allocate & bind a server socket using TCP or UDP
 *------------------------------------------------------------------------
 */
int
passivesock(service, protocol, qlen)
char *service; /* service associated with the desired port */
char *protocol; /* name of protocol to use ("tcp" or "udp") */
int qlen; /* maximum length of the server request queue */
{
  struct servent *pse; /* pointer to service information entry */
  struct protoent *ppe; /* pointer to protocol information entry */
  struct sockaddr_in sin; /* an Internet endpoint address */
  int s, type; /* socket descriptor and socket type */

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;

  /* Map service name to port number */
  pse = getservbyname(service, protocol);
  if (pse) {
    sin.sin_port = htons(ntohs((u_short)pse->s_port) + portbase);
  } else {
    sin.sin_port = htons((u_short)atoi(service));
    if (sin.sin_port == 0) {
      errexit("can't get \"%s\" service entry\n", service);
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

  /* Bind the socket */
  if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    errexit("can't bind to %s port: %s\n", service, sys_errlist[errno]);
  }
  if (type == SOCK_STREAM && listen(s, qlen) < 0) {
    errexit("can't listen on %s port: %s\n", service, sys_errlist[errno]);
  }
  return s;
}

