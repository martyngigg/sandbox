# Copyright (c) Jupyter Development Team.
# Distributed under the terms of the Modified BSD License.

# Debian Jessie debootstrap from 2017-02-27
# https://github.com/docker-library/official-images/commit/aa5973d0c918c70c035ec0746b8acaec3a4d7777
FROM debian@sha256:52af198afd8c264f1035206ca66a5c48e602afb32dc912ebf9e9478134601ec4

MAINTAINER Jupyter Project <jupyter@googlegroups.com>

USER root

# Install all OS dependencies for notebook server that starts but lacks all
# features (e.g., download as all possible file formats)
ENV DEBIAN_FRONTEND noninteractive
RUN REPO=http://cdn-fastly.deb.debian.org \
 && echo "deb $REPO/debian jessie main\ndeb $REPO/debian-security jessie/updates main" > /etc/apt/sources.list \
 && apt-get update && apt-get -yq dist-upgrade \
 && apt-get install -yq --no-install-recommends \
    wget \
    bzip2 \
    ca-certificates \
    sudo \
    locales \
    fonts-liberation \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

RUN echo "en_US.UTF-8 UTF-8" > /etc/locale.gen && \
    locale-gen

# Install Tini
RUN wget --quiet https://github.com/krallin/tini/releases/download/v0.10.0/tini && \
    echo "1361527f39190a7338a0b434bd8c88ff7233ce7b9a4876f3315c22fce7eca1b0 *tini" | sha256sum -c - && \
    mv tini /usr/local/bin/tini && \
    chmod +x /usr/local/bin/tini

# Configure environment
ENV CONDA_DIR /opt/conda
ENV PATH $CONDA_DIR/bin:$PATH
ENV SHELL /bin/bash
ENV NB_USER jovyan
ENV NB_UID 1000
ENV HOME /home/$NB_USER
ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

# Create jovyan user with UID=1000 and in the 'users' group
RUN useradd -m -s /bin/bash -N -u $NB_UID $NB_USER && \
    mkdir -p $CONDA_DIR && \
    chown $NB_USER $CONDA_DIR

USER $NB_USER

# Setup work directory for backward-compatibility
RUN mkdir /home/$NB_USER/work

# Install conda as jovyan
RUN cd /tmp && \
    mkdir -p $CONDA_DIR && \
    wget --quiet https://repo.continuum.io/miniconda/Miniconda3-4.2.12-Linux-x86_64.sh && \
    echo "c59b3dd3cad550ac7596e0d599b91e75d88826db132e4146030ef471bb434e9a *Miniconda3-4.2.12-Linux-x86_64.sh" | sha256sum -c - && \
    /bin/bash Miniconda3-4.2.12-Linux-x86_64.sh -f -b -p $CONDA_DIR && \
    rm Miniconda3-4.2.12-Linux-x86_64.sh && \
    $CONDA_DIR/bin/conda config --system --add channels conda-forge && \
    $CONDA_DIR/bin/conda config --system --set auto_update_conda false && \
    conda clean -tipsy

# Install IPython
RUN conda install --quiet --yes \
    'ipython=5.0.*' \
    'ipykernel' \
    'matplotlib' \
    && conda clean -tipsy

USER root

EXPOSE 8888
WORKDIR $HOME

# Configure container startup
ENTRYPOINT ["tini", "--"]
CMD ["start.sh"]

# Add local files as late as possible to avoid cache busting
COPY start.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/start.sh
#COPY start-notebook.sh /usr/local/bin/
#COPY start-singleuser.sh /usr/local/bin/
#COPY jupyter_notebook_config.py /etc/jupyter/
#RUN chown -R $NB_USER:users /etc/jupyter/

# Switch back to jovyan to avoid accidental container runs as root
USER $NB_USER
