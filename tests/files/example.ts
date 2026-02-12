// TypeScript 高亮示例

import { EventEmitter } from 'events';
export type Result<T> = Success<T> | Failure;

// 接口定义
interface User {
    readonly id: number;
    name: string;
    email?: string;
}

// 枚举
enum Status {
    Active = "active",
    Inactive = "inactive",
}

// 泛型类
class Repository<T extends { id: number }> {
    private items: Map<number, T> = new Map();

    add(item: T): void {
        this.items.set(item.id, item);
    }

    find(id: number): T | undefined {
        return this.items.get(id);
    }

    getAll(): Array<T> {
        return Array.from(this.items.values());
    }
}

// 装饰器
@Component({ selector: 'app-root' })
class AppComponent {
    title: string = "Hello";
}

// 内置类型和工具类型
type ReadonlyUser = Readonly<User>;
type PartialUser = Partial<User>;
type PickedUser = Pick<User, "id" | "name">;
type StringKeys = Extract<keyof User, string>;
type Callback = Parameters<typeof setTimeout>;

// 内置常量和对象
const a: boolean = true;
const b: boolean = false;
const c: null = null;
const d: undefined = undefined;
const e: number = NaN;
const f: number = Infinity;
const g: never = (() => { throw new Error() })();

// 全局对象
const data = JSON.stringify({ key: "value" });
const promise = Promise.all([1, 2, 3]);
const set = new Set<string>(["a", "b"]);
const map = new Map<string, number>();
console.log(Math.floor(3.14));
const el = document.querySelector("div");

// 数字字面量
let hex: number = 0xFF;
let bin: number = 0b1010;
let big: bigint = 100_000n;

// 模板字符串
const msg = `Items: ${set.size}, Data: ${data}`;

// 正则表达式
const pattern = /^[a-z]+$/i;

// async/await
async function fetchUser(id: number): Promise<User> {
    const response = await fetch(`/api/users/${id}`);
    return response.json() as unknown as User;
}

// 类型守卫
function isString(val: any): val is string {
    return typeof val === "string";
}

/* 多行注释
   跨越多行 */
