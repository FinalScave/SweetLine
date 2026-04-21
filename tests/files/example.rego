# METADATA
# title: SweetLine Rego coverage sample
# description: Exercises rule heads, comprehensions, metadata comments, raw strings, and imports.
# owners:
#   - platform
#   - security

package sweetline.authz

import rego.v1
import data.inventory.services as services
import data.inventory.images as images

default allow := false
default max_latency_ms := 250

allowed_roles := {"admin", "sre", "auditor"}
blocked_regions := {"moon-base", "offline-lab"}

valid_image(image) if {
  startswith(image, "registry.example.com/")
} else if {
  startswith(image, "ghcr.io/sweetline/")
}

user_is_break_glass if {
  raw_mode := `break-glass`
  input.headers["x-session-mode"] == raw_mode
}

allow if {
  input.user.role in allowed_roles
  not deny[_]
  count(warn) <= 10
}

deny contains msg if {
  input.region in blocked_regions
  msg := sprintf("region %v is blocked", [input.region])
}

deny contains msg if {
  some container in input.request.object.spec.containers
  image := container.image
  not valid_image(image)
  msg := sprintf("image %v is not trusted", [image])
}

deny contains msg if {
  input.request.kind.kind == "Deployment"
  replicas := input.request.object.spec.replicas
  replicas > 20
  msg := sprintf("replicas %v exceed hard limit", [replicas])
}

warn contains notice if {
  some service in services
  service.metadata.team == input.team
  latency := service.metrics.p95_ms
  latency > max_latency_ms
  notice := sprintf("service %v has p95=%v", [service.metadata.name, latency])
}

warn contains notice if {
  image_name := input.request.object.spec.containers[_].image
  digest := images[image_name].digest
  not endswith(digest, ":stable")
  notice := sprintf("image %v does not pin a stable digest", [image_name])
}

team_names contains team if {
  some service in services
  team := service.metadata.team
}

service_ports := [port |
  some service in services
  some port in service.ports
]

service_index := {name: service |
  some service in services
  name := service.metadata.name
}

matching_tags := {tag |
  some service in services
  tag := service.metadata.tags[_]
  contains(tag, input.tag_filter)
}

active_routes := [route |
  some service in services
  every endpoint in service.endpoints {
    endpoint.enabled
  }
  route := {
    "name":   service.metadata.name,
    "team":   service.metadata.team,
    "public": service.metadata.visibility == "public",
  }
]

normalized_path(path) := trimmed if {
  trimmed := trim(path, "/")
}

request_summary := {
  "actor":     input.user.name,
  "path":      normalized_path(input.request.path),
  "method":    input.request.method,
  "breakglass": user_is_break_glass,
}

deny contains msg if {
  startswith(input.request.path, "/internal")
  not input.user.role == "admin"
  msg := sprintf("path %v requires admin", [input.request.path])
}

deny contains msg if {
  some label, value in input.request.object.metadata.labels
  label == "debug"
  value == "true"
  msg := "debug workloads cannot be admitted"
}

supports_tls(service) if {
  "https" in service.protocols
}

public_service_names contains name if {
  some service in services
  service.metadata.visibility == "public"
  supports_tls(service)
  name := service.metadata.name
}

audit_event := {
  "allowed": allow,
  "deny":    [msg | msg := deny[_]],
  "warn":    [msg | msg := warn[_]],
  "team":    input.team,
  "user":    input.user.name,
}

debug_query if {
  count(allowed_roles) >= 3
  count(service_ports) > 0
  "registry.example.com/" != ""
}
