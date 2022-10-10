FROM ubuntu:20.04
RUN DEBIAN_FRONTEND=noninteractive \
  apt-get update \
  && apt-get install -y sudo libsqlite3-dev cmake g++
RUN useradd -ms /bin/bash distrinet && echo "distrinet:distrinet" | chpasswd && adduser distrinet sudo
USER distrinet
CMD /bin/bash
WORKDIR /home/distrinet/