FROM alpine as build-env
RUN apk add --no-cache autoconf gcc alpine-sdk libzmq zeromq-dev zeromq coreutils build-base libuuid util-linux util-linux-dev
WORKDIR /app
COPY . .
# Compile the binaries
RUN make
CMD ["ls"]
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
RUN apk add libzmq 
WORKDIR /app
RUN chmod +x /app/*
CMD ["/app/zsub"]
