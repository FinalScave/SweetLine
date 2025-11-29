// JavaScript sample

import { readFile } from 'fs/promises';
export default class App {};

/**
 * JSDoc comment
 * @param {string} name - name
 */
function greet(name) {
    return `Hello, ${name}!`;
}

// class definition
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

// built-in object and method
const arr = Array.from({ length: 5 }, (_, i) => i);
const now = Date.now();
const str = String.fromCharCode(65);
const jsonStr = JSON.stringify({ key: "value" });
const parsed = JSON.parse('{"a": 1}');
const promise = Promise.resolve(42);
console.log(Math.PI);

// built-in constant
const a = true;
const b = false;
const c = null;
const d = undefined;
const e = NaN;
const f = Infinity;

// global function
const num = parseInt("42", 10);
const flt = parseFloat("3.14");
const valid = isNaN(NaN);
const finite = isFinite(100);
const encoded = encodeURIComponent("hello world");

// numeric literal
let hex = 0xFF;
let bin = 0b1010;
let oct = 0o77;
let big = 100_000n;
let sci = 1.5e10;

// regular expression
const regex = /^[a-z]+$/gi;

// template string
const tpl = `Count: ${arr.length}, Time: ${now}`;

// destructuring and spreading
const { key: val } = parsed;
const [first, ...rest] = arr;

// asynchronous/await
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

// optional chaining and nullish coalescing
const obj = { nested: { value: 10 } };
const safe = obj?.nested?.value ?? 0;

/* multi-line comment
   can span multiple lines */
