// TypeScript Highlight Example

import { EventEmitter } from 'events';
export type Result<T> = Success<T> | Failure;

// Interface: members with optional (?), readonly, generic types
interface User {
    readonly id: number;
    name: string;
    email?: string;
    tags?: string[];
    metadata?: Map<string, any>;
}

// Enum
enum Status {
    Active = "active",
    Inactive = "inactive",
}

// Generic class with extends and implements
class Repository<T extends { id: number }> implements Iterable<T> {
    private items: Map<number, T> = new Map();
    protected count: number = 0;
    public readonly name: string;

    constructor(name: string) {
        this.name = name;
    }

    add(item: T): void {
        this.items.set(item.id, item);
        this.count++;
    }

    find(id: number): T | undefined {
        return this.items.get(id);
    }

    getAll(): Array<T> {
        return Array.from(this.items.values());
    }

    async fetchById(id: number): Promise<T | null> {
        return this.find(id) ?? null;
    }

    [Symbol.iterator](): Iterator<T> {
        return this.items.values();
    }
}

// Multi-line class declaration
class AdvancedRepo<K, V>
    extends Repository<V>
    implements Map<K, V> {
}

// Decorator
@Component({ selector: 'app-root' })
class AppComponent {
    title: string = "Hello";
    visible?: boolean;
}

// Utility types
type ReadonlyUser = Readonly<User>;
type PartialUser = Partial<User>;
type PickedUser = Pick<User, "id" | "name">;
type StringKeys = Extract<keyof User, string>;
type Callback = Parameters<typeof setTimeout>;

// Built-in values
const a: boolean = true;
const b: boolean = false;
const c: null = null;
const d: undefined = undefined;
const e: number = NaN;
const f: number = Infinity;
const g: never = (() => { throw new Error() })();

// Built-in objects and methods
const data = JSON.stringify({ key: "value" });
const promise = Promise.all([1, 2, 3]);
const set = new Set<string>(["a", "b"]);
const map = new Map<string, number>();
console.log(Math.floor(3.14));

// Number literals
let hex: number = 0xFF;
let bin: number = 0b1010;
let oct: number = 0o77;
let big: bigint = 100_000n;

// Template strings
const msg = `Items: ${set.size}, Data: ${data}`;

// Regex
const pattern = /^[a-z]+$/i;

// Async function with return type
async function fetchUser(id: number): Promise<User> {
    const response = await fetch(`/api/users/${id}`);
    return response.json() as unknown as User;
}

// Generic function
function identity<T>(value: T): T {
    return value;
}

// Arrow function with return type
const double = (x: number): number => x * 2;

// Function with optional params and rest
function configure(
    host: string,
    port?: number,
    ...options: string[]
): void {
    console.log(host);
}

// Type guard: val is string
function isString(val: any): val is string {
    return typeof val === "string";
}

// Conditional type
type IsString<T> = T extends string ? "yes" : "no";

// Mapped type
type Optional<T> = { [K in keyof T]?: T[K] };

// Namespace path in type
const el: HTMLElement | null = document.querySelector("div");

/* Multi-line
   comment */
