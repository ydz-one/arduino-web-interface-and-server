/*
 * request.cc
 *
 *  Created on: Apr 1, 2018
 *      Author: cis505
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <string>
#include "request.h"

using namespace std;

//Macro
#define BUFFERSIZE 10000

// function pointer
bool do_read(int socket_fd, vector<char>buffer, char **line_end);

/*
 * constructor for a request
 */
request::request(int client_fd) {
	vector<char> buffer;
	char buffer_char[BUFFERSIZE];
	int start = 0; // where the line start
	int end = 0; // where the line end
	int line = 0;
	bool message_body = false;
	// keep reading request
	while (true) {
		// find \r\n
		vector<char> end_char = {'\r', '\n'};
		printf("end_char size: %lu\n", end_char.size());
		printf("vector size: %lu\n", buffer.size());
		printf("end: %d\n", end);
		auto iter = search(buffer.begin() + end, buffer.end(), end_char.begin(), end_char.end());
		if (iter != buffer.end()) {
			// buffer has a statement
			end = iter - buffer.begin();
			printf("end: %d\n", end);
		} else if (message_body) {
			// following is message
			if (end == buffer.size()) {
				// keep reading
				int read_characters = read(client_fd, buffer_char, BUFFERSIZE);
				printf("read_characters: %d\n", read_characters);
				if (read_characters <= 0) {
					// no more content
					break;
				}
				// copy character to vector
				for (int i = 0; i < read_characters; i++) {
					buffer.push_back(buffer_char[i]);
					printf("buffer[%d]: %c\n", i, buffer[i]);
					buffer_char[i] = 0; // clear buffer_char
				}
			}
			start = end;
			end = buffer.end() - buffer.begin();
			printf("end in body: %d\n", end);
		} else {
			// need to keep reading from the socket
			// append to the end of the buffer
			int read_characters = read(client_fd, buffer_char, BUFFERSIZE);
			printf("read_characters: %d\n", read_characters);
			if (read_characters <= 0) {
				// no more content
				break;
			}
			// copy character to vector
			for (int i = 0; i < read_characters; i++) {
				buffer.push_back(buffer_char[i]);
				printf("buffer[%d]: %c\n", i, buffer[i]);
				buffer_char[i] = 0; // clear buffer_char
			}
			printf("vector size: %lu\n", buffer.size());
			printf("end: %d\n", end );
			continue;
		}
		if (!message_body) {
			end += 2; // skip \r\n
		}
		printf("start : %d\n", start);
		string receive(buffer.begin() + start, buffer.begin() + end);// get the line of statement
		printf("receive: ==========%s\n", receive.c_str());
		//end = end + 1; // skip \n
		if (!message_body) {
			start = end; // change start and skip '\n'
		}
//		if (message_body) {
//			line_end += 1; // skip \r
//		} else {
//			line_end += 2; // skip \r\n
//		}
		char copy[20480];
		strcpy(copy, receive.c_str());
		printf("====================copy: %s\n", copy);


		// first line
		if (line == 0) {
			// copy attribute of request
			char *token;
			token = strtok(copy, " \r");
			this->method = string(token);
			token = strtok(NULL, " ");
			this->path = string(token);
			token = strtok(NULL, " ");
			this->version = string(token, strlen(token) - 1); // skip the final \n
			// print debug information
			if (debugInfo) {
				printf("[%d] Method: %s\n", client_fd, this->method.c_str());
				printf("[%d] path: %s\n", client_fd, this->path.c_str());
				printf("[%d] HTTP Version: %s\n", client_fd, this->version.c_str());
			}
			line++;
		} else if (strcasecmp(copy, "\r\n") == 0) {
			// skip the blank line
			// mark the message body follows
			printf("message body!!!!!!!!!!!!!!!!\n");
			message_body = true;
		} else if (message_body) {
			printf("message body!!!!!!!!!!!!!!!!\n");
			printf("start: %d\n", start);
			printf("end: %d\n", end);
			this->message.insert(this->message.end(), buffer.begin() + start, buffer.begin() + end);
			printf("message size: %lu\n", this->message.size());
		}

		//free(copy);


		// remove the used line statement from buffer
//		char *temp = buffer;
//		while (temp != line_end) {
//			*temp = '\0'; // clear old line
//			temp++;
//		}
//		// copy new line to the start of buffer
//		int len = strlen(line_end);
//		for (int i = 0; i < len; i++) {
//			buffer[i] = line_end[i];
//			line_end[i] = '\0';
//		}
//		buffer[len] = '\0';
//
//		// move line end to end of buffer
//		line_end = buffer;
//		while (*line_end != '\0') {
//			line_end++;
//		}
//

	}
}


/*
 * read from sever or client
 * return true if read successfully and found a line otherwise return false
 */
//bool do_read(int socket_fd, char *buffer, char **line_end) {
//
//	int n = read(socket_fd, *line_end, BUFFERSIZE - strlen(buffer));
//	if (n <= 0) {
//		return false;
//	}
//	char *tmp = buffer;
//	while ((tmp = strstr(buffer, "\r\n")) == NULL ) {
//		*line_end += n;// go to end of current statement
//		n = read(socket_fd, *line_end, BUFFERSIZE - strlen(buffer)); // keep reading
//		if (n <= 0) {
//			return false;
//		}
//	}
//	*line_end = tmp;// copy the end pointer to line_end
//	return true;
//}
