// C# 高亮示例
using System;
using System.Linq;

#if DEBUG
#warning Debug build active
#endif

namespace SweetLine.Demo
{
    [Serializable]
    public class Person
    {
        public decimal Salary { get; private set; }
        public List<string> Tags { get; set; }
        public Dictionary<string, int> Scores { get; set; }

        public Person(string name, int age)
        {
            Name = name ?? throw new ArgumentNullException(nameof(name));
            Age = age;
        }

        public override string ToString() => $"Name: {Name}, Age: {Age}";
    }

    public interface IRepository<T> where T : class
    {
        Task<T?> FindAsync(int id);
        void Add(T entity);
    }

    public record Point(double X, double Y);
    public record class ColorPoint(double X, double Y, string Color);
    public record struct Velocity(float Dx, float Dy);

    public enum Status { Active, Inactive, Pending }

    public static class Program
    {
        public static async Task Main(string[] args)
        {
            var person = new Person("Alice", 30);
            Console.WriteLine(person.ToString());

            // 数字字面量
            int hex = 0xFF_AB;
            int bin = 0b1100_0011;
            decimal price = 19.99m;
            float ratio = 0.5f;

            bool isValid = true;
            object obj = null;
            int size = sizeof(int);
            Type t = typeof(Person);
            string def = default;

            // 泛型集合
            var list = new List<string> { "one", "two", "three" };
            var dict = new Dictionary<string, int>();
            dict.Add("key", 42);

            // LINQ 与 lambda
            var filtered = list.Where(x => x.Length > 2).ToList();

            // 字符串
            string verbatim = @"C:\Users\test\file.txt";
            string interpolated = $"Count: {list.Count}";

            // 控制流
            foreach (var item in filtered)
            {
                if (item is not null)
                {
                    Console.WriteLine(item);
                }
            }

            /* 多行注释
               跨越多行 */
            await Task.Delay(100);
        }
    }
}
