FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive
ARG USERNAME=default

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        curl \
        vim \
        sudo \
        openssh-server

RUN useradd --create-home --shell /bin/bash $USERNAME \
    && usermod -aG sudo $USERNAME \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers \
    && mkdir -p /home/$USERNAME/code \
    && chown -R $USERNAME:$USERNAME /home/$USERNAME/code
COPY --chown=$USERNAME:$USERNAME ./container/.bash_aliases /home/$USERNAME/.bash_aliases

COPY docker-entrypoint.sh /usr/local/bin/docker-entrypoint.sh
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

WORKDIR /home/$USERNAME/code
ENTRYPOINT ["/usr/local/bin/docker-entrypoint.sh"]
