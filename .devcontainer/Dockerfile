ARG VERSION=26.04
FROM ubuntu:${VERSION} AS base

ARG DEBIAN_FRONTEND=noninteractive
USER root

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        git \
        curl \
        ca-certificates \
        vim \
        unzip \
        sudo \
        openssh-client \
        locales \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=C.UTF-8

ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=$USER_UID

COPY scripts/aliases.sh /etc/profile.d/00-aliases.sh
RUN userdel -f -r ubuntu 2>/dev/null || true \
    && groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID --create-home --shell /bin/bash $USERNAME \
    && echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && echo "source /etc/profile.d/00-aliases.sh" >> /etc/bash.bashrc \
    && mkdir -p /home/$USERNAME/.ssh \
    && (ssh-keyscan -t ed25519 github.com >> /home/$USERNAME/.ssh/known_hosts || true) \
    && chown -R dev:dev /home/$USERNAME/.ssh \
    && chmod 700 /home/$USERNAME/.ssh \
    && chmod 600 /home/$USERNAME/.ssh/known_hosts

FROM base AS cpp

ARG USERNAME=dev
ARG DEBIAN_FRONTEND=noninteractive
USER root

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        gcc \
        g++ \
        make \
        clang \
        clang-format \
        clang-tidy \
        clangd \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

USER $USERNAME

CMD ["/bin/bash"]
