FROM node:14
WORKDIR /usr/src/mobileApp
COPY package.json ./
COPY yarn.lock .
COPY lerna.json .
COPY packages/react-app ./packages/react-app
COPY packages/server ./packages/server
RUN yarn
RUN yarn build
EXPOSE 4000
CMD ["yarn", "prod"]