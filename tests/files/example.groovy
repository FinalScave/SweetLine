// Groovy Sample

package com.example

import groovy.transform.CompileStatic
import java.util.stream.Collectors

interface Greeter {
    String greet(String name)
}

trait Auditable {
    Date createdAt = new Date()
}

enum Status {
    ACTIVE, INACTIVE, PENDING
}

// Closure & Groovy def variable
def process(List<String> items) {
    items.each { item ->
        println(item.toUpperCase())
    }

    // Groovy implicit variable
    items.collect { it.length() }
    def obj = new Person("test", 0)
    def d = obj.delegate
    def o = obj.owner
}

// Constant literal
def flag = true
def off = false
def empty = null

def main() {
    def person = new Person("Alice", 30)
    println(person.toString())

    // Number
    def hex = 0xDEAD
    def bin = 0b1010
    def bigDecimal = 3.14G
    def bigInt = 100G
    def longVal = 1000L

    // GString
    def msg = "Hello, ${person.name}! Age: ${person.age}"
    def simple = "Name: $person.name"

    // Multi-line string
    def multi = """
        Line 1
        Line ${1 + 1}
    """

    def raw = '''
        raw string
        no interpolation
    '''

    // Regex
    def pattern = /^[a-z]+$/

    // List
    def list = [1, 2, 3, 4, 5]
    def map = [name: "Alice", age: 30]
    def range = 1..10

    // Control
    list.findAll { it > 2 }.each { println(it) }

    switch (person.age) {
        case 0..17:
            println("minor")
            break
        case 18..64:
            println("adult")
            break
        default:
            println("senior")
    }

    // Safe nav & Elvis
    def safe = person?.name ?: "unknown"
    def spread = [person]*.name

    /* comment
       multi-line */
}
