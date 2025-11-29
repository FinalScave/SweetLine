package com.sweetline.example;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.io.*;

/* multi-line comment
 * show Java highlight features
 */

// annotation definition
@interface MyAnnotation {
    String value() default "default";
    int count() default 0;
    boolean enabled() default true;
}

// annotation with target
@interface Validate {
    String message() default "";
    Class<?>[] groups() default {};
}

// enum type
enum Color {
    RED, GREEN, BLUE
}

// record type
record Point(double x, double y) {}

// interface + generic
interface Comparable<T> {
    int compareTo(T other);
}

// sealed class + permits
sealed class Shape permits Circle, Rectangle {
    abstract double area();
}

// generic class + extends + implements
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

// non-sealed class
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

// generic method, wildcard, variable, control flow
@SuppressWarnings("unchecked")
@MyAnnotation(value = "示例", count = 5, enabled = true)
public class Example<T extends Comparable<T>> {

    // primitive type variable
    private static final int MAX_SIZE = 100;
    private boolean enabled = true;
    private char letter = 'A';
    private long bigNum = 0xFFFF_FFFFL;
    private int binary = 0b1010_0101;
    private double pi = 3.14159d;
    private float ratio = 2.5f;

    // generic field
    private List<String> names;
    private Map<String, List<Integer>> dataMap;

    // constructor
    public Example(List<String> names) {
        this.names = names;
        this.dataMap = Map.of();
    }

    // generic method + wildcard (simple generic params)
    public static <T> T identity(T item) {
        return item;
    }

    // generic method with primitive return
    public static <T> void process(T item) {
        System.out.println(item);
    }

    // generic method with generic return type
    public static <T> List<T> wrap(T item) {
        return List.of(item);
    }

    // generic method + nested generic params (>> ending)
    public static <E extends Comparable<? super E>> E findMax(List<? extends E> list) {
        E max = null;
        for (E item : list) {
            if (max == null || item.compareTo(max) > 0) {
                max = item;
            }
        }
        return max;
    }

    // generic method with nested params + generic return
    public static <E extends Comparable<? super E>> List<E> sorted(List<E> input) {
        return input.stream().sorted().collect(Collectors.toList());
    }

    // multiple type params
    public static <K, V> Map<K, V> combine(List<K> keys, List<V> values) {
        return Map.of();
    }

    // lambda + method reference + stream API
    public List<String> transform() {
        return names.stream()
            .filter(name -> name.length() > 3)
            .map(String::toUpperCase)
            .sorted()
            .collect(Collectors.toList());
    }

    // switch expression + yield
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

    // array + loop
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

    // generic new + assertion
    public void demo() {
        List<String> list = new ArrayList<String>();
        Map<String, List<Integer>> map = new ArrayList<>();
        var value = list.get(0);
        assert value != null : "值不能为空";
    }

    // text block
    public String getJson() {
        return """
            {
                "name": "SweetLine",
                "version": "1.0.0"
            }
            """;
    }

    // main method
    public static void main(String[] args) {
        Example<Circle> example = new Example<>(List.of());
        Circle c1 = new Circle(5.0);
        Circle c2 = new Circle(3.0);
        Circle max = findMax(List.of(c1, c2));
        System.out.println("面积: " + c1.area());
        System.out.println("较大: " + max);
    }
}
