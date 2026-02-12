# Python 高亮示例

"""模块文档字符串"""

import os
from typing import List, Optional

@staticmethod
def greet(name: str) -> str:
    return f"Hello, {name}!"

class Animal:
    """动物基类"""

    def __init__(self, name: str, age: int):
        self.name = name
        self.age = age

    @property
    def info(self) -> str:
        return f'{self.name} is {self.age} years old'

    async def fetch_data(self) -> Optional[dict]:
        await some_async_call()
        return None

class Dog(Animal):
    def speak(self) -> str:
        return "Woof!"

# 内置函数和类型
nums = list(range(10))
total = sum(nums)
length = len(nums)
result = sorted(nums, reverse=True)

# 字面量和数字
flag = True
empty = None
pi = 3.14159
hex_val = 0xFF
bin_val = 0b1010
oct_val = 0o77
complex_val = 3+4j

# 控制流
for i, val in enumerate(nums):
    if val > 5 and not flag:
        print(f"Value: {val}")
        break
    elif val == 0:
        continue

# 异常处理
try:
    data = open("file.txt").read()
except FileNotFoundError as e:
    print(repr(e))
finally:
    pass

# lambda 和推导式
square = lambda x: x ** 2
evens = [x for x in range(20) if x % 2 == 0]
mapping = {str(k): bool(v) for k, v in zip(nums, evens)}
