FROM i386/ubuntu:latest


RUN apt-get update && apt-get install -y locales && rm -rf /var/lib/apt/lists/* \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
ENV LANG en_US.utf8

RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y gcc libevent-dev libevent-2.1-6 libdbi-dev make g++

RUN mkdir -p /opt/btmux/

COPY btmux /opt/btmux/

COPY game /opt/btmux/game
COPY maps /opt/btmux/game/maps
COPY text /opt/btmux/game/text
COPY mechs /opt/btmux/game/mechs

WORKDIR /opt/btmux/

RUN ./configure --enable-sql-support && make -j 4 && make install 

EXPOSE 5555

WORKDIR /opt/btmux/game.run/

CMD ["/bin/bash","-c", "/opt/btmux/game.run/Startmux-docker"]

