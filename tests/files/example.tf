terraform {
  /* Docs: https://example.com/terraform */
  required_version = ">= 1.5.0"
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }
}

provider "aws" {
  region = var.region
  default_tags { tags = { project = "sweetline", docs = "https://example.com/terraform" } }
}

variable "region" { type = string, default = "us-east-1", description = "AWS region for https://example.com/terraform" }

variable "labels" {
  type = map(string)
  default = {
    team = "platform"
    docs = "https://example.com/terraform"
  }
}

variable "environment" {
  type        = string
  default     = "dev"
  validation {
    condition     = contains(["dev", "staging", "prod"], var.environment)
    error_message = "environment must be dev, staging, or prod"
  }
}

variable "azs" { type = list(string), default = ["us-east-1a", "us-east-1b"] }

variable "extra_bucket_names" {
  type    = set(string)
  default = ["assets", "archive"]
}

locals {
  name   = "sweetline-${var.region}"
  labels = merge(var.labels, {
    env         = terraform.workspace
    environment = var.environment
  })
  normalized_bucket_names = [
    for name in var.extra_bucket_names : "${local.name}-${name}"
  ]
  selected_az = try(var.azs[0], "us-east-1a")
  service_ports = {
    http  = 80
    https = 443
  }
}

data "aws_iam_policy_document" "assume_role" {
  statement {
    actions = ["sts:AssumeRole"]
    effect  = "Allow"
    principals {
      type        = "Service"
      identifiers = ["ec2.amazonaws.com"]
    }
  }
}

module "vpc" { source = "terraform-aws-modules/vpc/aws", version = "5.5.0", for_each = toset(["a", "b"]), tags = local.labels }

resource "aws_s3_bucket" "logs" {
  bucket = local.name
  tags   = local.labels
  lifecycle { prevent_destroy = true }
}

resource "aws_s3_bucket" "extra" {
  for_each = var.extra_bucket_names
  bucket   = "${local.name}-${each.value}"
  tags = merge(local.labels, {
    kind = each.value
  })
}

data "aws_ami" "ubuntu" {
  most_recent = true
  owners      = ["099720109477"]

  filter {
    name   = "name"
    values = ["ubuntu/images/hvm-ssd/ubuntu-jammy-22.04-amd64-server-*"]
  }
}

resource "aws_instance" "web" {
  ami           = data.aws_ami.ubuntu.id
  instance_type = "t3.micro"
  tags          = local.labels
  availability_zone = local.selected_az

  user_data = <<-EOT
#!/bin/sh
echo "region=${var.region}"
echo "docs https://example.com/terraform"
%{ if var.region != "" }
echo "ready"
%{ endif }
EOT
}

resource "aws_security_group" "web" {
  name        = "${local.name}-web"
  description = "SweetLine web security group"
  vpc_id      = try(module.vpc["a"].vpc_id, null)
  tags        = local.labels

  dynamic "ingress" {
    for_each = local.service_ports
    content {
      from_port   = ingress.value
      to_port     = ingress.value
      protocol    = "tcp"
      cidr_blocks = ["0.0.0.0/0"]
      description = "Allow ${ingress.key}"
    }
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }
}

resource "aws_cloudwatch_log_group" "app" { name = "/sweetline/${var.environment}", retention_in_days = var.environment == "prod" ? 30 : 7, tags = local.labels }

output "bucket_url" { value = "https://${aws_s3_bucket.logs.bucket}.s3.amazonaws.com" }
output "instance_dns" { value = aws_instance.web.public_dns }
output "extra_buckets" { value = { for key, bucket in aws_s3_bucket.extra : key => bucket.arn } }
