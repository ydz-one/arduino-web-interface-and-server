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

// global variable
int port_number;
int fd_usb;
char* file_name;
char temperature[100];
char buffer_temperature[100];
string avg_temperature_C = string("0");
string avg_temperature_F = string("0");
string min_temperature_C = string("200");
string min_temperature_F = string("200");
string max_temperature_C = string("0");
string max_temperature_F = string("0");
vector<string> all_temperature_C;
vector<string> all_temperature_F;
float temperature_sum_C = 0;
float temperature_sum_F = 0;

// functional pointer
void parse_temperature();
string convert_C_to_F(string temp);
string convert_F_to_C(string temp);


void configure(int fd) {
  struct  termios pts;
  tcgetattr(fd, &pts);
  cfsetospeed(&pts, 9600);   
  cfsetispeed(&pts, 9600);   
  tcsetattr(fd, TCSANOW, &pts);
}

void* handle_request(void* fd_pointer){

  int client_fd = *(int *)fd_pointer;
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
      if (::bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
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
	int bytes_read = read(fd_usb, buffer_temperature + counter, 100 - counter);
	while(bytes_read < 0){
	  bytes_read = read(fd_usb, buffer_temperature + counter, 100 - counter);
	}
	counter += bytes_read;
	if(buffer_temperature[counter - 1] == '\n'){
	  break_counter++;
	  buffer_temperature[counter] = '\0';
	  counter = 0;
	  if(break_counter > 2){
		strcpy(temperature, buffer_temperature);
		parse_temperature(); // store the result to the vector
		sleep(1); // read one new temperature per second
	  }
	  memset(buffer_temperature, '\0', 100);
	}
  }
  close(fd_usb);
  free(file_name);

}

/*
 * parse the temperature string and update the result
 */
void parse_temperature() {
	string temp;
	string unit;
	char token_array[100];
	char token_array_2[100];
	char copy[100];
	strcpy(copy, temperature);

	char* token;

	token = strtok(copy, " ");

	// get temperature and unit
	strcpy(token_array, token);
	temp = string(token_array);
	temp = temp.substr(0, 4);
	token = strtok(NULL, " ");

	strcpy(token_array_2, token);
	unit = string(token_array_2);

	string temp_F;
	string temp_C;

	// update the vector
	if (strcasecmp(unit.c_str(), "c\n") == 0) {
		// Celsius
		temp_C = temp;
		temp_F = convert_C_to_F(temp);
	} else if (strcasecmp(unit.c_str(), "f\n") == 0) {
		// Fahrenheit
		temp_F = temp;
		temp_C = convert_F_to_C(temp);
	}

	all_temperature_C.push_back(temp_C);
	all_temperature_F.push_back(temp_F);

	float remove_C = 0;
	float remove_F = 0;

	if (all_temperature_C.size() == 3600) {
		// only record one hour
		remove_C = atof(all_temperature_C.at(0).c_str());
		all_temperature_C.erase(all_temperature_C.begin());
		remove_F = atof(all_temperature_F.at(0).c_str());
		all_temperature_F.erase(all_temperature_C.begin());
	}

	// update global variable
	float ftemp_F = atof(temp_F.c_str());
	float ftemp_C = atof(temp_C.c_str());

	temperature_sum_C += ftemp_C - remove_C;
	temperature_sum_F += ftemp_F - remove_F;

	avg_temperature_C = to_string(temperature_sum_C / all_temperature_C.size()).substr(0, 4);
	avg_temperature_F = to_string(temperature_sum_F / all_temperature_F.size()).substr(0, 4);

	if (ftemp_F > atof(max_temperature_F.c_str())) {
		max_temperature_F = temp_F;
	}

	if (ftemp_F < atof(min_temperature_F.c_str())) {
		min_temperature_F = temp_F;
	}

	if (ftemp_C > atof(max_temperature_C.c_str())) {
		max_temperature_C = temp_C;
	}

	if (ftemp_C < atof(min_temperature_C.c_str())) {
		min_temperature_C = temp_C;
	}
}

/*
 * convert temperature unit from C to F
 */
string convert_C_to_F(string temp) {
	float t = atof(temp.c_str());
	float f = t * 1.8 + 32;
	string temp_F = to_string(f);
	return temp_F.substr(0, 4);
}

/*
 * convert temperature unit from F to C
 */
string convert_F_to_C(string temp) {
	float t = atof(temp.c_str());
	float c = (t - 32) / 1.8;
	string temp_C = to_string(c);
	return temp_C.substr(0, 4);
}



