FROM ubuntu:22.04

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
    qtbase5-dev \
    qtchooser \
    qt5-qmake \
    qtbase5-dev-tools \
    build-essential \
    libqt5sql5-sqlite \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /root/server/

COPY server/*.cpp /root/server/
COPY server/*.h /root/server/
COPY server/*.pro /root/server/

RUN qmake echoServer.pro
RUN make

EXPOSE 33333

ENTRYPOINT ["./echoServer"]
