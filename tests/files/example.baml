// Shared client for inexpensive structured extraction.
client<llm> FastOpenAI {
  provider "openai"
  options {
    model "gpt-5-mini"
    api_key env.OPENAI_API_KEY
    temperature 0.2
    max_tokens 1200
  }
}

// Backup client for long reasoning prompts.
client<llm> CarefulAnthropic {
  provider "anthropic"
  options {
    model "claude-sonnet-4-20250514"
    api_key env.ANTHROPIC_API_KEY
    temperature 0.1
  }
}

retry_policy ShortRetry {
  max_retries 2
  strategy "exponential_backoff"
}

generator client {
  output_type "typescript"
  default_client_mode "async"
  version "0.217.0"
}

enum Tone {
  Friendly @description("Warm, concise, and direct")
  Formal @description("Precise language for business records")
  Urgent @description("Short language for active incidents")
}

enum Priority {
  Low
  Medium
  High
  Critical

  @@description("Support priority inferred from the customer text")
}

class Customer {
  name string @description("Visible customer name")
  account_id string @alias("account")
  tier string?
  tags string[]
}

class SupportTicket {
  subject string
  body string
  customer Customer
  attachments image[]?
  locale string? @description("IETF language tag when known")
}

class RoutingDecision {
  priority Priority
  team string
  reason string
  confidence float
}

class ReplyDraft {
  subject string
  body string
  tone Tone
  follow_up_questions string[]
}

class SafetyFinding {
  label string
  severity int
  explanation string
}

class TicketSummary {
  title string
  bullets string[]
  metadata map<string, string>
}

function ClassifyTicket(ticket: SupportTicket) -> RoutingDecision {
  client FastOpenAI
  prompt #"
    {{ _.role("system") }}
    You route support requests for a developer platform.
    Return the smallest valid object that satisfies the schema.

    {{ ctx.output_format }}

    {{ _.role("user") }}
    Subject: {{ ticket.subject }}
    Customer tier: {{ ticket.customer.tier or "unknown" }}

    Body:
    {{ ticket.body }}

    {% if ticket.attachments %}
    The customer attached {{ ticket.attachments | length }} image files.
    {% endif %}
  "#
}

function DraftReply(ticket: SupportTicket, decision: RoutingDecision, tone: Tone) -> ReplyDraft {
  client CarefulAnthropic
  prompt #"
    {{ _.role("system") }}
    Write a support reply that matches the requested tone.
    Keep the response useful without promising work that has not been scheduled.

    {{ ctx.output_format }}

    {% set owner = decision.team %}
    {{ _.role("user") }}
    Priority: {{ decision.priority }}
    Team: {{ owner }}
    Tone: {{ tone }}

    Customer message:
    {{ ticket.body }}

    {% for tag in ticket.customer.tags %}
    Customer tag: {{ tag }}
    {% endfor %}
  "#
}

function CheckSafety(text: string) -> SafetyFinding[] {
  client "openai-responses/gpt-5-mini"
  prompt #"
    {# The output must stay machine readable for automation. #}
    {{ _.role("system") }}
    Identify policy or safety concerns in the text.
    If nothing is concerning, return an empty array.

    {{ ctx.output_format }}

    {{ _.role("user") }}
    {{ text }}
  "#
}

function SummarizeTicket(ticket: SupportTicket) -> TicketSummary {
  client FastOpenAI
  prompt #"
    {{ _.role("system") }}
    Summarize the ticket for an engineer who will triage it.
    Prefer concrete nouns over broad categories.

    {{ ctx.output_format }}

    {{ _.role("user") }}
    Account: {{ ticket.customer.account_id }}
    Subject: {{ ticket.subject }}
    Body: {{ ticket.body }}
  "#
}

function BuildIncidentNote(ticket: SupportTicket, findings: SafetyFinding[]) -> string {
  client FastOpenAI
  prompt #"
    {{ _.role("system") }}
    Produce a plain text note for the incident channel.

    {{ _.role("user") }}
    Ticket: {{ ticket.subject }}

    {% if findings %}
    Findings:
    {% for finding in findings %}
    - {{ finding.label }} at severity {{ finding.severity }}: {{ finding.explanation }}
    {% endfor %}
    {% else %}
    No safety findings were detected.
    {% endif %}
  "#
}

test ClassifyTicketSmoke {
  functions [ClassifyTicket]
  args {
    ticket {
      subject "Builds fail after rotating credentials"
      body "Our deployment started failing after we changed the token yesterday."
      customer {
        name "Northwind Apps"
        account_id "acct_2468"
        tier "enterprise"
        tags ["ci", "deployments"]
      }
      locale "en-US"
    }
  }
}

test DraftReplySmoke {
  functions [DraftReply]
  args {
    tone Formal
    decision {
      priority High
      team "platform-support"
      reason "Deployment is blocked for an enterprise account"
      confidence 0.82
    }
    ticket {
      subject "Webhook retries are delayed"
      body "Retries now arrive after twenty minutes instead of two."
      customer {
        name "Atlas Labs"
        account_id "acct_1357"
        tier "business"
        tags ["webhooks", "latency"]
      }
    }
  }
}
