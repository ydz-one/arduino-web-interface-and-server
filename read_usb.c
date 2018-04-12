#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include "read_usb.h"
/*
This code configures the file descriptor for use as a serial port.
*/
//char buffer[100];

void configure(int fd) {
  struct  termios pts;
  tcgetattr(fd, &pts);
  cfsetospeed(&pts, 9600);   
  cfsetispeed(&pts, 9600);   
  tcsetattr(fd, TCSANOW, &pts);
}

/*
void* read_usb(void* file_name) {

  // try to open the file for reading and writing
  // you may need to change the flags depending on your platform
  printf("%s\n", (char*)file_name);
  int fd = open((char*)file_name, O_RDWR | O_NOCTTY | O_NDELAY);
  
  if (fd < 0) {
    perror("Could not open file\n");
    exit(1);
  }
  else {
    printf("Successfully opened %s for reading and writing\n", (char*)file_name);
  }

  configure(fd);

  /*
    Write the rest of the program below, using the read and write system calls.
  */
  /*
  int counter = 0;
  int break_counter = 0;
  while(1){
    int bytes_read = read(fd, buffer_temperature + counter, 100 - counter);
    counter += bytes_read;
    if(buffer_temperature[counter - 1] == '\n'){
      break_counter++;
      counter = 0;
      //printf("%s", buffer);
      if(break_counter > 5){
        close(fd);
        break;
      }
      memset(buffer_temperature, '\0', 100);
    }
  }
  return (void*)buffer_temperature;
}*/
