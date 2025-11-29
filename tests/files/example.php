// PHP sample
<?php

declare(strict_types=1);

// Namespace and Use Statements
namespace App\Services\Payment;

use App\Models\User;
use function App\Helpers\format_currency;
use const App\Constants\TAX_RATE;

enum PaymentStatus: string
{
    case Pending = 'pending';
    case Completed = 'completed';

    public function label(): string
    {
        return match($this) {
            self::Pending => 'Payment Pending',
            self::Completed => 'Payment Completed',
        };
    }

    public function isPaid(): bool
    {
        return $this === self::Completed;
    }
}

// Interfaces
interface Refundable
{
    public function refund(float $amount): bool;
    public function getRefundableAmount(): float;
}

// Traits
trait HasTimestamps
{
    private \DateTimeImmutable $createdAt;
    private ?\DateTimeImmutable $updatedAt = null;

    public function getCreatedAt(): \DateTimeImmutable
    {
        return $this->createdAt;
    }

    public function setUpdatedAt(\DateTimeImmutable $date): void
    {
        $this->updatedAt = $date;
    }
}

trait HasUuid
{
    private string $uuid;

    public function getUuid(): string
    {
        return $this->uuid;
    }

    protected function generateUuid(): string
    {
        return sprintf(
            '%04x%04x-%04x-%04x-%04x-%04x%04x%04x',
            mt_rand(0, 0xffff), mt_rand(0, 0xffff),
            mt_rand(0, 0xffff),
            mt_rand(0, 0x0fff) | 0x4000,
            mt_rand(0, 0x3fff) | 0x8000,
            mt_rand(0, 0xffff), mt_rand(0, 0xffff), mt_rand(0, 0xffff)
        );
    }
}

// Abstract Class
abstract class BasePayment implements Loggable
{
    use HasTimestamps;
    use HasUuid;

    protected PaymentStatus $status = PaymentStatus::Pending;

    abstract protected function processPayment(float $amount): bool;

    public function getLogContext(): array
    {
        return [
            'uuid' => $this->getUuid(),
            'status' => $this->status->value,
            'created_at' => $this->createdAt->format('Y-m-d H:i:s'),
        ];
    }
}

// Attributes (PHP 8.0+)
#[Attribute(Attribute::TARGET_METHOD | Attribute::TARGET_FUNCTION)]
class Deprecated
{
    public function __construct(
        public readonly string $reason = '',
        public readonly ?string $since = null,
    ) {}
}

#[Attribute(Attribute::TARGET_PROPERTY)]
class Validate
{
    public function __construct(
        public readonly string $rule,
        public readonly ?string $message = null,
    ) {}
}

// Main Service Class
/**
 * Payment processing service.
 *
 * @author SweetLine Team
 * @since 1.0.0
 * @see PaymentGateway
 */
final class PaymentService extends BasePayment implements Refundable
{
    /** @var array<string, mixed> */
    private array $metadata = [];

    private static int $instanceCount = 0;

    public function __construct(
        private readonly PaymentGateway $gateway,
        private readonly LoggerInterface $logger,
        #[Validate(rule: 'required|numeric|min:0')]
        private float $amount = 0.0,
        private Currency $currency = Currency::USD,
    ) {
        $this->createdAt = new \DateTimeImmutable();
        $this->uuid = $this->generateUuid();
        self::$instanceCount++;
    }

    // Constructor promotion and readonly properties
    public static function getInstanceCount(): int
    {
        return self::$instanceCount;
    }

    // Method with union type and named arguments
    public function charge(
        float|int $amount,
        ?string $description = null,
        bool $capture = true,
    ): PaymentStatus {
        $this->logger->info("Processing payment", [
            'amount' => $amount,
            'currency' => $this->currency->value,
            'capture' => $capture,
        ]);

        try {
            // Null coalescing and null safe operator
            $formattedAmount = $description ?? "Payment of {$this->currency->value} {$amount}";

            // Match expression (PHP 8.0)
            $fee = match(true) {
                $amount > 10000 => $amount * 0.015,
                $amount > 1000 => $amount * 0.02,
                $amount > 100 => $amount * 0.025,
                default => $amount * 0.03,
            };

            $total = $amount + $fee;

            if ($this->processPayment($total)) {
                $this->status = PaymentStatus::Completed;
                $this->metadata['charged_at'] = date('Y-m-d H:i:s');
                $this->metadata['fee'] = $fee;

                // Null safe method call
                $userName = $this->gateway->getUser()?->getName() ?? 'Unknown';
                $this->logger->info("Payment successful for {$userName}");
            } else {
                $this->status = PaymentStatus::Failed;
                throw new PaymentException("Payment processing failed");
            }
        } catch (PaymentException $e) {
            $this->logger->error("Payment error: {$e->getMessage()}");
            throw $e;
        } catch (\Throwable $e) {
            $this->logger->error("Unexpected error: {$e->getMessage()}");
            $this->status = PaymentStatus::Failed;
            throw new PaymentException("Unexpected error", 0, $e);
        } finally {
            $this->setUpdatedAt(new \DateTimeImmutable());
        }

        return $this->status;
    }

    protected function processPayment(float $amount): bool
    {
        return $this->gateway->process($amount, $this->currency->value);
    }

    // Refundable implementation
    public function refund(float $amount): bool
    {
        if ($this->status !== PaymentStatus::Completed) {
            return false;
        }

        $refunded = $this->gateway->refund($this->getUuid(), $amount);
        if ($refunded) {
            $this->status = PaymentStatus::Refunded;
        }
        return $refunded;
    }

    public function getRefundableAmount(): float
    {
        return $this->status === PaymentStatus::Completed ? $this->amount : 0.0;
    }

    // Intersection types (PHP 8.1)
    public function setLogger(LoggerInterface&Loggable $logger): void
    {
        // Intersection type: must implement both interfaces
    }

    // First class callable syntax (PHP 8.1)
    public function getProcessor(): \Closure
    {
        return $this->processPayment(...);
    }

    // Fiber usage
    #[Deprecated(reason: 'Use async processing instead', since: '2.0')]
    public function processAsync(): mixed
    {
        $fiber = new \Fiber(function (): void {
            $result = $this->processPayment($this->amount);
            \Fiber::suspend($result);
        });

        $fiber->start();
        return $fiber->getReturn();
    }

    // Readonly properties
    public function getStatus(): PaymentStatus
    {
        return $this->status;
    }

    public function getAmount(): float
    {
        return $this->amount;
    }

    // Array and string operations
    public function getMetadata(): array
    {
        return $this->metadata;
    }
}

// Standalone Functions
function processPayments(PaymentService ...$payments): array
{
    $results = [];
    foreach ($payments as $index => $payment) {
        $results[$index] = $payment->charge(
            amount: $payment->getAmount(),
            description: "Batch payment #{$index}",
            capture: true,
        );
    }
    return $results;
}

// Arrow functions (PHP 7.4+)
$filterCompleted = fn(PaymentStatus $status): bool => $status === PaymentStatus::Completed;

$formatAmount = fn(float $amount, string $currency = 'USD'): string =>
    "{$currency} " . number_format($amount, 2);

// Numeric Literals
$integer = 42;
$negative = -17;
$float = 3.14159;
$scientific = 1.2e10;
$hex = 0xFF;
$octal = 0o777;
$binary = 0b10110;
$underscore = 1_000_000;
$floatUnderscore = 1_234.567_89;

// String Types
$singleQuoted = 'Hello, World!';
$doubleQuoted = "Hello, $singleQuoted";
$interpolated = "User {$user->name} has balance: {$user->getBalance()}";
$escaped = "Line 1\nLine 2\tTabbed\0Null";
$unicode = "\u{1F600}";

// Heredoc
$heredoc = <<<EOT
This is a heredoc string.
It supports $variable interpolation.
And {$object->property} access.
EOT;

// Nowdoc
$nowdoc = <<<'EOT'
This is a nowdoc string.
No $variable interpolation happens here.
Everything is literal: {$object->property}.
EOT;

// Control Flow
$value = 42;

// Match expression
$result = match(true) {
    $value < 0 => 'negative',
    $value === 0 => 'zero',
    $value > 0 && $value <= 100 => 'small positive',
    default => 'large positive',
};

// Named arguments
$date = new \DateTimeImmutable(
    datetime: '2024-01-15',
    timezone: new \DateTimeZone('UTC'),
);

// Null coalescing assignment
$config['timeout'] ??= 30;

// Spread operator
$merged = [...$array1, ...$array2, 'extra' => 'value'];

// Array destructuring
[$first, $second, ...$rest] = [1, 2, 3, 4, 5];
['name' => $name, 'age' => $age] = $userData;

// Generators
function fibonacci(): \Generator
{
    [$a, $b] = [0, 1];
    while (true) {
        yield $a;
        [$a, $b] = [$b, $a + $b];
    }
}

// Type checking
if ($payment instanceof Refundable) {
    $payment->refund(100.00);
}

// Static methods and constants
$count = PaymentService::getInstanceCount();
$status = PaymentStatus::from('completed');
$cases = PaymentStatus::cases();

// Array functions with closures
$amounts = [100, 250, 50, 800, 1200];
$filtered = array_filter($amounts, fn($n) => $n > 100);
$doubled = array_map(fn($n) => $n * 2, $amounts);
$total = array_reduce($amounts, fn($carry, $item) => $carry + $item, 0);

// Ternary and Elvis operator
$display = $name ?: 'Anonymous';
$checked = isset($value) ? $value : 'default';
$coalesced = $nullable ?? 'fallback';

// Magic constants
echo __FILE__ . PHP_EOL;
echo __LINE__ . PHP_EOL;
echo __CLASS__ . PHP_EOL;
echo __FUNCTION__ . PHP_EOL;
echo __METHOD__ . PHP_EOL;
echo __NAMESPACE__ . PHP_EOL;

?>
