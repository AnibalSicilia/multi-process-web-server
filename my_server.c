#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

struct {
	char *extension;
	char *type_f;
} supported_f[] = {{"html","text/html"},{"htm","text/html"},{"jpg","image/jpeg"}, {"jpeg", "image/jpeg"},
				   {"gif", "image/gif"}, {"css", "text/css"}, {"js", "text/javascript"}, 
				   {"png", "image/png"}, {"flv", "video/x-flv"}, {"mp4", "video/mp4"}, {0,0}};

void syserr(char *msg) {
	perror(msg);
	exit(-1);
}

void server_messages(int m_code, int newsockfd, char *messa, long cont_size) {
	int c_fbd, c_nf;
	char sm_buffer[1024];

	switch(m_code)
	{	
		case(200):
			sprintf(sm_buffer, "HTTP/1.0 200 OK\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);	
			sprintf(sm_buffer, "Content-Type: %s\r\n", messa);
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);             
			sprintf(sm_buffer, "Content-Length: %lu\r\n", cont_size);
            send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
            sprintf(sm_buffer, "\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
			break;
		case 404:
			sprintf(sm_buffer, "HTTP/1.0 404 NOT FOUND\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
			sprintf(sm_buffer,  "Connection: Close\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
			sprintf(sm_buffer, "\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
			sprintf(sm_buffer, "<html><head><title>Not Found</title></head>\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
			sprintf(sm_buffer, "<body><h1>404 Not Found</h1><p>The Server could not fulfill\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
			sprintf(sm_buffer, "your request because the resource specified\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
			sprintf(sm_buffer, "is unavailable or nonexistent.\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
			sprintf(sm_buffer, "</p></body></html>\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
            break;
		case 403:
			sprintf(sm_buffer, "HTTP/1.0 403 FORBIDDEN\r\n");
            send(newsockfd, sm_buffer, strlen(sm_buffer), 0); 
			sprintf(sm_buffer,  "Connection: Close\r\n");
			send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
            sprintf(sm_buffer, "\r\n");
            send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
            sprintf(sm_buffer, "<html><title>Forbidden</title>\r\n");
            send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
            sprintf(sm_buffer, "<body><h1>403 Forbidden</h1><p>%s\r\n", messa);
            send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
            sprintf(sm_buffer, "</p></body></html>\r\n");
            send(newsockfd, sm_buffer, strlen(sm_buffer), 0);
			break;
	}   	
}


void process_request(int newsockfd) {
	FILE *opened_file = NULL;
	int string_size, fd_reader, i, ext_size;
	int DATA_SIZE = 8192;
	long opened_file_size;	// to use with ftell
	int  read_file_size;
	char pr_buffer[DATA_SIZE];
	char *ext_type, *substring, *filename, *m;
	size_t handle_f;
	
        fd_reader = recv(newsockfd, pr_buffer, sizeof(pr_buffer), 0);
        if(fd_reader<0) {
        	syserr("can't receive from client\n");
        }
	else {
		pr_buffer[fd_reader] = '\0';
	}

	printf("SERVER GOT MESSAGE: %s\n", pr_buffer);
	// Handle if requested page is HTTP or not
	substring = strstr(pr_buffer, " HTTP/");	// locate substring
	if(substring == NULL) {
		syserr("Not HTTP!");
		return;
	}
	else 
	{
		
		*substring = 0;
		 substring = NULL;

		if(strncmp(pr_buffer, "GET ", 4) == 0) {	
			substring = pr_buffer + 5;
		}
		else
		{
			m = "You have performed an illegal operation";
			server_messages(403, newsockfd, m, 0L);
		}
		 
		filename = strtok(substring, " ");
		//if(substring !=NULL) {

		int is_acc;
		// if no page specified at root, load default web page
		if(filename != NULL) {
				printf("the filename is: %s\n", filename);
				printf("...and substring is %s\n", substring);
		//if((is_acc = (access(filename, R_OK))) != 0)
		if((is_acc = (access(filename, R_OK|W_OK))) != 0) {
			if(access(filename, F_OK) != 0)
					server_messages(404, newsockfd, "aplication/octect-stream", 0L);
			else {
				//ext_type = "application/octect-stream";	// send this one to the client in HTTP header response
				m = "You are not authorized to open this file";
				server_messages(403, newsockfd, m, 0L);
			}
				return;
		}

			// Identify file extension
			string_size = strlen(pr_buffer); // get the length of pr_buffer and store it in string_size
			char *cmp1, *cmp2;
	       	for(i=0; supported_f[i].extension != 0; i++) {
				ext_size = strlen(supported_f[i].extension);	// ext_size: holds file extension size value
				cmp1 = &pr_buffer[string_size-ext_size];	// detach extension from file for comparison
				cmp2 = supported_f[i].extension;		// get one of the supported server extensions type
			
				int r;	// to obtain which file extension the file has
				r = strcmp(cmp1, cmp2);
				//printf("the value of r is: %d\n", r);
			
				if((r == 0) && (supported_f[i].extension != 0 )) // if the filename requested have a supported extension...
				{
					printf("this is pr_buffer[%s]\n", cmp1); // this is the filename extension
					printf("this is supported_f[i]_extension: %s\n", cmp2); // supported by our server extension
					ext_type = supported_f[i].type_f; // get the extension type to send to client in HTTP header 
					break;
				}
		    }
			if(ext_type == 0) { // if extension type is not supported in my_server
				//ext_type = "application/octect-stream";	// send this one to the client in HTTP header response
				m = "You have performed an illegal operation.";
				server_messages(403, newsockfd, m, 0L);
				return;
			}
			else {
			
				printf("size of pr_buffer is %d\n", strlen(pr_buffer));
				// handle recognized file extensions
				opened_file = fopen(filename, "r");	// opened_file is a FILE type
	
				if(opened_file == NULL) {	// if there is no file...
	          			server_messages(404, newsockfd, "aplication/octect-stream", 0L);
					return;
				}                      
				else {
					// if file exists... look for content with fopen()
					printf("extension type is : %s\n", ext_type); 
	
					fseek(opened_file, 0L, SEEK_END);	// go to end of file
					opened_file_size = ftell(opened_file);	// then save it to obtain the file's length
					fseek(opened_file, 0L, SEEK_SET);
					printf("The size of the opened file is: %ld\n", opened_file_size);
					server_messages(200, newsockfd, ext_type, opened_file_size);
				
	
					// fill pr_buffer with zeroes
					int w;
					for(w = 0; w < DATA_SIZE; w++)
	                   	pr_buffer[w] = 0;
		
					int tmp;
					while(( tmp = fread(pr_buffer, sizeof(char), DATA_SIZE, opened_file)) > 0)
						write(newsockfd, pr_buffer, tmp);
					printf("pr_buffer = %d\n", strlen(pr_buffer));
				}
			}
		}
		else // if no filename then load default web-page
		{
				opened_file = fopen("default.html", "r");	// opened_file is a FILE type
				if(opened_file == NULL) {	// if there is no file...
	          		server_messages(404, newsockfd, "aplication/octect-stream", 0L);
					return;
				}                      
				else {
					printf("No file name specified. Loading Default Web Page\n");
	
					fseek(opened_file, 0L, SEEK_END);	// go to end of file
					opened_file_size = ftell(opened_file);	// then save it to obtain the file's length
					fseek(opened_file, 0L, SEEK_SET);
					printf("The size of the opened file is: %ld\n", opened_file_size);
					server_messages(200, newsockfd, ext_type, opened_file_size);

					// fill pr_buffer with zeroes
					int w;
					for(w = 0; w < DATA_SIZE; w++)
	                   	pr_buffer[w] = 0;
		
					int tmp;
					while(( tmp = fread(pr_buffer, sizeof(char), DATA_SIZE, opened_file)) > 0)
						write(newsockfd, pr_buffer, tmp);
					printf("pr_buffer = %d\n", strlen(pr_buffer));
				}
		}
	}
	usleep(100);
	fclose(opened_file);
	close(newsockfd);
}

int main(int argc, char **argv)
{
	int sockfd, newsockfd; // file descriptors referring SOCK_STREAM types
	pid_t pid; // to use for the process_id
	struct sockaddr_in serv_addr, clt_addr;
	socklen_t addrlen; // in bytes size of struct addr of accept() function
	int portno;

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <server-port>\n", argv[0]);
		return 1;
	}

	portno = atoi(argv[1]); // retrieve port number as an integer

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		syserr("can't open socket");
	}
	
	printf("create socket...\n");
	memset(&serv_addr, 0, sizeof(serv_addr)); // initialized to all zeroes
	serv_addr.sin_family = AF_INET; // IPv4 family
	serv_addr.sin_addr.s_addr = INADDR_ANY; // IP address
	serv_addr.sin_port = htons(portno); // port number

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		syserr("can't bind");
	}

	printf("binding socket to port %d...\n", portno);
	listen(sockfd, 5);

	for(;;) {
		addrlen = sizeof(clt_addr);
		newsockfd =  accept(sockfd, (struct sockaddr *) &clt_addr, &addrlen);
		if(newsockfd < 0) {
			syserr("server can't accept");
		}

		printf("\nNew incoming connection\n");
		pid = fork();
		if(pid < 0) {
			syserr("Error on fork");
		}
		if(pid == 0) {
			close(sockfd);
			process_request(newsockfd);
			exit(0);
		}
		else {
			close(newsockfd);
		}
	}
	close(sockfd);
	return 0;
}

