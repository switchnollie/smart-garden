FROM node:latest
WORKDIR /usr/src/broker
COPY package*.json ./
RUN npm install
COPY . .
EXPOSE 1883
CMD ["node", "index.js"]