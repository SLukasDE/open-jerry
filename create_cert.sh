#!/bin/sh

#openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.pem -sha256 -days 365 -nodes
openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.pub -sha256 -days 365 -nodes
