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
		} else {
			// need to keep reading from the socket
			// append to the end of the buffer
			int read_characters = read(client_fd, buffer_char, BUFFERSIZE);
			if (read_characters <= 0) {
				// no more content
				break;
			}
			// copy character to vector
			for (int i = 0; i < read_characters; i++) {
				buffer.push_back(buffer_char[i]);
				//printf("buffer[%d]: %c\n", i, buffer[i]);
				buffer_char[i] = 0; // clear buffer_char
			}

			continue;
		}
		if (!message_body) {
			end += 2; // skip \r\n
		}
		string receive(buffer.begin() + start, buffer.begin() + end);// get the line of statement
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
			if (true) {
				printf("[%d] Method: %s\n", client_fd, this->method.c_str());
				printf("[%d] path: %s\n", client_fd, this->path.c_str());
				printf("[%d] HTTP Version: %s\n", client_fd, this->version.c_str());
			}
			line++;
		} else if (strcasecmp(copy, "\r\n") == 0) {
			// skip the blank line
			// mark the message body follows
			message_body = true;
		} else if (message_body) {
			this->message.insert(this->message.end(), buffer.begin() + start, buffer.begin() + end);
		}

		

	}
}


