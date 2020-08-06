FROM node:latest
WORKDIR /usr/src/dashboard
COPY package*.json ./
COPY lerna.json .
COPY packages/react-app ./packages/react-app
COPY packages/server ./packages/server
RUN yarn
RUN yarn build
EXPOSE 4000
CMD ["yarn", "prod"]