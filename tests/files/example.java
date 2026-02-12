package com.sweetline.example;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.io.*;

/* 多行注释
 * 展示 Java 高亮特性
 */

// 注解定义
@interface MyAnnotation {
    String value() default "default";
}

// 枚举类型
enum Color {
    RED, GREEN, BLUE
}

// record 类型
record Point(double x, double y) {}

// 接口 + 泛型
interface Comparable<T> {
    int compareTo(T other);
}

// sealed 类 + permits
sealed class Shape permits Circle, Rectangle {
    abstract double area();
}

// 泛型类 + extends + implements
public final class Circle extends Shape implements Comparable<Circle> {
    private final double radius;

    public Circle(double radius) {
        this.radius = radius;
    }

    @Override
    public double area() {
        return Math.PI * radius * radius;
    }

    @Override
    public int compareTo(Circle other) {
        return Double.compare(this.radius, other.radius);
    }
}

// 非密封类
non-sealed class Rectangle extends Shape {
    protected int width;
    protected int height;

    Rectangle(int width, int height) {
        this.width = width;
        this.height = height;
    }

    @Override
    double area() {
        return width * height;
    }
}

// 泛型方法、通配符、变量、控制流
@SuppressWarnings("unchecked")
@MyAnnotation(value = "示例")
public class Example<T extends Comparable<T>> {

    // 原始类型变量
    private static final int MAX_SIZE = 100;
    private boolean enabled = true;
    private char letter = 'A';
    private long bigNum = 0xFFFF_FFFFL;
    private int binary = 0b1010_0101;
    private double pi = 3.14159d;
    private float ratio = 2.5f;

    // 泛型字段
    private List<String> names;
    private Map<String, List<Integer>> dataMap;

    // 构造方法
    public Example(List<String> names) {
        this.names = names;
        this.dataMap = Map.of();
    }

    // 泛型方法 + 通配符
    public static <E extends Comparable<? super E>> E findMax(List<? extends E> list) {
        E max = null;
        for (E item : list) {
            if (max == null || item.compareTo(max) > 0) {
                max = item;
            }
        }
        return max;
    }

    // lambda + 方法引用 + 流式API
    public List<String> process() {
        return names.stream()
            .filter(name -> name.length() > 3)
            .map(String::toUpperCase)
            .sorted()
            .collect(Collectors.toList());
    }

    // switch表达式 + yield
    public String describe(Color color) {
        return switch (color) {
            case RED -> "红色";
            case GREEN -> "绿色";
            case BLUE -> {
                String msg = "蓝色";
                yield msg;
            }
        };
    }

    // instanceof + try-catch-finally
    public void handle(Object obj) {
        try {
            if (obj instanceof String str) {
                System.out.println(str.length());
            } else if (obj instanceof Integer) {
                throw new IllegalArgumentException("不支持整数");
            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } finally {
            System.out.println("处理完毕");
        }
    }

    // 数组 + 循环
    public int[] createArray() {
        int[] arr = new int[MAX_SIZE];
        int i = 0;
        while (i < arr.length) {
            arr[i] = i * 2;
            i++;
        }
        do {
            i--;
        } while (i > 0);
        return arr;
    }

    // 泛型 new + 断言
    public void demo() {
        List<String> list = new ArrayList<String>();
        Map<String, List<Integer>> map = new ArrayList<>();
        var value = list.get(0);
        assert value != null : "值不能为空";
    }

    // 文本块
    public String getJson() {
        return """
            {
                "name": "SweetLine",
                "version": "1.0.0"
            }
            """;
    }

    // main方法
    public static void main(String[] args) {
        Example<Circle> example = new Example<>(List.of());
        Circle c1 = new Circle(5.0);
        Circle c2 = new Circle(3.0);
        Circle max = findMax(List.of(c1, c2));
        System.out.println("面积: " + c1.area());
        System.out.println("较大: " + max);
    }
}
