/* 
   http://coding.debuntu.org/system/files/htmlget.c 
   
   GPL

   Strongly debugged by Luca Deri <deri@ntop.org>
*/


#ifdef WIN32
#include "ntop_includes.h"
#else
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#endif

int create_tcp_socket();
char *get_ip(char *host, char *buf, u_int buf_len);
char *build_get_query(char *host, char *page, char *buf, u_int buf_len);
void usage();


char* http_get(char *host, u_int port, char *page, char* rsp, u_int rsp_len) {
  struct sockaddr_in remote;
  int sock, tmpres;
  u_int out_len = 0;
  char *ip, _ip[64];
  char get_buf[256];
  char buf[1024];
  int htmlstart = 0, debug = 0;
  char *htmlcontent;

  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    // perror("Can't create TCP socket");
    return(NULL);
  }

  snprintf(get_buf, sizeof(get_buf), "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", (page[0] == '/') ? &page[1] : page, host);

  if(debug) fprintf(stderr, "Query is:\n---\n%s---\n", get_buf);

  if((ip = get_ip(host, _ip, sizeof(_ip))) == NULL) return(NULL);
  remote.sin_family = AF_INET;
  tmpres = inet_pton(AF_INET, ip, (void *)(&(remote.sin_addr.s_addr)));
  if(tmpres < 0)  {
    // perror("Can't set remote.sin_addr.s_addr");
    return(NULL);
  } else if(tmpres == 0) {
    // fprintf(stderr, "%s is not a valid IP address\n", ip);
    return(NULL);
  }

  remote.sin_port = htons(port);

  if(connect(sock, (struct sockaddr *)&remote, sizeof(struct sockaddr)) < 0) {
    // perror("Could not connect");
    return(NULL);
  }
  
  // Send the query to the server
  u_int sent = 0;
  while(sent < strlen(get_buf)) { 
    tmpres = send(sock, get_buf+sent, strlen(get_buf)-sent, 0);

    if(tmpres == -1) {
      // perror("Can't send query");
      return(NULL);
    }
    sent += tmpres;
  }
  //now it is time to receive the page
  memset(buf, 0, sizeof(buf));

  while((tmpres = recv(sock, buf, sizeof(buf)-1, 0)) > 0) {
    buf[tmpres] = '\0';
    
    if(debug) fprintf(stderr, "Received %d bytes\n", tmpres);

    if(htmlstart == 0) {
      /* Under certain conditions this will not work.
       * If the \r\n\r\n part is splitted into two messages
       * it will fail to detect the beginning of HTML content
       */
      htmlcontent = strstr(buf, "\r\n\r\n");

      if(htmlcontent != NULL) {
	htmlstart = 1;
	htmlcontent += 4;
      }
    } else
      htmlcontent = buf;    

    if(htmlstart) {
      if(rsp_len > out_len) {
	int n = snprintf(&rsp[out_len], rsp_len-out_len, "%s", htmlcontent);
	
	if(n > 0) out_len += n;
	// fprintf(stdout, "%s", htmlcontent);
      }
    }
  }

#ifdef WIN32
  closesocket(sock);
#else
  close(sock);
#endif

  if(debug) fprintf(stderr, "%s\n", rsp);
  
  return(rsp);
}

char *get_ip(char *host, char *ip, u_int ip_len) {
  struct hostent *hent;

  memset(ip, 0, ip_len);
  if((hent = gethostbyname(host)) == NULL) {
    //herror("Can't get IP");
    return(NULL);
  }

  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, ip_len) == NULL) {
    // perror("Can't resolve host");
    return(NULL);
  }

  return ip;
}
