FROM alpine:latest


RUN apk update && apk add gcc \
    musl-dev \
    make \
    cmake \
    openssl-dev \
    g++ \
    linux-headers \
    libwebsockets-dev

WORKDIR app
COPY ./entrypoint.sh ./entrypoint.sh
COPY ./public ./public
COPY ./Makefile ./Makefile
COPY ./src ./src
RUN make

CMD ["./entrypoint.sh"]