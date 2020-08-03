#!/bin/bash
# Create a self signed Certificate that is valid for one year and uses no passphrase
cd k8s/secrets
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes
# Import the certificate in K8s
kubectl create secret tls tls-secret --namespace=smart-garden --cert=cert.pem --key=key.pem > tls.secret.yml
