# Smart Garden

## Required Environment Variables

- MONGO_USERNAME
- MONGO_PASSWORD
- MONGO_DATABASE
- MONGO_PORT
- MONGO_HOST

## Start all containers locally using Docker Compose and a local instance of MongoDB

- Install Docker Community Edition in a up-to-date version.
- Provide the environment variables (e.g. as a `.env` File)
- Start the Docker Containers using Docker Compose: 

```sh
docker-compose up
```

## Start a local cluster using Kubernetes (e.g. minikube) and an externally managed instance of MongoDB

- Install Docker Community Edition and a Kubernetes Development Environment like `minikube`

- Provide the environment variables as a `.env` File and create a k8s secret called `dev-secret` with them

```sh
kubectl create secret generic dev-secret --from-env-file=.env --namespace=smart-garden
```

optionally you can also save the secret in a `*.secret.yml`-File:

```sh
kubectl create secret generic dev-secret --from-env-file=.env -o yaml --save-config=true --namespace=smart-garden > k8s/dev.secret.yml
```

### Test a single container locally

```sh
docker run -it --rm --env-file .env <image-name>
```