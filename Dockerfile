FROM alpine as build-env
RUN apk add --no-cache autoconf gcc alpine-sdk libzmq zeromq-dev zeromq coreutils build-base libuuid util-linux util-linux-dev sqlite-dev musl-dev libc-dev
WORKDIR /app
COPY . .
# Set up SQLite
#WORKDIR ./src/sqlite
#RUN CFLAGS="-DSQLITE_ENABLE_JSON1" ./configure; make install -f Makefile
# Compile the binaries
#WORKDIR ../../
RUN make
FROM alpine as zpub
COPY --from=build-env /app/out/* /app/
RUN apk add libzmq libuuid
WORKDIR /app
RUN chmod +x /app/*
CMD ["/app/zpub"]

FROM alpine as zpi
COPY --from=build-env /app/out/* /app/
RUN apk add libzmq 
WORKDIR /app
RUN chmod +x /app/*
CMD ["/app/zpi"]

FROM alpine as zsub
COPY --from=build-env /app/out/* /app/
RUN apk add libzmq sqlite-dev
WORKDIR /app
RUN chmod +x /app/*
CMD ["/app/zsub"]
