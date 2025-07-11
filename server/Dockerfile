FROM alpine:latest


RUN apk update && apk add gcc musl-dev make
WORKDIR app
COPY . .
RUN make

CMD ["./entrypoint.sh"]