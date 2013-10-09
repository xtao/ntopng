/* 
   http://coding.debuntu.org/system/files/htmlget.c 
   
   GPL
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

int create_tcp_socket();
char *get_ip(char *host, char *buf, u_int buf_len);
char *build_get_query(char *host, char *page, char *buf, u_int buf_len);
void usage();


char* http_get(char *host, u_int port, char *page, char* rsp, u_int rsp_len) {
  struct sockaddr_in remote;
  int sock, tmpres;
  u_int out_len = 0;
  char *ip, _ip[64];
  char *get, _get[256];
  char buf[BUFSIZ+1];

  if((sock = create_tcp_socket()) < 0) return(NULL);
  if((ip = get_ip(host, _ip, sizeof(_ip))) == NULL) return(NULL);

  remote.sin_family = AF_INET;
  tmpres = inet_pton(AF_INET, ip, (void *)(&(remote.sin_addr.s_addr)));
  if( tmpres < 0)  {
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

  get = build_get_query(host, page, _get, sizeof(_get));
  // fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);
  
  //Send the query to the server
  u_int sent = 0;
  while(sent < strlen(get))
    { 
      tmpres = send(sock, get+sent, strlen(get)-sent, 0);

      if(tmpres == -1) {
	// perror("Can't send query");
	return(NULL);
      }
      sent += tmpres;
    }
  //now it is time to receive the page
  memset(buf, 0, sizeof(buf));
  int htmlstart = 0;
  char * htmlcontent;
  while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0) {
    if(htmlstart == 0)
      {
	/* Under certain conditions this will not work.
	 * If the \r\n\r\n part is splitted into two messages
	 * it will fail to detect the beginning of HTML content
	 */
	htmlcontent = strstr(buf, "\r\n\r\n");
	if(htmlcontent != NULL) {
	  htmlstart = 1;
	  htmlcontent += 4;
	}
      }else{
      htmlcontent = buf;
    }

    if(htmlstart) {
      if(rsp_len > out_len) {
	int n = snprintf(&rsp[out_len], rsp_len-out_len, "%s", htmlcontent);
	
	if(n == 0) 
	  out_len += n;
	else
	  break;
	// fprintf(stdout, "%s", htmlcontent);
      }
    }
 
    memset(buf, 0, tmpres);
  }
  if(tmpres < 0) {
    // perror("Error receiving data");
  }

  close(sock);
  
  return(rsp);
}

#if 0
int main(int argc, char **argv) {
  char rsp[256], *out, *host, *page;

  if(argc == 1) {
    usage();
    exit(2);
  }  
  host = argv[1];
  if(argc > 2) {
    page = argv[2];
  }

  if(!host || !page) usage();
 
  out = http_get(host, 80, page, rsp, sizeof(rsp));
  
  if(out)
    printf("%s\n", out);
  else
    printf("Error or empty response received\n");

  return(0);
}


void usage()
{
  fprintf(stderr, "USAGE: htmlget host [page]\n\
\thost: the website hostname. ex: coding.debuntu.org\n\
\tpage: the page to retrieve. ex: index.html, default: /\n");
}
#endif


int create_tcp_socket()
{
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    // perror("Can't create TCP socket");
    return(-1);
  }
  return sock;
}


char *get_ip(char *host, char *ip, u_int ip_len) {
  struct hostent *hent;

  memset(ip, 0, ip_len+1);
  if((hent = gethostbyname(host)) == NULL)
    {
      //herror("Can't get IP");
      return(NULL);
    }

  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, ip_len) == NULL)
    {
      // perror("Can't resolve host");
      return(NULL);
    }

  return ip;
}

char *build_get_query(char *host, char *page, char *buf, u_int buf_len)
{
  char *getpage = page;
  const char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n";

  if(getpage[0] == '/') {
    getpage = getpage + 1;
    //fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
  }
  // -5 is to consider the %s %s %s in tpl and the ending \0

  snprintf(buf, buf_len, tpl, getpage, host);

  return(buf);
}

