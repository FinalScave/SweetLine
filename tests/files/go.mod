module example.com/platform/orchestrator

go 1.22.3

toolchain go1.22.5

godebug x509sha1=1

tool example.com/platform/tools/schema-sync
tool example.com/platform/tools/release-lint

require (
	// Cloud and edge runtimes
	cloud.google.com/go/pubsub v1.42.0
	cloud.google.com/go/secretmanager v1.13.3
	cloud.google.com/go/storage v1.43.0
	github.com/Azure/azure-sdk-for-go/sdk/azcore v1.14.0
	github.com/Azure/azure-sdk-for-go/sdk/storage/azblob v1.4.0
	github.com/aws/aws-sdk-go-v2 v1.31.0
	github.com/aws/aws-sdk-go-v2/config v1.27.36
	github.com/aws/aws-sdk-go-v2/service/s3 v1.61.2
	github.com/go-resty/resty/v2 v2.14.0
	golang.org/x/time v0.6.0

	// Data and storage drivers
	github.com/Masterminds/squirrel v1.5.4
	github.com/go-sql-driver/mysql v1.8.1
	github.com/jackc/pgx/v5 v5.7.1
	github.com/jmoiron/sqlx v1.4.0
	github.com/lib/pq v1.10.9
	github.com/minio/minio-go/v7 v7.0.78
	github.com/pelletier/go-toml/v2 v2.2.3
	github.com/redis/go-redis/v9 v9.6.1
	go.mongodb.org/mongo-driver v1.17.1
	modernc.org/sqlite v1.33.1

	// Messaging and workflow
	github.com/IBM/sarama v1.43.3
	github.com/ThreeDotsLabs/watermill v1.3.7
	github.com/ThreeDotsLabs/watermill-kafka/v2 v2.2.1
	github.com/cenkalti/backoff/v4 v4.3.0
	github.com/hashicorp/raft v1.7.1
	github.com/nats-io/nats.go v1.37.0
	github.com/oklog/ulid/v2 v2.1.0
	github.com/rabbitmq/amqp091-go v1.10.0
	github.com/robfig/cron/v3 v3.0.1
	github.com/sethvargo/go-retry v0.3.0

	// Observability and metrics
	github.com/grafana/regexp v0.0.0-20240518133315-a468a5bfb3bc
	github.com/prometheus/client_golang v1.20.4
	github.com/prometheus/common v0.60.1
	go.opentelemetry.io/contrib/instrumentation/net/http/otelhttp v0.56.0
	go.opentelemetry.io/otel v1.31.0
	go.opentelemetry.io/otel/exporters/otlp/otlptrace/otlptracegrpc v1.31.0
	go.opentelemetry.io/otel/sdk v1.31.0
	go.opentelemetry.io/otel/trace v1.31.0
	go.uber.org/zap v1.27.0
	gopkg.in/natefinch/lumberjack.v2 v2.2.1

	// API and transport layers
	github.com/99designs/gqlgen v0.17.58
	github.com/getkin/kin-openapi v0.128.0
	github.com/gin-gonic/gin v1.10.0
	github.com/go-chi/chi/v5 v5.1.0
	github.com/go-playground/validator/v10 v10.22.1
	github.com/golang-jwt/jwt/v5 v5.2.1
	github.com/google/uuid v1.6.0
	github.com/grpc-ecosystem/grpc-gateway/v2 v2.22.0
	github.com/labstack/echo/v4 v4.12.0
	github.com/rs/cors v1.11.1

	// Platform auth and policy
	github.com/coreos/go-oidc/v3/oidc v3.11.0
	github.com/hashicorp/consul/api v1.30.0
	github.com/hashicorp/memberlist v0.5.1
	github.com/hashicorp/vault/api v1.15.0
	github.com/joho/godotenv v1.5.1
	github.com/lestrrat-go/jwx/v2 v2.1.1
	github.com/markbates/goth v1.80.0
	github.com/open-policy-agent/opa v0.68.0
	github.com/xeipuuv/gojsonschema v1.2.0
	golang.org/x/oauth2 v0.23.0

	// CLI and operator surfaces
	github.com/alecthomas/kingpin/v2 v2.4.0
	github.com/charmbracelet/bubbles v0.20.0
	github.com/charmbracelet/bubbletea v1.1.1
	github.com/charmbracelet/lipgloss v0.13.0
	github.com/fatih/color v1.17.0
	github.com/fsnotify/fsnotify v1.8.0
	github.com/muesli/termenv v0.15.2
	github.com/spf13/afero v1.11.0
	github.com/spf13/cobra v1.8.1
	github.com/spf13/viper v1.19.0

	// Schema, config, and external integrations
	github.com/BurntSushi/toml v1.4.0
	github.com/google/go-github/v61 v61.0.0
	github.com/hashicorp/go-multierror v1.1.1
	github.com/hashicorp/go-retryablehttp v0.7.7
	github.com/hashicorp/hcl/v2 v2.22.0
	github.com/mitchellh/mapstructure v1.5.0
	github.com/sendgrid/sendgrid-go v3.16.0+incompatible
	github.com/slack-go/slack v0.14.0
	github.com/stripe/stripe-go/v79 v79.12.0
	github.com/swaggo/swag v1.16.4

	// Shared transitive dependencies
	github.com/golang/mock v1.6.0 // indirect
	github.com/google/go-cmp v0.6.0 // indirect
	github.com/onsi/gomega v1.34.2 // indirect
	github.com/stretchr/testify v1.9.0 // indirect
	github.com/tidwall/gjson v1.18.0 // indirect
	github.com/tidwall/sjson v1.2.5 // indirect
	github.com/vektah/gqlparser/v2 v2.5.18 // indirect
	golang.org/x/crypto v0.28.0 // indirect
	golang.org/x/net v0.30.0 // indirect
	golang.org/x/sync v0.8.0 // indirect
	golang.org/x/sys v0.26.0 // indirect
	golang.org/x/text v0.19.0 // indirect
	google.golang.org/api v0.203.0 // indirect
	google.golang.org/grpc v1.67.1 // indirect
	google.golang.org/protobuf v1.35.1 // indirect
	gopkg.in/yaml.v3 v3.0.1 // indirect
)

replace (
	example.com/platform/common => ../common
	example.com/platform/identity => ../domains/identity
	example.com/platform/jobs => ../domains/jobs
	example.com/platform/queue => ../domains/queue
	example.com/platform/reporting => ../domains/reporting
	example.com/platform/secrets => ../domains/secrets
	example.com/platform/ui-shared => ../packages/ui-shared
	github.com/hashicorp/memberlist => ../third_party/memberlist
	github.com/labstack/echo/v4 => github.com/labstack/echo/v4 v4.12.0
	github.com/segmentio/kafka-go => github.com/example-forks/kafka-go v0.4.47-platform.1
	google.golang.org/grpc => google.golang.org/grpc v1.67.1
)

exclude (
	github.com/prometheus/client_golang v1.18.0
	github.com/spf13/viper v1.17.0
	google.golang.org/grpc v1.64.0
)

retract (
	v0.9.0
	[v0.8.0, v0.8.2]
)
