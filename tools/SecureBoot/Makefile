all: clean build

build:
	$(CC) generate_public_key_code.c rsa_params.c -o generate_public_key_code -lssl -lcrypto

clean:
	rm -f generate_public_key_code
	rm -f rsa_public_key.h
