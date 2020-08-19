# Smart Garden

## Required Environment Variables

| Env Variable     | Description                           |
| ---------------- | ------------------------------------- |
| `MONGO_USERNAME` | Username of Mongo Atlas Instance      |
| `MONGO_PASSWORD` | Password of Mongo Atlas Instance      |
| `MONGO_DATABASE` | Database Name of Mongo Atlas Instance |
| `MONGO_PORT`     | Port of Mongo Atlas Instance          |
| `MONGO_HOST`     | Hostname/ URI of Mongo Atlas Instance |

## Development setup

To start the application locally, you will need a MongoDB Atlas instance as well as a Docker installation on your machine.

### Setup Steps

1. Install Docker Community Edition in an up-to-date version.
2. Provide the mongo environment variables (e.g. as a `.env` File)
3. Create the self signed certificates for the application gateway (haproxy):

   ```sh
   ./createCerts.sh
   ```

   This will prompt you to:

   - Enter a passphrase for the CA keys
   - Enter Information about your self signed CA cert
   - Enter Information about the server cert.

4) Generate Keys for the API-Server (used for JWT issuance and validation)

5. Start the Docker Containers using Docker Compose:

```sh
docker-compose up -f docker-compose.dev.yml
```

## Production Deployment (Digital Ocean)

The production deployment is running on a Digital Ocean Droplet (VPS). The machine allows connections on Port 80, 443, 1883 and 8883. All connections are proxied through the HAProxy Application Gateway which redirects unencrypted traffic for MQTT (1883) and HTTP (80) to use the encrypted versions.

The certificates are issued by Let's Encrypt using the Certbot Client which is deployed as a seperate microservice.

A Hostname of `smartgarden.timweise.com` is associated with the droplet i.e. the mobile web application can be accessed on that domain and the MQTT clients can connect with the broker on `smartgarden.timweise.com:8883`.

### Test the Production MQTT Connection with the Mosquitto Client

- Subscribe to the topic `test`

```sh
mosquitto_sub -h smartgarden.timweise.com -p 8883 -t test -d --cafile letsencryptRootCa.pem
```

- Publish Moisture Value `42` on the topic `5f2d2b58d65dd0c3e0ac05e7/5f2d2bfe7824f2b9fd33cb66/5f2d2f46c254098c1222a484/moisture`
  (DeviceId 5f2d2f46c254098c1222a484, WateringGroupId: 5f2d2bfe7824f2b9fd33cb66, UserId: 5f2d2b58d65dd0c3e0ac05e7)

```sh
mosquitto_pub -h smartgarden.timweise.com -p 8883 -t 5f2d2b58d65dd0c3e0ac05e7/5f2d2bfe7824f2b9fd33cb66/5f2d2f46c254098c1222a484/moisture -m 42 --cafile letsencryptRootCa.pem -i 5f2d2f46c254098c1222a484
```

- Publish on Pump Topic `5f2d2b58d65dd0c3e0ac05e7/5f2d2bfe7824f2b9fd33cb66/5f2d2f515e9536fb08962ba5/pump` (DeviceId 5f2d2f515e9536fb08962ba5, WateringGroupId: 5f2d2bfe7824f2b9fd33cb66, UserId: 5f2d2b58d65dd0c3e0ac05e7)

```sh
mosquitto_pub -h smartgarden.timweise.com -p 8883 -t 5f2d2b58d65dd0c3e0ac05e7/5f2d2bfe7824f2b9fd33cb66/5f2d2f515e9536fb08962ba5/pump -m 1 --cafile letsencryptRootCa.pem -i admin-app
```
