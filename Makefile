TARGETS = frontend

all: $(TARGETS)


frontend: frontend.cc request.cc response.cc
	g++ $^ -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lcrypto -lpthread -g -o $@


clean::
	rm -fv $(TARGETS) *~

