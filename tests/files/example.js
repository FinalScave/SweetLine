// JavaScript 高亮示例

import { readFile } from 'fs/promises';
export default class App {};

/**
 * JSDoc 注释
 * @param {string} name - 名称
 */
function greet(name) {
    return `Hello, ${name}!`;
}

// 类定义
class Animal {
    constructor(name, type) {
        this.name = name;
        this.type = type;
    }

    speak() {
        return `${this.name} is a ${this.type}`;
    }

    static create(name) {
        return new Animal(name, "unknown");
    }
}

class Dog extends Animal {
    #secret = 42;

    get info() {
        return super.speak() + " (Dog)";
    }
}

// 内置对象和方法
const arr = Array.from({ length: 5 }, (_, i) => i);
const now = Date.now();
const str = String.fromCharCode(65);
const jsonStr = JSON.stringify({ key: "value" });
const parsed = JSON.parse('{"a": 1}');
const promise = Promise.resolve(42);
console.log(Math.PI);

// 内置常量
const a = true;
const b = false;
const c = null;
const d = undefined;
const e = NaN;
const f = Infinity;

// 全局函数
const num = parseInt("42", 10);
const flt = parseFloat("3.14");
const valid = isNaN(NaN);
const finite = isFinite(100);
const encoded = encodeURIComponent("hello world");

// 数字字面量
let hex = 0xFF;
let bin = 0b1010;
let oct = 0o77;
let big = 100_000n;
let sci = 1.5e10;

// 正则表达式
const regex = /^[a-z]+$/gi;

// 模板字符串
const tpl = `Count: ${arr.length}, Time: ${now}`;

// 解构和展开
const { key: val } = parsed;
const [first, ...rest] = arr;

// async/await
async function fetchData() {
    try {
        const data = await readFile("test.txt");
        return data;
    } catch (err) {
        throw new Error("Failed");
    } finally {
        console.log("done");
    }
}

// 可选链和空值合并
const obj = { nested: { value: 10 } };
const safe = obj?.nested?.value ?? 0;

/* 多行注释
   跨越多行 */
