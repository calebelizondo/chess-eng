FROM gcc-alpine


WORKDIR app
COPY . .
RUN make

CMD ["./build/server"]