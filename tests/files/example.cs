// C# syntax highlighting test
using System;
using System.Linq;
using System.Collections.Generic;
using static System.Math;

#if DEBUG
#warning Debug build active
#endif
#nullable enable
#region Main Code

namespace SweetLine.Demo
{
    // attribute
    [Serializable]
    [Obsolete("Use NewPerson instead")]
    public class Person
    {
        // member field: built-in type
        private int _age;
        private string _name = "unknown";
        private decimal _salary;
        private bool? _isActive;
        private int? _score = null;
        private string[] _tags;
        private int[]? _data;

        // member field: reference type
        private Person? _parent;
        private Address _address;
        private List<string> _items;
        private List<string>? _optionalItems;
        private Dictionary<string, int> _scores;
        private Dictionary<string, List<int>>? _nested;

        // property: built-in type
        public string Name { get; set; }
        public int Age { get; private set; }
        public decimal Salary { get; init; }
        public bool? IsActive { get; set; }
        public int? Score { get; set; }

        // property: reference type
        public Person? Parent { get; set; }
        public List<string> Tags { get; set; }
        public List<string>? OptionalTags { get; set; }
        public Dictionary<string, int> Scores { get; set; }

        // arrow member (expression body)
        public string FullName => $"{_name} (age: {_age})";
        public int DoubleAge => _age * 2;
        public Person? GetParent => _parent;
        public List<int>? Numbers => null;

        // constructor
        public Person(string name, int age)
        {
            _name = name ?? throw new ArgumentNullException(nameof(name));
            _age = age;
        }

        // method: built-in type return
        public string ToString() => $"Name: {_name}, Age: {_age}";
        public int GetAge() { return _age; }
        public bool? CheckActive() { return _isActive; }

        // method: reference type return
        public Person? FindParent(int id) { return null; }
        public List<string> GetTags() { return _tags.ToList(); }
        public Dictionary<string, int>? GetScores() { return _scores; }

        // method with multiple params
        public void Update(
            string name,
            int age,
            bool? active = null,
            params string[] tags)
        {
            _name = name;
            _age = age;
        }
    }

    // interface with generic constraint
    public interface IRepository<T> where T : class, new()
    {
        Task<T?> FindAsync(int id);
        Task<List<T>> GetAllAsync();
        void Add(T entity);
        bool Remove(T entity);
    }

    // generic class with multiple constraints
    public class Repository<T, TKey> : IRepository<T>
        where T : class, new() where TKey : struct
    {
        private readonly Dictionary<TKey, T> _store = new Dictionary<TKey, T>();

        public Task<T?> FindAsync(int id)
        {
            return Task.FromResult<T?>(default);
        }

        public Task<List<T>> GetAllAsync()
        {
            var list = new List<T>();
            return Task.FromResult(list);
        }
    }

    // record types
    public record Point(double X, double Y);
    public record class ColorPoint(double X, double Y, string Color);
    public record struct Velocity(float Dx, float Dy);

    // enum
    public enum Status { Active, Inactive, Pending }

    // delegate
    public delegate void EventCallback(object sender, EventArgs args);
    public delegate Task<bool> AsyncPredicate<T>(T item);

    public static class Extensions
    {
        // generic method with built-in return type
        public static T[] ToArray<T>(IEnumerable<T> source) { return null; }

        // generic method with generic return type
        public static List<T> Filter<T>(List<T> items) { return items; }

        // simple return type + generic method
        public static Person FindFirst<T>(List<T> items) { return null; }
    }

    public static class Program
    {
        // extension method
        public static string ToUpper(this string s) => s.ToUpperInvariant();

        public static async Task Main(string[] args)
        {
            var person = new Person("Alice", 30);
            Console.WriteLine(person.ToString());

            // numeric literal
            int hex = 0xFF_AB;
            int bin = 0b1100_0011;
            decimal price = 19.99m;
            float ratio = 0.5f;
            long big = 1_000_000L;

            // nullable
            int? nullableInt = null;
            string? nullableStr = "hello";
            Person? nullablePerson = null;
            List<int>? nullableList = null;

            // built-in constant
            bool isValid = true;
            object obj = null;
            int size = sizeof(int);
            Type t = typeof(Person);
            string name = nameof(person);
            string def = default;

            // generic collection
            var list = new List<string> { "one", "two", "three" };
            var dict = new Dictionary<string, int>();
            var set = new HashSet<string>();
            dict.Add("key", 42);

            // LINQ and lambda
            var filtered = list.Where(x => x.Length > 2).ToList();
            var result = list.Select(x => new { Name = x, Len = x.Length });

            // string type
            string normal = "hello world";
            string verbatim = @"C:\Users\test\file.txt";
            string interpolated = $"Count: {list.Count}";
            string interpVerbatim = $@"Path: C:\{name}\file";
            string interpVerbatim2 = @$"Path: C:\{name}\file";
            string raw = """
                This is a raw string literal
                No escaping needed: \n \t
                """;
            char ch = 'A';
            char escape = '\n';

            // control flow
            foreach (var item in filtered)
            {
                if (item is not null)
                {
                    Console.WriteLine(item);
                }
            }

            for (int i = 0; i < 10; i++)
            {
                switch (i)
                {
                    case 0: break;
                    case 1: continue;
                    default: Console.Write(i); break;
                }
            }

            // pattern matching
            object value = 42;
            if (value is int number)
            {
                Console.WriteLine(number);
            }

            // try-catch
            try
            {
                await Task.Delay(100);
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine(ex.Message);
            }
            finally
            {
                Console.WriteLine("done");
            }

            // null-coalescing
            string s = nullableStr ?? "default";
            int len = nullableStr?.Length ?? 0;
            nullableStr ??= "fallback";

            /* multi-line comment
               can span multiple lines */
            /// <summary>XML doc comment</summary>
        }
    }

    #endregion
}
