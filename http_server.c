/**
 *simple http webserver made for it215 system software project assignment
 *by Ayudh Das (201001131) and Kim Malar (201001126)
 *DA-IICT
 * 
 *
 * TODO
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define DEFAULT_PORT 8181
#define BUFSIZE 8096
#define BACKLOG 50

pthread_mutex_t mutex_pagefd = PTHREAD_MUTEX_INITIALIZER;
int listenfd, pagefd;
static struct sockaddr_in client_addr;

struct {
	char *ext;
	char *filetype;
} extensions [] = {  
	{"htm", "text/html" },  
	{"html","text/html" },
	{"txt", "text/plain"},
	{"c", "text/plain"  },
	{0,0} };


void *response( void* argv){
    char buffer[BUFSIZE+1], *mimetype;
    int temp, i, buflen, len;
    int connectionfd = (int)argv;
    temp = read(connectionfd,buffer,BUFSIZE); // http request captured;
    if(temp <= 0)
	printf("error fetching request\n");
    printf("request **%s**\n",buffer);
    
    if(temp > 0 && temp < BUFSIZE)	/* return code is valid chars */
	buffer[temp]=0;		/* terminate the buffer */
    else
	buffer[0]=0;
    /*
     * we are assuming that only simple get requests are supported by his server. 
     */
    
    
    for(i=4;i<BUFSIZE;i++) { /* null terminate after the second space to ignore extra stuff */
	if(buffer[i] == ' ') { /* string is "GET URL " +lots of other stuff */
		buffer[i] = 0;
		break;
	}
    }
    
    if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) /* convert no filename to index file */
	strcpy(buffer,"GET /index.html");
    
    buflen=strlen(buffer);
    mimetype = (char *)0;
    
    /*
     * mimetype check
     */
    for(i=0;extensions[i].ext != 0;i++) {
	len = strlen(extensions[i].ext);
	if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
		mimetype =extensions[i].filetype;
		break;
	}
    }
    

    if(mimetype == 0)
	printf("file extension type not supported");
    
    /*locking area*/
    pthread_mutex_lock(&mutex_pagefd);
    if(( pagefd = open(&buffer[5],O_RDONLY)) == -1) /* open the file for reading */
	printf("failed to open page");

    sprintf(buffer,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", mimetype);
    /*buffer = strcat("HTTP/1.1 200 OK\nContent-Type: ", mimetype);
    buffer = strcat(buffer,"\n");*/
    
    write(connectionfd,buffer,strlen(buffer));
    printf("*******\nresponse sent*****\n%s",buffer);

    while ((temp = read(pagefd, buffer, BUFSIZE)) > 0 ) {
	write(connectionfd,buffer,temp);
	printf("%s",buffer);
    }
    close(pagefd);
    printf("*******\nall response sent*****\n");
    pthread_mutex_unlock(&mutex_pagefd);
    pthread_mutex_destroy(&mutex_pagefd);
    close(connectionfd);
    sleep(1);
    printf("%d time to go\n",pthread_self());
    pthread_exit(0);
    
}

int main(int argc, char *argv[]){
    int socketfd,count,tempfd;
    size_t length;
    pthread_t tid[1000];
    static struct sockaddr_in serv_addr;
    
    /* this process is not intented to be made a  daemon so that it can log its progress in stdout*/
    if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
	printf("error in socket()\n");
	exit(1);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(DEFAULT_PORT);
    
    if((bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr))) < 0){
	printf("binding error\n");
	exit(1);
    }
    
    if(listen(listenfd,BACKLOG) < 0){
	printf("listening error\n");
	exit(1);
    }
    
    count = 0;
    while(1){
	length = sizeof(client_addr);
	if((socketfd = accept(listenfd,(struct sockaddr*)&client_addr,&length)) < 0){
	    printf("error in accept\n");
	    exit(1);
	}
	tempfd = socketfd;
	if(pthread_create(&tid[count],NULL,response,(void *)tempfd) < 0){
	    printf("error creating thread\n");
	    exit(1);
	}
	else{
	    count++;
	}
    }
}