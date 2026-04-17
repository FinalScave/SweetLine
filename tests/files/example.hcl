# Example HCL configuration with block labels, interpolation, and URLs.
/* Docs: https://example.com/hcl */
locals {
  service_name = "sweetline"
  docs_url     = "https://example.com/docs"
  retries      = 3
  enabled      = true
  regions      = ["us-east", "eu-west", "ap-south"]
  ports        = [8080, 8443, 9090]
  owners = {
    platform = "platform@example.com"
    qa       = "qa@example.com"
  }
  labels = {
    service = "sweetline"
    docs    = "https://example.com/source"
    team    = "editor"
  }
}

service "api" "primary" {
  image    = "ghcr.io/example/api:1.2.3"
  endpoint = "https://api.example.com/v1"
  owner    = "platform"
  enabled  = local.enabled

  tags = {
    owner = "platform"
    docs  = "https://example.com/source"
    tier  = "gold"
  }

  command = [
    "sh",
    "-c",
    "curl -fsS https://example.com/health"
  ]

  check "startup" {
    type    = "http"
    target  = "https://api.example.com/healthz"
    timeout = "5s"
    headers = {
      Accept        = "application/json"
      X-Docs-Source = local.docs_url
    }
  }

  rollout {
    strategy   = "blue-green"
    max_surge  = 2
    max_unavail = 0
  }
}

service "worker" "batch" {
  image    = "ghcr.io/example/worker:4.0.0"
  endpoint = "https://jobs.example.com/run"
  owner    = "qa"

  environment = {
    APP_ENV      = "staging"
    APP_DOCS_URL = local.docs_url
    RETRIES      = local.retries
  }

  schedule = {
    cron      = "0 */6 * * *"
    timezone  = "UTC"
    suspended = false
  }
}

listener "https" "public" {
  address  = "0.0.0.0:443"
  protocol = "tls"

  tls {
    min_version = "1.2"
    cert_file   = "/etc/sweetline/tls/tls.crt"
    key_file    = "/etc/sweetline/tls/tls.key"
  }

  routes = [
    {
      name     = "api"
      match    = "/api/*"
      upstream = "api-primary"
    },
    {
      name     = "docs"
      match    = "/docs/*"
      upstream = "docs-site"
    }
  ]
}

feature "search" {
  enabled = true
  rollout = 0.25
  owners  = [for region in local.regions : "${region}@example.com"]
}

template = <<-EOF
Hello ${local.service_name}
Docs: https://example.com/docs
%{ if local.enabled }
retry count = ${local.retries}
ports = ${jsonencode(local.ports)}
%{ else }
disabled = true
%{ endif }
EOF

policy "rate_limit" "api" {
  match = {
    method = ["GET", "POST"]
    path   = "^/api/(v1|v2)/.*$"
  }

  limits = {
    requests_per_minute = 600
    burst               = 100
  }
}

profile "dev" {
  inherit = ["base", "logging"]
  values = {
    docs      = local.docs_url
    namespace = local.service_name
    retries   = local.retries
  }
}

output "summary" {
  value = {
    service = local.service_name
    docs    = local.docs_url
    owners  = local.owners
  }
}

# See https://example.com/hcl for details.
