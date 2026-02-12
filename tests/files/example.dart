// Dart 高亮示例

import 'dart:async';
import 'dart:math';

/// 文档注释
abstract class Animal {
  final String name;
  final int age;
  late String nickname;
  String? description;
  bool isActive = true;

  Animal(this.name, this.age);

  String speak();

  @override
  String toString() => '$name (age: $age)';
}

class Dog extends Animal {
  final String breed;

  Dog(String name, int age, this.breed) : super(name, age);

  @override
  String speak() => '$name says Woof!';

  void greet(String greeting, int times) {
    for (int i = 0; i < times; i++) {
      print(greeting);
    }
  }
}

class Cat extends Animal implements Comparable<Cat> {
  Cat(String name, int age) : super(name, age);

  @override
  String speak() => '$name says Meow!';

  @override
  int compareTo(Cat other) => name.compareTo(other.name);
}

class Repository<T extends Comparable<T>> {
  final List<T> items;
  late Map<String, T> index;

  Repository(this.items);

  Future<T?> findById(String id) async {
    return index[id];
  }
}

mixin Flyable on Animal {
  void fly() => print('$name is flying');
}

enum Color { red, green, blue }

class Pair<T, U> {
  final T first;
  final U second;
  late Map<String, dynamic> metadata;
  const Pair(this.first, this.second);
}

extension StringExt on String {
  bool get isPalindrome => this == split('').reversed.join();
}

// 异步
Future<String> fetchData() async {
  await Future.delayed(Duration(seconds: 1));
  return "data";
}

Stream<int> countStream(int max) async* {
  for (int i = 0; i < max; i++) {
    yield i;
  }
}

void main() async {
  final dog = Dog("Buddy", 3);
  print(dog.speak());

  // 数字字面量
  int hex = 0xFF;
  double pi = 3.14159;
  num sci = 1.5e10;

  // 内置常量
  bool flag = true;
  bool off = false;
  var empty = null;

  // 字符串
  String single = 'single quotes';
  String double_ = "double quotes";
  String interpolated = "Name: ${dog.name}, Age: ${dog.age}";
  String raw = r'raw string \n no escape';
  String multiline = '''
    multi-line
    string literal
  ''';

  // 集合
  var list = <int>[1, 2, 3];
  var map = <String, int>{'a': 1, 'b': 2};
  var set = <String>{'x', 'y', 'z'};

  // 控制流
  for (var item in list) {
    if (item > 1) {
      print(item);
    }
  }

  switch (dog.age) {
    case 0:
      break;
    default:
      print("adult");
  }

  // 空安全
  String? nullable;
  var safe = nullable?.length ?? 0;
  var forced = nullable!;

  // 级联操作
  dog
    ..speak()
    ..toString();

  /* 多行注释
     跨越多行 */
  final data = await fetchData();
}
