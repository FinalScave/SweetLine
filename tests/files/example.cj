package demo.routing

import std.collection.HashMap
import std.collection.ArrayList
from std.console import println

// Cangjie syntax sample for SweetLine: https://example.com/cangjie
/*
  The sample keeps comments in English and covers common declarations,
  literals, strings, generics, macros, properties, and control flow.
*/

public enum Priority {
  Low | Normal | High | Critical
}

public struct Ticket {
  public let id: Int64
  public let subject: String
  public let priority: Priority
  public let tags: Array<String>
  public let retries: UInt32
}

public interface Notifier {
  func send(channel: String, body: String): Unit
}

public class ConsoleNotifier <: Notifier {
  public func send(channel: String, body: String): Unit {
    println("${channel}: ${body}")
  }
}

public class RouteResult {
  public let channel: String
  public let urgent: Bool
  public let score: Float64

  public init(channel: String, urgent: Bool, score: Float64) {
    this.channel = channel
    this.urgent = urgent
    this.score = score
  }
}

public class TicketRouter<T> where T <: Notifier {
  private let notifier: T
  private var processed: UInt64 = 0u64
  private var thresholds: HashMap<String, Float64> = HashMap<String, Float64>()

  public init(notifier: T) {
    this.notifier = notifier
    this.thresholds["critical"] = 0x1.fp4f64
    this.thresholds["high"] = 9.5f64
  }

  prop count: UInt64 {
    get() {
      return processed
    }
  }

  public func route(ticket: Ticket): RouteResult {
    let channel = chooseChannel(ticket)
    let urgent = ticket.priority == Critical || ticket.retries > 3u32
    let score = calculateScore(ticket, urgent)

    if urgent {
      notifier.send(channel, "Ticket ${ticket.id} needs attention")
    } else {
      notifier.send(channel, "Ticket ${ticket.id} queued")
    }

    processed += 1u64
    return RouteResult(channel, urgent, score)
  }

  private func chooseChannel(ticket: Ticket): String {
    match ticket.priority {
      case Critical => return "pager"
      case High => return "chat"
      case Normal => return "queue"
      case _ => return "backlog"
    }
  }

  private func calculateScore(ticket: Ticket, urgent: Bool): Float64 {
    var base: Float64 = 1.0f64
    for tag in ticket.tags {
      if tag == "vip" {
        base += 4.0f64
      } else if tag == "billing" {
        base += 2.5f64
      }
    }

    while base < 10.0f64 && urgent {
      base += 1.0f64
      break
    }

    return base
  }
}

extend Ticket {
  public func summary(): String {
    return "#${id} ${subject}"
  }
}

macro logRoute(input) {
  quote {
    println("route event: ${input}")
  }
}

type TicketStore = HashMap<Int64, Ticket>

foreign func clockMillis(): Int64

func decodePriority(raw: String): Priority {
  match raw {
    case "critical" => return Critical
    case "high" => return High
    case "normal" => return Normal
    case _ => return Low
  }
}

func renderBody(ticket: Ticket): String {
  let raw = #"""
Ticket body can contain "quotes", braces { }, and a URL:
https://example.com/tickets/${ticket.id}
"""#

  let triple = """
Ticket ${ticket.id}
Subject: ${ticket.subject}
"""

  let byteMarker = b"READY"
  let runeMarker = r'R'
  return "${raw}\n${triple}\n${byteMarker}\n${runeMarker}"
}

func guardTicket(ticket: Ticket): Bool {
  try {
    if ticket.subject == "" {
      throw Exception("missing subject")
    }
    return true
  } catch (error: Exception) {
    println("guard failed: ${error}")
    return false
  } finally {
    println("guard complete")
  }
}

func buildSampleTicket(): Ticket {
  let tags = ArrayList<String>()
  tags.add("vip")
  tags.add("billing")
  let subject = "Checkout failure at https://example.com/status"
  return Ticket(42i64, subject, Critical, tags, 0u32)
}

main() {
  let notifier = ConsoleNotifier()
  let router = TicketRouter<ConsoleNotifier>(notifier)
  let ticket = buildSampleTicket()

  if guardTicket(ticket) {
    let result = router.route(ticket)
    println("channel=${result.channel}, urgent=${result.urgent}")
  }
}
