ARG ALPINE_VERSION=3.20
ARG NODE_VERSION=22-alpine
ARG RUNTIME_IMAGE=alpine:${ALPINE_VERSION}

FROM node:${NODE_VERSION} AS frontend-builder
LABEL stage="frontend" org.opencontainers.image.source="https://example.com/docker"
WORKDIR /workspace/web

ARG PUBLIC_URL=https://cdn.example.com/assets
ENV CI=true \
    NODE_ENV=production \
    PUBLIC_URL=${PUBLIC_URL}

COPY web/package.json web/package-lock.json ./
RUN npm ci --include=dev

COPY web/ ./
RUN npm run lint
RUN npm run build -- --base="${PUBLIC_URL}"

FROM alpine:${ALPINE_VERSION} AS docs
WORKDIR /docs
ADD https://example.com/assets/config.json ./config.json
ADD https://example.com/assets/logo.svg ./logo.svg
RUN printf '%s\n' \
    '{' \
    '  "name": "sweetline",' \
    '  "docs": "https://example.com/docker",' \
    '  "support": "support@example.com"' \
    '}' > manifest.json

FROM ghcr.io/example/toolchain:1.9 AS builder
SHELL ["/bin/sh", "-euxo", "pipefail", "-c"]
WORKDIR /src

ARG BUILD_TYPE=Release
ARG ENABLE_TESTS=1
ARG GIT_SHA=dev
ARG APP_HOME=/opt/sweetline

ENV APP_HOME=${APP_HOME} \
    BUILD_TYPE=${BUILD_TYPE} \
    ENABLE_TESTS=${ENABLE_TESTS} \
    SWEETLINE_DOCS=https://example.com/docker

COPY cmake/ ./cmake/
COPY include/ ./include/
COPY src/ ./src/
COPY tests/ ./tests/
COPY CMakeLists.txt README.md LICENSE ./

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DENABLE_TESTS="${ENABLE_TESTS}" \
    -DSWEETLINE_GIT_SHA="${GIT_SHA}"
RUN cmake --build build --target sweetline_cli -j"$(nproc)"
RUN if [ "${ENABLE_TESTS}" = "1" ]; then ctest --test-dir build --output-on-failure; fi

FROM ${RUNTIME_IMAGE} AS runtime
LABEL org.opencontainers.image.title="SweetLine" \
      org.opencontainers.image.description="Rich syntax highlighting demo container" \
      org.opencontainers.image.documentation="https://example.com/docker" \
      org.opencontainers.image.licenses="MIT"

ARG APP_HOME=/opt/sweetline
ARG USER_NAME=sweetline
ARG USER_UID=10001
ARG USER_GID=10001

ENV APP_HOME=${APP_HOME} \
    PATH=${APP_HOME}/bin:${PATH} \
    SWEETLINE_MODE=server \
    SWEETLINE_CONFIG=${APP_HOME}/config/runtime.json

WORKDIR ${APP_HOME}

RUN addgroup -g "${USER_GID}" "${USER_NAME}" \
 && adduser -D -u "${USER_UID}" -G "${USER_NAME}" "${USER_NAME}" \
 && mkdir -p "${APP_HOME}/bin" "${APP_HOME}/config" "${APP_HOME}/share" "${APP_HOME}/logs" \
 && chown -R "${USER_NAME}:${USER_NAME}" "${APP_HOME}"

COPY --from=builder /src/build/bin/sweetline_cli ./bin/sweetline
COPY --from=frontend-builder /workspace/web/dist ./share/web
COPY --from=docs /docs/manifest.json ./config/runtime.json
COPY --from=docs /docs/config.json ./config/upstream.json
COPY --chmod=755 scripts/start.sh ./bin/start.sh
COPY --chown=${USER_NAME}:${USER_NAME} docker/entrypoint.sh ./entrypoint.sh

RUN printf '%s\n' \
    '#!/bin/sh' \
    'set -eu' \
    'echo "Docs: https://example.com/docker"' \
    'exec "${APP_HOME}/bin/sweetline" "$@"' > ./bin/healthcheck.sh \
 && chmod +x ./bin/healthcheck.sh ./entrypoint.sh

EXPOSE 8080 8443
VOLUME ["${APP_HOME}/logs", "${APP_HOME}/config"]
STOPSIGNAL SIGTERM

HEALTHCHECK --interval=30s --timeout=3s --start-period=10s --retries=3 \
  CMD ["/bin/sh", "-c", "${APP_HOME}/bin/healthcheck.sh --ping http://127.0.0.1:8080/healthz"]

USER ${USER_NAME}

ENTRYPOINT ["./entrypoint.sh"]
CMD ["serve", "--listen", "0.0.0.0:8080", "--docs", "https://example.com/docker"]

FROM runtime AS debug
ARG DEBUG_PORT=9229
ENV SWEETLINE_MODE=debug
ENV SWEETLINE_TRACE_URL=https://example.com/trace \
    SWEETLINE_LOG_LEVEL=debug
RUN apk add --no-cache curl jq
RUN printf '%s\n' \
    'debug=true' \
    'trace_url=https://example.com/trace' \
    'feature_flags=syntax,url,indent' > ./config/debug.env
HEALTHCHECK --interval=15s --timeout=2s \
  CMD ["/bin/sh", "-c", "curl -fsS http://127.0.0.1:8080/healthz | jq -e '.status' >/dev/null"]
CMD ["serve", "--listen", "0.0.0.0:8080", "--verbose"]
