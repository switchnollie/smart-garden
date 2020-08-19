# About

> This folder contains Kubernetes Cluster Configurations for a distributed microservice deployment
> Some aspects are not fully tested, we recommend the deployment on a single machine as a set
> of Docker containers (Docker compose)

## Start a local cluster using Kubernetes minikube, haproxy and an externally managed instance of MongoDB

- Install the Kubernetes Development Environment `minikube` and `kubectl`

- Start minikube with the standard docker driver

```sh
minikube start
```

- Install Voyager in your minikube cluster. If the above is not working follow the [official installation instructions](https://voyagermesh.com/docs/v12.0.0/setup/install/#script)

```sh
kubectl apply -f voyager.yaml
```

- Create a new kubernetes namespace called `smart-garden`:

```sh
kubectl create namespace smart-garden
```

- Provide the mongodb environment variables as a `.env` File, create a k8s secret called `mongo-secret` with them and save the secret in a `*.secret.yml`-File

```sh
mkdir secrets && kubectl create secret generic dev-secret --from-env-file=.env -o yaml --save-config=true --namespace=smart-garden > secrets/mongo.secret.yml
```

- Create a self signed certificate and import it into your minikube cluster as a secret. The voyager ingress node uses it to setup haproxy with tls:

```sh
./createCerts.sh
```

- Apply the k8s configurations

```sh
kubectl apply -f ./broker -f ./ingress.yml
```

### Test a single container locally

```sh
docker run -it --rm --env-file .env <image-name>
```

### Get Information about the Voyager Ingress

> voyager ingress shows up under a custom ressource definitions (CRD) and not under
> the ingress type ressources!
> this means that neither commands like `kubectl get ingress` nor the minikube dashboard
> will show the instances. Use type **ingress.voyager.appscode.com** instead!

```sh
kubectl get ingress.voyager.appscode.com -n smart-garden smart-garden-ingress
```
