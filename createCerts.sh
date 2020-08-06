#!/bin/bash
mkdir certs
# Create a self signed Certificate
# 1. generate CA Keys and cert
openssl genrsa -des3 -out certs/ca.key 2048
openssl req -new -x509 -days 1826 -key certs/ca.key -out certs/ca.crt
# 2. generate broker keys
openssl genrsa -out certs/server.key 2048
# 3. certificate signing request (csr)
openssl req -new -out certs/server.csr -key certs/server.key
# 4. verify csr with ca, sign broker certificate
openssl x509 -req -in certs/server.csr -CA certs/ca.crt -CAkey certs/ca.key -CAcreateserial -out certs/server.crt -days 365

# Create a Combined PEM SSL Certificate/Key File for HAProxy
cat certs/server.crt certs/server.key > certs/fullchain.pem

# Cleanup
rm server.csr

# Import the certificate in K8s
# kubectl create secret tls tls-secret -o yaml --namespace=smart-garden --cert=certs/cert.pem --key=certs/key.pem --save-config=true > k8s/secrets/tls.secret.yml