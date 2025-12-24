FROM rootproject/root:6.30.06-ubuntu22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git python3 \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/tfhic
COPY . /opt/tfhic

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DTFHIC_WITH_ROOT=ON \
  && cmake --build build -j"$(nproc)" \
  && cmake --install build --prefix /opt/tfhic/install

ENV TFHIC_DATA=/opt/tfhic/install/share/tfhic/data
ENV TFHIC_CONF=/opt/tfhic/install/share/tfhic/conf
ENV TFHIC_OUT=/data/out

RUN mkdir -p /data/out

WORKDIR /data
