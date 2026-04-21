package demo

import "strings"
import "list"

/* SweetLine CUE sample: definitions, constraints,
   comprehensions, interpolation, and dynamic fields. */

#Port: int & >=1 & <=65535
#Tier: "edge" | "api" | "worker"
#LabelKey: =~"^[a-z0-9-]+$"

#Service: {
	name:      string
	tier!:     #Tier
	replicas?: int & >=1
	ports:     [...#Port]
	labels?:   [#LabelKey]: string
	owners:    [...string]
}

env:     "prod"
cluster: "shanghai"
domain:  "svc.example.com"

if env == "prod" {
	minimumReplicas: 3
	rolloutWindow!:  "09:00-11:00"
}

services: [
	#Service & {
		name:     "gateway"
		tier:     "edge"
		replicas: 4
		ports:    [80, 443]
		labels: {
			team:  "web"
			track: "stable"
		}
		owners: ["ava", "milo"]
	},
	#Service & {
		name:  "catalog"
		tier:  "api"
		ports: [8080]
		labels: {
			team: "commerce"
		}
		owners: ["noah"]
	},
	#Service & {
		name:     "jobs"
		tier:     "worker"
		replicas: 2
		ports:    [9090]
		owners:   ["zoe", "emma"]
	},
]

quotedFields: {
	"x-trace-id": string
	"x-team":     strings.ToUpper(cluster)
}

serviceIndex: {
	for idx, svc in services
	let lower = strings.ToLower(svc.name) {
		"\(lower)": {
			order:    idx + 1
			hostname: "\(svc.name).\(cluster).\(domain)"
			display:  "\(strings.ToUpper(svc.name))/\(env)"
			ports:    svc.ports
			tier:     svc.tier
		}
	}
}

portMatrix: [
	for svc in services
	for port in svc.ports {
		{
			service: svc.name
			address: "\(svc.name):\(port)"
			public:  port == 80 || port == 443
		}
	},
]

apiServices: [
	for svc in services
	if svc.tier == "api" {
		svc.name
	},
]

summary: {
	count: len(services)
	tiers: list.SortStrings([for svc in services {svc.tier}])
	"\(cluster)-summary": {
		names:     [for svc in services {svc.name}]
		apiOnly:   apiServices
		firstHost: serviceIndex.gateway.hostname
	}
}

constraints: {
	[string]: {
		enabled: bool
		owner?:  string
	}
}

constraints: base: {
	enabled: true
	owner:   "platform"
}

deployment: {
	for _, svc in services {
		(svc.name): {
			image:    "registry.example.com/\(svc.name):v1"
			replicas: svc.replicas | *minimumReplicas
			ready:    svc.tier != "worker"
		}
	}
}

featureFlags: {
	canary?: bool
	debug?:  _|_
}

#Release: {
	version: =~"^v[0-9]+\\.[0-9]+\\.[0-9]+$"
	notes:   string
}

release: #Release & {
	version: "v1.7.2"
	notes:   "CUE example for SweetLine."
}
