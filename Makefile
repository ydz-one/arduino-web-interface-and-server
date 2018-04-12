TARGETS = server

all: $(TARGETS)


server: server.cc request.cc response.cc
	g++ $^ -std=c++11 -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lcrypto -lpthread -g -o $@


clean::
	rm -fv $(TARGETS) *~
