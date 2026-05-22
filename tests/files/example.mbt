/// Support triage domain sample for MoonBit highlighting.
/// It exercises records, enums, traits, generics, package calls, and comments.

pub enum Tier {
  Free
  Team
  Enterprise
}

pub enum Priority {
  Low
  Medium
  High
  Critical
}

pub enum Maybe[T] {
  Some(T)
  None
}

pub struct Customer {
  id: String
  name: String
  tier: Tier
  region: String
}

pub struct Signal {
  key: String
  weight: Int
  active: Bool
}

pub struct Ticket {
  id: String
  subject: String
  body: String
  customer: Customer
  signals: Array[Signal]
  created_at: Int64
}

pub struct RoutingDecision {
  priority: Priority
  team: String
  reason: String
  score: Double
}

pub trait Encoder {
  encode(Self) -> String
}

pub fn clamp(value: Int, low: Int, high: Int) -> Int {
  if value < low {
    low
  } else if value > high {
    high
  } else {
    value
  }
}

pub fn tier_weight(tier: Tier) -> Int {
  match tier {
    Free => 1
    Team => 3
    Enterprise => 8
  }
}

pub fn priority_label(priority: Priority) -> String {
  match priority {
    Low => "low"
    Medium => "medium"
    High => "high"
    Critical => "critical"
  }
}

pub fn normalize_subject(subject: String) -> String {
  subject.trim().replace("\n", " ")
}

pub fn signal_weight(signal: Signal) -> Int {
  if signal.active {
    signal.weight
  } else {
    0
  }
}

pub fn total_signal_weight(signals: Array[Signal]) -> Int {
  let mut total = 0
  for signal in signals {
    total = total + signal_weight(signal)
  }
  total
}

pub fn infer_priority(ticket: Ticket) -> Priority {
  let score = total_signal_weight(ticket.signals) + tier_weight(ticket.customer.tier)
  if score >= 16 {
    Critical
  } else if score >= 10 {
    High
  } else if score >= 5 {
    Medium
  } else {
    Low
  }
}

pub fn choose_team(ticket: Ticket, priority: Priority) -> String {
  match (ticket.customer.region, priority) {
    ("eu", Critical) => "emea-incident"
    ("us", Critical) => "amer-incident"
    ("apac", Critical) => "apac-incident"
    (_, High) => "customer-success"
    (_, Medium) => "support-routing"
    _ => "general-support"
  }
}

pub fn build_reason(ticket: Ticket, priority: Priority) -> String {
  let subject = normalize_subject(ticket.subject)
  let tier = ticket.customer.tier
  "priority \(priority_label(priority)) for \(ticket.customer.name) in \(tier): \(subject)"
}

pub fn route(ticket: Ticket) -> RoutingDecision {
  let priority = infer_priority(ticket)
  let team = choose_team(ticket, priority)
  let reason = build_reason(ticket, priority)
  let score = total_signal_weight(ticket.signals).to_double()
  RoutingDecision::{ priority, team, reason, score }
}

pub fn encode_signal(signal: Signal) -> String {
  if signal.active {
    "\(signal.key)=\(signal.weight)"
  } else {
    "\(signal.key)=inactive"
  }
}

pub fn encode_decision(decision: RoutingDecision) -> String {
  let label = priority_label(decision.priority)
  "{ \"priority\": \"\(label)\", \"team\": \"\(decision.team)\", \"score\": \(decision.score) }"
}

pub fn sample_ticket() -> Ticket {
  let customer = Customer::{
    id: "cus_42",
    name: "Ada Lovelace",
    tier: Enterprise,
    region: "eu",
  }
  let signals = [
    Signal::{ key: "refund", weight: 6, active: true },
    Signal::{ key: "outage", weight: 8, active: true },
    Signal::{ key: "duplicate", weight: 2, active: false },
  ]
  Ticket::{
    id: "ticket_100",
    subject: "Payment failed during renewal",
    body: "Customer sees https://example.com/billing-error in the console.",
    customer,
    signals,
    created_at: 1717000000L,
  }
}

pub fn render_report(ticket: Ticket) -> String {
  let decision = route(ticket)
  let payload = encode_decision(decision)
  @json.inspect(payload)
}

pub fn maybe_team(priority: Priority) -> Maybe[String] {
  match priority {
    Critical => Some("incident")
    High => Some("customer-success")
    _ => None
  }
}

test "routing escalates enterprise incident" {
  let ticket = sample_ticket()
  let decision = route(ticket)
  inspect(decision.priority, content="Critical")
  inspect(decision.team, content="emea-incident")
}

pub fn main {
  let ticket = sample_ticket()
  let report = render_report(ticket)
  println(report)
}
