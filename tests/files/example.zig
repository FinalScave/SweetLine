// Zig sample program
// This demonstrates various Zig features

const std = @import("std");
const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;

// Constants
const PI = 3.14159265359;
const MAX_SIZE: usize = 1024;

// Custom error set
const MyError = error{
    OutOfMemory,
    InvalidInput,
    NotFound,
};

// Struct definition
const Point = struct {
    x: f64,
    y: f64,

    // Method
    pub fn distance(self: Point, other: Point) f64 {
        const dx = self.x - other.x;
        const dy = self.y - other.y;
        return @sqrt(dx * dx + dy * dy);
    }

    // Constructor-like function
    pub fn init(x: f64, y: f64) Point {
        return Point{ .x = x, .y = y };
    }
};

// Enum definition
const Color = enum {
    red,
    green,
    blue,

    pub fn toRgb(self: Color) [3]u8 {
        return switch (self) {
            .red => [_]u8{ 255, 0, 0 },
            .green => [_]u8{ 0, 255, 0 },
            .blue => [_]u8{ 0, 0, 255 },
        };
    }
};

// Union definition
const Value = union(enum) {
    int: i64,
    float: f64,
    string: []const u8,

    pub fn print(self: Value) void {
        switch (self) {
            .int => |i| std.debug.print("int: {}\n", .{i}),
            .float => |f| std.debug.print("float: {}\n", .{f}),
            .string => |s| std.debug.print("string: {s}\n", .{s}),
        }
    }
};

// Generic function
fn max(comptime T: type, a: T, b: T) T {
    return if (a > b) a else b;
}

// Error handling with try
fn divide(a: f64, b: f64) MyError!f64 {
    if (b == 0) {
        return MyError.InvalidInput;
    }
    return a / b;
}

// Async function
fn asyncOperation() !void {
    const frame = async asyncHelper();
    const result = await frame;
    std.debug.print("Result: {}\n", .{result});
}

fn asyncHelper() i32 {
    return 42;
}

// Main function
pub fn main() !void {
    // Print to stdout
    const stdout = std.io.getStdOut().writer();
    try stdout.print("Hello, Zig!\n", .{});

    // Variables
    var x: i32 = 42;
    const y: i32 = 100;
    x += 10;

    // Pointers
    var ptr: *i32 = &x;
    ptr.* = 50;

    // Arrays
    const arr = [_]i32{ 1, 2, 3, 4, 5 };
    var sum: i32 = 0;
    for (arr) |item| {
        sum += item;
    }

    // Slices
    const slice = arr[1..4];
    for (slice, 0..) |item, index| {
        try stdout.print("slice[{}] = {}\n", .{ index, item });
    }

    // Struct usage
    const p1 = Point.init(0.0, 0.0);
    const p2 = Point{ .x = 3.0, .y = 4.0 };
    const dist = p1.distance(p2);
    try stdout.print("Distance: {}\n", .{dist});

    // Enum usage
    const color = Color.red;
    const rgb = color.toRgb();
    try stdout.print("RGB: {}\n", .{rgb});

    // Union usage
    const v1 = Value{ .int = 42 };
    const v2 = Value{ .float = 3.14 };
    const v3 = Value{ .string = "hello" };
    v1.print();
    v2.print();
    v3.print();

    // Error handling
    const result = divide(10.0, 2.0) catch |err| {
        try stdout.print("Error: {}\n", .{err});
        return;
    };
    try stdout.print("Result: {}\n", .{result});

    // Optionals
    var maybe: ?i32 = null;
    maybe = 42;
    if (maybe) |value| {
        try stdout.print("Value: {}\n", .{value});
    }

    // Error unions
    const error_union: MyError!i32 = 42;
    const unwrapped = error_union catch 0;

    // Switch
    const num: i32 = 2;
    const result_switch = switch (num) {
        1 => "one",
        2 => "two",
        3...10 => "three to ten",
        else => "other",
    };
    try stdout.print("Switch result: {s}\n", .{result_switch});

    // While loop
    var i: usize = 0;
    while (i < 5) : (i += 1) {
        try stdout.print("i = {}\n", .{i});
    }

    // For loop with range
    for (0..5) |index| {
        try stdout.print("index = {}\n", .{index});
    }

    // Labeled break and continue
    outer: for (0..3) |a| {
        for (0..3) |b| {
            if (a == 1 and b == 1) {
                break :outer;
            }
            try stdout.print("a={}, b={}\n", .{ a, b });
        }
    }

    // Defer
    {
        const file = try std.fs.cwd().createFile(
            "test.txt",
            .{ .read = true },
        );
        defer file.close();
        try file.writeAll("Hello, File!");
    }

    // Comptime
    comptime {
        const comptime_val = 42;
        std.debug.assert(comptime_val == 42);
    }

    // Generic ArrayList
    var list = ArrayList(i32).init(std.heap.page_allocator);
    defer list.deinit();
    try list.append(1);
    try list.append(2);
    try list.append(3);

    // String formatting
    const name = "Zig";
    const version = 0.11;
    try stdout.print("Language: {s}, Version: {:.2}\n", .{ name, version });

    // Bit manipulation
    const bits: u8 = 0b10101010;
    const shifted = bits << 2;
    const masked = bits & 0x0F;
    try stdout.print("Bits: {b}, Shifted: {b}, Masked: {b}\n", .{ bits, shifted, masked });

    // Inline assembly
    const result_asm = asm volatile (
        \\movl $42, %[ret]
        : [ret] "=r" (-> u32),
    );
    _ = result_asm;

    // Multiline string
    const multiline =
        \\This is a
        \\multiline
        \\string
    ;
    try stdout.print("{s}\n", .{multiline});

    // Character literals
    const char: u8 = 'A';
    const newline: u8 = '\n';
    _ = newline;

    // Hex, octal, binary literals
    const hex = 0xFF;
    const octal = 0o77;
    const binary = 0b11111111;
    _ = hex;
    _ = octal;
    _ = binary;

    // Float literals
    const float1: f32 = 3.14;
    const float2: f64 = 3.14159265359;
    const scientific = 1.5e10;
    _ = float1;
    _ = float2;
    _ = scientific;

    // Sentinel-terminated arrays
    const sentinel_arr: [5:0]u8 = [_:0]u8{ 'h', 'e', 'l', 'l', 'o' };
    _ = sentinel_arr;

    // Error set merging
    const CombinedError = MyError || error{AnotherError};
    _ = CombinedError;
}
