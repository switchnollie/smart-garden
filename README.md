# Smart Garden

## Required Environment Variables

- MONGO_USERNAME
- MONGO_PASSWORD
- MONGO_DATABASE
- MONGO_PORT
- MONGO_HOST

## Start locally using Docker Compose

- Install Docker Community Edition in a up-to-date version.
- Provide the environment variables (e.g. as a `.env` File)
- Start the Docker Containers using Docker Compose: 

```sh
docker-compose up
```

## Start locally using Kubernetes (e.g. minikube)

- Install Docker Community Edition and a Kubernetes Development Environment like `minikube`
- Provide the environment variables with the required credentials for the Mongo DB as a Kubernetes Secret.

```sh
kubectl create -f dev-secret.yaml
```

- The Kubernetes Secret is written like so:

```yml
apiVersion: v1
kind: Secret
metadata:
  name: mongo-secret
type: Opaque
data:
  MONGO_USERNAME=...
  MONGO_PASSWORD=...
  MONGO_DATABASE=...
  MONGO_PORT=1234
  MONGO_HOST=...
```