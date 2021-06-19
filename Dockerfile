FROM alpine as build-env
RUN apk add --no-cache autoconf gcc alpine-sdk libzmq zeromq-dev zeromq coreutils build-base 
WORKDIR /app
COPY . .
# Set up SQLite
WORKDIR ./src/sqlite
RUN CFLAGS="-DSQLITE_ENABLE_JSON1" ./configure; make install -f Makefile
# Compile the binaries
WORKDIR ../../
RUN make
FROM alpine as zpub
COPY --from=build-env /app/out/* /app/
RUN apk add libzmq 
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
COPY --from=build-env /usr/local/lib/libsqlite3.so /usr/local/lib/libsqlite3.so
COPY --from=build-env /usr/local/lib/libsqlite3.so.0.8.6 /usr/local/lib/libsqlite3.so.0.8.6
COPY --from=build-env /usr/local/lib/libsqlite3.a /usr/local/lib/libsqlite3.a
COPY --from=build-env /usr/local/lib/pkgconfig/sqlite3.pc /usr/local/lib/pkgconfig/sqlite3.pc
COPY --from=build-env /usr/local/lib/libsqlite3.la /usr/local/lib/libsqlite3.la
COPY --from=build-env /usr/local/lib/libsqlite3.so.0 /usr/local/lib/libsqlite3.so.0
COPY --from=build-env /usr/local/include/sqlite3.h /usr/local/include/sqlite3.h
COPY --from=build-env /usr/local/include/sqlite3ext.h /usr/local/include/sqlite3ext.h
RUN apk add libzmq 
WORKDIR /app
RUN chmod +x /app/*
CMD ["/app/zsub"]
