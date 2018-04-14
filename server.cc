/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "response.h"
#include "request.h"

int port_number;
int fd_usb;
char* file_name;
char temperature[100];
char buffer_temperature[100];

void configure(int fd) {
  struct  termios pts;
  tcgetattr(fd, &pts);
  cfsetospeed(&pts, 9600);   
  cfsetispeed(&pts, 9600);   
  tcsetattr(fd, TCSANOW, &pts);
}

void* handle_request(void* fd_pointer){

  int client_fd = *(int *)fd_pointer;
  printf("Yeah!\n");
  // create new request
  request req(client_fd);
  response res(client_fd, req);
  close(client_fd);
  pthread_exit(NULL);

}

void* start_server(void* arg){

      // structs to represent the server and client
  //prinf("buffer is: %s\n", );
  //strcpy(temperature, (char*)buffer); 
  struct sockaddr_in server_addr,client_addr;    

      int sock; // socket descriptor

      // 1. socket: creates a socket descriptor that you later use to make other system calls
      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
      }
      int temp;
      if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
      }

      // configure the server
      server_addr.sin_port = htons(port_number); // specify port number
      server_addr.sin_family = AF_INET;         
      server_addr.sin_addr.s_addr = INADDR_ANY; 
      bzero(&(server_addr.sin_zero),8); 

      
      
      // 2. bind: use the socket and associate it with the port number
      if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("Unable to bind");
        exit(1);
      }

      // 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
      if (listen(sock, 1) == -1) {
        perror("Listen");
        exit(1);
      }

      // once you get here, the server is set up and about to start listening
      printf("\nServer configured to listen on port %d\n", port_number);
      fflush(stdout);


      // 4. accept: wait here until we get a connection on that port
      
        //printf("%s\n\n", buffer);
      while(1){
        int sin_size = sizeof(struct sockaddr_in);

        int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 1000;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        if (fd != -1) {
          pthread_t thread_request;
          printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
          int status_2 = pthread_create(&thread_request, NULL, handle_request, &fd);
          
          //printf("Server closed connection\n");
        }
        
      }
      //int status_3 = pthread_join(thread_request, NULL);
      // 8. close: close the socket
      close(sock);
      printf("Server shutting down\n");

      return 0;
    } 



    int main(int argc, char *argv[]){
  // check the number of arguments
      //pthread_t t1;

      if (argc != 3) {
        printf("\nUsage: %s [port_number] [file_name]\n", argv[0]);
        exit(-1);
      }

      port_number = atoi(argv[1]);
      file_name = (char*)malloc(sizeof(char) * (strlen(argv[2]) + 1));
      strcpy(file_name, argv[2]);

      if (port_number <= 1024) {
        printf("\nPlease specify a port number greater than 1024\n");
        exit(-1);
      }
      
      fd_usb = open(file_name, O_RDWR | O_NOCTTY | O_NDELAY);
      write(fd_usb, "f" ,1);
      if (fd_usb < 0) {
        perror("Could not open file\n");
        exit(1);
      }
      else {
        printf("Successfully opened %s for reading and writing\n", (char*)file_name);
      }

      configure(fd_usb);

  /*
    Write the rest of the program below, using the read and write system calls.
  */
      int counter = 0;
      int break_counter = 0;
      char buffer_cpy[100];
      pthread_t thread_accept;
      int status_1 = pthread_create(&thread_accept, NULL, start_server, NULL);
      while(1){
    //printf("testing...\n");
        int bytes_read = read(fd_usb, buffer_temperature + counter, 100 - counter);
        while(bytes_read < 0){
          bytes_read = read(fd_usb, buffer_temperature + counter, 100 - counter);
        }
        counter += bytes_read;
        //printf("%d\n", counter);
        if(buffer_temperature[counter - 1] == '\n'){
          break_counter++;
          buffer_temperature[counter] = '\0';
          counter = 0;
          if(break_counter > 2){
            strcpy(temperature, buffer_temperature);
          }
          memset(buffer_temperature, '\0', 100);
        }
      }
      //int status_4 = pthread_join(thread_accept, NULL);
      close(fd_usb);
      free(file_name);

    }


