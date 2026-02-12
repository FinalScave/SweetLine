// Kotlin 高亮示例
package com.example.demo

import kotlin.collections.mutableListOf
import kotlinx.coroutines.delay

@Target(AnnotationTarget.CLASS)
annotation class Serializable

enum class Color { RED, GREEN, BLUE }

data class User(val name: String, val age: Int)

sealed class Result<out T> {
    data class Success<T>(val data: T) : Result<T>()
    data class Error(val message: String) : Result<Nothing>()
}

interface Repository<T> {
    suspend fun findById(id: Int): T?
    fun getAll(): List<T>
}

@Serializable
class UserService : Repository<User> {
    private val users = mutableListOf<User>()

    override suspend fun findById(id: Int): User? {
        delay(100)
        return users.getOrNull(id)
    }

    override fun getAll(): List<User> = users

    fun add(user: User) {
        users.add(user)
    }
}

// 扩展函数
fun String.isPalindrome(): Boolean =
    this == this.reversed()

fun main() {
    val service = UserService()
    val user = User("Alice", 30)
    service.add(user)

    // 数字字面量
    val hex = 0xFF_AB
    val bin = 0b1100_0011
    val long = 100_000L
    val float = 3.14f
    val double = 2.718

    val isValid = true
    val nothing: String? = null

    // 字符串模板
    val msg = "User: ${user.name}, Age: ${user.age}"
    val raw = """
        |multi-line
        |raw string
    """.trimMargin()

    // 控制流
    val result = when (user.age) {
        in 0..17 -> "minor"
        in 18..64 -> "adult"
        else -> "senior"
    }

    val items = listOf("a", "bb", "ccc")
    for ((index, item) in items.withIndex()) {
        if (item.length > 1) {
            println("$index: $item")
        }
    }

    // lambda 和高阶函数
    val squares = items.map { it.length }.filter { it > 1 }
    val owner = false

    /* 多行注释
       跨越多行 */
}
