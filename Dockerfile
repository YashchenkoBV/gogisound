FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y build-essential make libsndfile1-dev

WORKDIR /app

COPY . .

ENV SRC_DIR=src
ENV TEST_DIR=tests
ENV BUILD_DIR=build

RUN mkdir -p $BUILD_DIR

CMD ["make", "-C", "/app"]