# Smart Garden

## Required Environment Variables

- MONGO_USERNAME
- MONGO_PASSWORD
- MONGO_DATABASE
- MONGO_PORT
- MONGO_HOST

## Start all containers locally using Docker Compose and a local instance of MongoDB

- Install Docker Community Edition in a up-to-date version.
- Provide the mongo environment variables (e.g. as a `.env` File)
- Start the Docker Containers using Docker Compose: 

```sh
docker-compose up
```

## Start a local cluster using Kubernetes minikube, haproxy and an externally managed instance of MongoDB

- Install the Kubernetes Development Environment `minikube` and `kubectl`

- Configure minikube to use a vm-driver (Ingress Nodes are not supported in the default Docker environment). Recommended driver is `hyperkit`. (If minikube can't access the Docker Registry, follow [these steps](https://github.com/kubernetes/minikube/issues/4589#issuecomment-614631444))

```sh
minikube start --driver=hyperkit
```

- Install Voyager in your minikube cluster. If the above is not working follow the [official installation instructions](https://voyagermesh.com/docs/v12.0.0/setup/install/#script)

```sh
kubectl apply -f k8s/voyager.yaml
```

- Create a new kubernetes namespace called `smart-garden`:

```sh
kubectl create namespace smart-garden
```

- Provide the mongodb environment variables as a `.env` File, create a k8s secret called `mongo-secret` with them and save the secret in a `*.secret.yml`-File

```sh
mkdir k8s/secrets && kubectl create secret generic dev-secret --from-env-file=.env -o yaml --save-config=true --namespace=smart-garden > k8s/secrets/mongo.secret.yml
```

- Create a self signed certificate and import it into your minikube cluster as a secret. The voyager ingress node uses it to setup haproxy with tls:

```sh
./createCerts.sh
```

- Apply the k8s configurations

```sh
kubectl apply -f ./k8s/broker -f ./k8s/ingress.yml
```

### Test a single container locally

```sh
docker run -it --rm --env-file .env <image-name>
```

### Test MQTT connection using the mosquitto client

- Publish a message on the test topic

```sh
mosquitto_pub
```