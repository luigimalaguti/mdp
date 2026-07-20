FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive
ARG USERNAME=default

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        curl \
        vim \
        sudo \
        ca-certificates \
        openssh-server \
        build-essential \
        clang \
        clangd \
        git

RUN useradd --create-home --shell /bin/bash $USERNAME \
    && usermod -aG sudo $USERNAME \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers \
    && mkdir -p /home/$USERNAME/packages \
    && chown -R $USERNAME:$USERNAME /home/$USERNAME/packages
COPY --chown=$USERNAME:$USERNAME ./container/.bash_aliases /home/$USERNAME/.bash_aliases

COPY docker-entrypoint.sh /usr/local/bin/docker-entrypoint.sh
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

WORKDIR /home/$USERNAME/packages
ENTRYPOINT ["/usr/local/bin/docker-entrypoint.sh"]
