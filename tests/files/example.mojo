# Mojo support workflow sample for SweetLine highlighting.
# The file exercises structs, traits, decorators, ownership markers, and strings.

from collections import List, Dict
from math import sqrt
from memory import UnsafePointer


alias Score = Float64
alias TicketId = String


@value
struct Customer:
    var id: String
    var name: String
    var tier: String
    var region: String

    fn __init__(out self, id: String, name: String, tier: String, region: String):
        self.id = id
        self.name = name
        self.tier = tier
        self.region = region


@value
struct Signal:
    var key: String
    var weight: Int
    var active: Bool

    fn __init__(out self, key: String, weight: Int, active: Bool):
        self.key = key
        self.weight = weight
        self.active = active


@value
struct Ticket:
    var id: TicketId
    var subject: String
    var body: String
    var customer: Customer
    var score: Score

    fn __init__(out self, id: TicketId, subject: String, body: String, customer: Customer):
        self.id = id
        self.subject = subject
        self.body = body
        self.customer = customer
        self.score = 0.0


trait Scorer:
    fn score(self, borrowed ticket: Ticket) -> Score:
        ...


@value
struct KeywordScorer:
    var urgent_weight: Score
    var billing_weight: Score

    fn __init__(out self, urgent_weight: Score, billing_weight: Score):
        self.urgent_weight = urgent_weight
        self.billing_weight = billing_weight

    fn score(self, borrowed ticket: Ticket) -> Score:
        var value: Score = 0.0
        if "urgent" in ticket.body:
            value += self.urgent_weight
        if "billing" in ticket.body:
            value += self.billing_weight
        if ticket.customer.tier == "enterprise":
            value += 4.0
        return value


fn clamp(value: Score, low: Score, high: Score) -> Score:
    if value < low:
        return low
    elif value > high:
        return high
    else:
        return value


fn priority_label(score: Score) -> String:
    if score >= 12.0:
        return "critical"
    if score >= 7.0:
        return "high"
    if score >= 3.0:
        return "medium"
    return "low"


fn choose_team(ticket: Ticket, score: Score) -> String:
    if score >= 12.0 and ticket.customer.region == "eu":
        return "emea-incident"
    if score >= 12.0:
        return "global-incident"
    if ticket.customer.tier == "enterprise":
        return "customer-success"
    return "support-routing"


fn normalize_subject(subject: String) -> String:
    return subject.strip().replace("\n", " ")


fn render_summary(ticket: Ticket, score: Score) -> String:
    let label = priority_label(score)
    let team = choose_team(ticket, score)
    return f"{ticket.id}: {label} for {ticket.customer.name} routed to {team}"


fn distance(a: SIMD[DType.float64, 2], b: SIMD[DType.float64, 2]) -> Float64:
    let dx = b[0] - a[0]
    let dy = b[1] - a[1]
    return sqrt(dx * dx + dy * dy)


fn accumulate_signals(signals: List[Signal]) -> Int:
    var total = 0
    for signal in signals:
        if signal.active:
            total += signal.weight
    return total


async fn enrich_ticket(mut ticket: Ticket, scorer: KeywordScorer) raises -> Ticket:
    let base = scorer.score(ticket)
    ticket.score = clamp(base, 0.0, 20.0)
    return ticket


fn build_sample_ticket() -> Ticket:
    let customer = Customer(
        "cus_42",
        "Ada Lovelace",
        "enterprise",
        "eu",
    )
    let body = """
    Customer reports an urgent billing failure.
    Diagnostic link: https://example.com/billing-error
    """
    return Ticket("ticket_100", "Renewal failed", body, customer)


fn render_policy_context(topic: String, limit: Int = 3) -> String:
    var lines = List[String]()
    for index in range(limit):
        lines.append(f"- {topic} policy excerpt {index}")
    return "\n".join(lines)


@staticmethod
fn debug_pointer(ptr: UnsafePointer[Int]) -> String:
    if ptr == UnsafePointer[Int]():
        return "null"
    return "set"


fn main():
    let ticket = build_sample_ticket()
    let scorer = KeywordScorer(8.0, 5.0)
    let score = scorer.score(ticket)
    let summary = render_summary(ticket, score)
    let context = render_policy_context("refund")
    print(summary)
    print(context)
