package com.qiplat.sweetline.demo.scala
import scala.annotation.{tailrec, targetName, unused}
import scala.collection.mutable.{Map as MutableMap}
import scala.concurrent.{ExecutionContext, Future}
import scala.util.{Failure, Success, Try}
import java.time.{Instant, ZoneId}
import java.time.format.DateTimeFormatter
/* Demo covers package/import/export, opaque type, enum, given/using, extension, match/case,
   raw/interpolated/triple strings, annotations, generic types, lazy vals and async flow. */
opaque type UserId = Long
object UserId:
  def apply(value: Long): UserId = value
  extension (id: UserId)
    def value: Long = id
    def render: String = s"user-${id.value}"
enum Permission derives CanEqual:
  case Read, Write, Delete, Admin
  case Custom(name: String)
sealed trait DomainError extends Product with Serializable:
  def message: String
object DomainError:
  final case class Validation(message: String) extends DomainError
  final case class NotFound(id: UserId) extends DomainError:
    def message: String = s"User ${id.render} was not found"
  final case class External(service: String, reason: String) extends DomainError:
    def message: String = s"$service failed: $reason"
type JsonObject = Map[String, String]
type Scalar = Int | Long | Double | BigDecimal
type AuditLog = ExecutionContext ?=> String => Unit
trait Clock: def now(): Instant
object Clock:
  given systemClock: Clock with
    def now(): Instant = Instant.parse("2024-01-20T09:30:00Z")
given Conversion[UserId, Long] with
  def apply(id: UserId): Long = id.value
trait JsonEncoder[-A]:
  def encode(value: A): String
object JsonEncoder:
  given JsonEncoder[String] with
    def encode(value: String): String = "\"" + value.replace("\"", "\\\"") + "\""
  given JsonEncoder[Long] with
    def encode(value: Long): String = value.toString
  given JsonEncoder[Boolean] with
    def encode(value: Boolean): String = value.toString
  given [A](using enc: JsonEncoder[A]): JsonEncoder[Option[A]] with
    def encode(value: Option[A]): String = value match
      case Some(inner) => enc.encode(inner)
      case None        => "null"
  def apply[A](using instance: JsonEncoder[A]): JsonEncoder[A] = instance
final case class Money(amount: BigDecimal, currency: String):
  @targetName("plusMoney")
  def +(other: Money): Money =
    require(currency == other.currency, "Currencies must match")
    copy(amount = amount + other.amount)
  def formatted: String = f"$amount%.2f $currency"
final case class Address(line1: String, city: String, country: String, postalCode: String)
final case class User(
  id: UserId, email: String, name: String, permissions: Set[Permission],
  address: Address, createdAt: Instant, balance: Money, tags: List[String]
)
trait UserRepositoryAlg:
  def upsert(user: User): Unit
  def findById(id: UserId): Option[User]
  def allUsers(): List[User]
final class InMemoryUserRepository private (private val store: MutableMap[UserId, User]) extends UserRepositoryAlg:
  def upsert(user: User): Unit = store.update(user.id, user)
  def findById(id: UserId): Option[User] = store.get(id)
  def allUsers(): List[User] = store.values.toList.sortBy(_.name.toLowerCase)
  def groupedByCity: Map[String, List[User]] = allUsers().groupBy(_.address.city)
object InMemoryUserRepository:
  def empty(): InMemoryUserRepository = new InMemoryUserRepository(MutableMap.empty)
given Ordering[User] with
  def compare(left: User, right: User): Int =
    Ordering[String].compare(left.name, right.name)
extension (user: User)
  def displayName: String = s"${user.name} <${user.email}>"
  def can(permission: Permission): Boolean =
    permission match
      case Permission.Read | Permission.Write =>
        user.permissions.contains(permission) || user.permissions.contains(Permission.Admin)
      case Permission.Delete =>
        user.permissions.contains(Permission.Delete) || user.permissions.contains(Permission.Admin)
      case Permission.Admin => user.permissions.contains(Permission.Admin)
      case Permission.Custom(name) =>
        user.permissions.exists {
          case Permission.Custom(value) => value.equalsIgnoreCase(name)
          case _                        => false
        }
extension [A](values: List[A])
  def foldMap[B](zero: B)(f: A => B)(combine: (B, B) => B): B =
    values.foldLeft(zero)((acc, next) => combine(acc, f(next)))
  def renderLines(using show: A => String): String = values.map(show).mkString("\n")
extension [A: JsonEncoder](value: A)
  def toJson: String = summon[JsonEncoder[A]].encode(value)
inline def timed[A](label: String)(block: => A): A =
  val started = System.nanoTime()
  val result = block
  val elapsedMs = (System.nanoTime() - started) / 1000000.0
  println(f"$label completed in $elapsedMs%.2f ms")
  result
def parseUserId(raw: String): Either[DomainError, UserId] =
  Try(raw.trim.toLong) match
    case Success(value) if value > 0L => Right(UserId(value))
    case Success(_)                   => Left(DomainError.Validation("Identifier must be positive"))
    case Failure(_)                   => Left(DomainError.Validation(s"Invalid identifier: $raw"))
def renderUserAsJson(user: User): String =
  val permissionJson = user.permissions.toList.map {
    case Permission.Custom(name) => JsonEncoder[String].encode(name)
    case other                   => JsonEncoder[String].encode(other.toString)
  }
  s"""
     |{
     |  "id": ${user.id.value},
     |  "email": ${user.email.toJson},
     |  "name": ${user.name.toJson},
     |  "permissions": [${permissionJson.mkString(", ")}],
     |  "city": ${user.address.city.toJson},
     |  "tags": [${user.tags.map(_.toJson).mkString(", ")}]
     |}
     |""".stripMargin
given JsonEncoder[User] with
  def encode(user: User): String = renderUserAsJson(user)
final class UserService(
  repo: UserRepositoryAlg,
  audit: String => Unit = msg => println(s"[audit] $msg")
)(using ec: ExecutionContext, clock: Clock):
  export repo.allUsers
  private lazy val formatter =
    DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss").withZone(ZoneId.of("UTC"))
  def register(
    email: String, name: String, city: String, country: String, postalCode: String,
    initial: Money = Money(BigDecimal(0), "USD")
  ): Either[DomainError, User] =
    if !email.contains("@") then Left(DomainError.Validation("Email must contain @"))
    else if name.trim.isEmpty then Left(DomainError.Validation("Name cannot be blank"))
    else
      val user = User(
        id = UserId(math.abs(email.hashCode.toLong) + 1L),
        email = email,
        name = name,
        permissions = Set(Permission.Read),
        address = Address("unknown", city, country, postalCode),
        createdAt = clock.now(),
        balance = initial,
        tags = List("new", "trial")
      )
      repo.upsert(user)
      audit(s"registered ${user.displayName}")
      Right(user)
  def activate(id: UserId, permission: Permission): Either[DomainError, User] =
    repo.findById(id) match
      case Some(user) =>
        val updated = user.copy(permissions = user.permissions + permission)
        repo.upsert(updated)
        audit(s"activated ${permission.toString} for ${updated.displayName}")
        Right(updated)
      case None => Left(DomainError.NotFound(id))
  def fetchJson(id: UserId): Either[DomainError, String] =
    repo.findById(id) match
      case Some(user) => Right(user.toJson)
      case None       => Left(DomainError.NotFound(id))
  def cityReport(): String =
    repo.allUsers().groupBy(_.address.city).toList.sortBy(_._1)
      .map { case (city, users) => s"$city => ${users.map(_.name).sorted.mkString(", ")}" }
      .mkString("\n")
  def riskyComputation(id: UserId): Future[Either[DomainError, String]] =
    Future {
      repo.findById(id) match
        case Some(user) if user.can(Permission.Read) =>
          Right(s"loaded:${user.email}:${formatter.format(user.createdAt)}")
        case Some(_)  => Left(DomainError.Validation("Permission denied"))
        case None     => Left(DomainError.NotFound(id))
    }
object Samples:
  val rawPattern = raw"""(\d{4})-(\d{2})-(\d{2})"""
  val newline = '\n'
  val `type` = "escaped-identifier"
  val banner =
    s"""|Scala Highlight Demo
        |====================
        |marker = `${`type`}`
        |newline = $newline
        |pattern = $rawPattern
        |""".stripMargin
  val sampleJson: JsonObject = Map("language" -> "scala", "mode" -> "demo", "version" -> "3.x")
  def installAudit(log: AuditLog): Unit = ()
  def sampleUsers(now: Instant): List[User] = List(
    User(
      id = UserId(1L), email = "ada@example.com", name = "Ada Lovelace",
      permissions = Set(Permission.Read, Permission.Write, Permission.Custom("metrics")),
      address = Address("1 Analytical Engine Way", "London", "UK", "EC1A 1AA"),
      createdAt = now, balance = Money(BigDecimal(1250.50), "USD"), tags = List("math", "history")
    ),
    User(
      id = UserId(2L), email = "grace@example.com", name = "Grace Hopper",
      permissions = Set(Permission.Read, Permission.Admin),
      address = Address("44 Compiler Ave", "Arlington", "US", "22201"),
      createdAt = now.minusSeconds(3600L), balance = Money(BigDecimal(9800.00), "USD"), tags = List("navy", "compiler")
    ),
    User(
      id = UserId(3L), email = "linus@example.com", name = "Linus",
      permissions = Set(Permission.Read),
      address = Address("7 Kernel Road", "Helsinki", "FI", "00100"),
      createdAt = now.minusSeconds(7200L), balance = Money(BigDecimal(42.42), "USD"), tags = List("kernel")
    )
  )
  @tailrec
  def sumBalances(users: List[User], acc: BigDecimal = BigDecimal(0)): BigDecimal =
    users match
      case head :: tail => sumBalances(tail, acc + head.balance.amount)
      case Nil          => acc
  def debug(@unused label: String): Unit = ()
@main def runScalaHighlightDemo(): Unit =
  given ExecutionContext = ExecutionContext.global
  val repo = InMemoryUserRepository.empty()
  val service = new UserService(repo)
  timed("seed-users") { Samples.sampleUsers(summon[Clock].now()).foreach(repo.upsert) }
  println(Samples.banner)
  println(Samples.sampleJson.map { case (k, v) => s"$k=$v" }.mkString(", "))
  println(service.cityReport())
  for
    (city, users) <- repo.groupedByCity.toList.sortBy(_._1)
    user <- users
  do println(s"[$city] ${user.displayName}")
  val activation =
    parseUserId("2").flatMap(id => service.activate(id, Permission.Delete).map(_.displayName))
  activation match
    case Right(name) => println(s"Activated permissions for $name")
    case Left(error) => println(s"Could not activate user: ${error.message}")
  val premiumUsers =
    service.allUsers().filter(_.balance.amount >= BigDecimal(1000))
      .foldMap(List.empty[String])(user => List(user.displayName))(_ ++ _)
  println(premiumUsers.mkString("Premium users => ", ", ", ""))
  println(s"Total balance => ${Samples.sumBalances(service.allUsers())}")
  service.riskyComputation(UserId(1L)).foreach {
    case Right(value) => println(s"Async payload: $value")
    case Left(error)  => println(s"Async error: ${error.message}")
  }
  try
    val forcedId = parseUserId("oops") match
      case Right(value) => value
      case Left(error)  => throw new IllegalArgumentException(error.message)
    println(service.fetchJson(forcedId))
  catch
    case ex: IllegalArgumentException =>
      println(s"Recovered from parse failure: ${ex.getMessage}")
  finally
    println(raw"Regex sample => ${Samples.rawPattern}")
  Samples.debug("done")
end runScalaHighlightDemo
