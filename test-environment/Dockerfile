FROM ubuntu:22.04 AS veyon_build
ENV DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC
RUN apt update
RUN apt install -y g++ libc6-dev make cmake qtbase5-dev qtbase5-private-dev \
            qtbase5-dev-tools qttools5-dev qttools5-dev-tools \
            qtdeclarative5-dev qtquickcontrols2-5-dev libfakekey-dev \
            xorg-dev libxtst-dev libjpeg-dev zlib1g-dev libssl-dev libpam0g-dev \
            libprocps-dev liblzo2-dev libqca-qt5-2-dev libldap2-dev \
            libsasl2-dev ninja-build
RUN apt install -y libavcodec-dev libavformat-dev libswscale-dev file
RUN apt install -y git
RUN git clone --branch 1.1.1 --recurse-submodules https://github.com/ComputationalReflection/veyon.git
RUN mkdir -p /veyon/build
WORKDIR /veyon/build
RUN cmake -DCMAKE_INSTALL_PREFIX=/usr .. 
RUN make -j8
RUN make package


FROM trfore/docker-ubuntu2204-systemd AS student
ENV DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC
RUN apt-get update && apt-get install -y \
    xfce4 xrdp dbus-x11 x11-xserver-utils \
    sudo wget pulseaudio
RUN apt-get clean
RUN useradd -m ubuntu && echo "ubuntu:ubuntu" | chpasswd && adduser ubuntu sudo
RUN echo xfce4-session > /home/ubuntu/.xsession && \
    chown ubuntu:ubuntu /home/ubuntu/.xsession
RUN sed -i 's/port=3389/port=3389/' /etc/xrdp/xrdp.ini
RUN apt update
RUN apt install -y libqt5widgets5 libqt5gui5 libqt5core5a libfakekey0 \
            libxtst6 libjpeg-turbo8 zlib1g libssl3 libpam0g libprocps8 \
            liblzo2-2 libqca-qt5-2 libldap-2.5-0 libsasl2-2
RUN apt install -y libavcodec58 libavformat58 libswscale5
RUN apt install -y net-tools vim iputils-ping bash-completion
RUN apt install -y xfce4-terminal
RUN apt install -y chromium-browser
RUN apt clean
COPY --from=veyon_build /veyon/build/veyon_* /home/ubuntu/
WORKDIR /home/ubuntu
RUN dpkg -i veyon_*
RUN mkdir -p /home/ubuntu/veyon-screenshoots
COPY master.config.json .
RUN QT_QPA_PLATFORM=offscreen veyon-cli config import master.config.json
COPY 2025_public_key.pem .
COPY 2025_private_key.pem .
RUN QT_QPA_PLATFORM=offscreen veyon-cli authkeys import ubuntu/public 2025_public_key.pem
RUN QT_QPA_PLATFORM=offscreen veyon-cli authkeys import ubuntu/private 2025_private_key.pem
# RDP port
EXPOSE 3389
# Veyon port
EXPOSE 11100
ENV container=docker
# Instala systemd
RUN apt update && \
    apt install -y systemd systemd-sysv sudo
VOLUME [ "/sys/fs/cgroup" ]
STOPSIGNAL SIGRTMIN+3
CMD ["/lib/systemd/systemd"]


FROM accetto/ubuntu-vnc-xfce-chromium-g3:22.04 AS lecturer
ENV XDG_RUNTIME_DIR=/tmp/runtime-root
ENV VNC_PW=ubuntu
ENV DEBIAN_FRONTEND=noninteractive \
    TZ=Etc/UTC
USER root
RUN apt update
RUN apt install -y libqt5widgets5 libqt5gui5 libqt5core5a libfakekey0 \
            libxtst6 libjpeg-turbo8 zlib1g libssl3 libpam0g libprocps8 \
            liblzo2-2 libqca-qt5-2 libldap-2.5-0 libsasl2-2
RUN apt install -y libavcodec58 libavformat58 libswscale5
RUN apt install -y net-tools vim iputils-ping bash-completion
RUN apt install -y xfce4-terminal
RUN apt clean
RUN useradd -m ubuntu && echo "ubuntu:ubuntu" | chpasswd && adduser ubuntu sudo
COPY --from=veyon_build /veyon/build/veyon_* /home/ubuntu/
WORKDIR /home/ubuntu
RUN dpkg -i veyon_*
RUN mkdir -p /home/ubuntu/veyon-screenshoots
COPY master.config.json .
RUN QT_QPA_PLATFORM=offscreen veyon-cli config import master.config.json
COPY 2025_public_key.pem .
COPY 2025_private_key.pem .
RUN QT_QPA_PLATFORM=offscreen veyon-cli authkeys import ubuntu/public 2025_public_key.pem
RUN QT_QPA_PLATFORM=offscreen veyon-cli authkeys import ubuntu/private 2025_private_key.pem
# VNC ports
EXPOSE 5901
EXPOSE 6901
WORKDIR /dockerstartup