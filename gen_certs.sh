#!/bin/sh

CRT_DIR=certs
CA_KEY=$CRT_DIR/ca.key
CA_CRT=$CRT_DIR/ca.crt
SERVER_KEY=$CRT_DIR/server.key
SERVER_CSR=$CRT_DIR/server.csr
SERVER_CRT=$CRT_DIR/server.crt
CLIENT_KEY=$CRT_DIR/client.key
CLIENT_CSR=$CRT_DIR/client.csr
CLIENT_CRT=$CRT_DIR/client.crt

# generate CA key + cert
openssl genrsa -out $CA_KEY 2048 # ca key
openssl req -new -x509 -key $CA_KEY -days 1825 -out $CA_CRT # ca crt

# generate client keu + ca signed cert
openssl genrsa -out $CLIENT_KEY 2048 # client key
openssl req -new -key $CLIENT_KEY -out $CLIENT_CSR # client key signing request
openssl x509 -req -in $CLIENT_CSR -CA $CA_CRT -CAkey $CA_KEY -CAcreateserial -out $CLIENT_CRT -days 1825 # client crt

# generate server key + cert
openssl genrsa -out $SERVER_KEY 2048 # server key
openssl req -out $SERVER_CSR -key $SERVER_KEY -new # server key signing request
openssl x509 -req -in $SERVER_CSR -CA $CA_CRT -CAkey $CA_KEY -CAcreateserial -out $SERVER_CRT -days 1825 # server crt
