FROM mminks/haproxy-docker-logging:latest

# Add personalized configuration
ADD haproxy.cfg /usr/local/etc/haproxy/haproxy.cfg

# Define working directory.
WORKDIR /usr/local/etc/haproxy

# Expose ports.
EXPOSE 80
EXPOSE 443
EXPOSE 1883
EXPOSE 8883