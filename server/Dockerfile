FROM alpine:latest


RUN apk update && apk add gcc \
    musl-dev \
    make \
    cmake \
    openssl-dev \
    g++ \
    linux-headers

WORKDIR app
COPY . .
RUN make DEPS_ONLY=true
RUN make

CMD ["./entrypoint.sh"]