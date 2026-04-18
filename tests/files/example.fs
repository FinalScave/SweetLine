// F# syntax highlighting test
namespace SweetLine.Demo

open System
open System.Collections.Generic
open System.Linq

#if DEBUG
#warn "Debug mode"
#endif
#nowarn "9"

/// XML doc comment for a module
module Core =

    // --- attribute ---
    [<Obsolete("Use newApi instead")>]
    let legacyApi () = ()

    [<Serializable>]
    [<StructuralComparison>]
    type Color =
        | Red
        | Green
        | Blue
        | Custom of int * int * int

    // --- discriminated union ---
    type Shape =
        | Circle of radius: float
        | Rectangle of width: float * height: float
        | Triangle of a: float * b: float * c: float

    // --- record type ---
    type Person = {
        Name: string
        Age: int
        Email: string option
    }

    type MutableCounter = {
        mutable Count: int
    }

    // --- enum ---
    type Status =
        | Active = 0
        | Inactive = 1
        | Pending = 2

    // --- struct ---
    [<Struct>]
    type Point = {
        X: float
        Y: float
    }

    // --- let bindings ---
    let x = 42
    let y = 3.14
    let z = "hello"
    let flag = true
    let nothing = None

    let mutable counter = 0
    counter <- counter + 1

    let rec factorial n =
        if n <= 1 then 1
        else n * factorial (n - 1)

    let add x y = x + y
    let ``value with spaces`` = add x 8
    let inline ``quoted function`` value = value + 1

    // --- pattern matching ---
    let describe shape =
        match shape with
        | Circle r -> sprintf "Circle: %f" r
        | Rectangle (w, h) -> sprintf "Rect: %fx%f" w h
        | Triangle (a, b, c) -> sprintf "Tri: %f,%f,%f" a b c

    let classify n =
        match n with
        | 0 -> "zero"
        | 1 -> "one"
        | _ -> "other"

    // --- option / Result ---
    let tryParse (s: string) =
        match Int32.TryParse s with
        | true, v -> Some v
        | false, _ -> None

    let divide x y =
        if y = 0 then Error "division by zero"
        else Ok (x / y)

    // --- collection operations ---
    let nums = [1; 2; 3; 4; 5]
    let arr = [| 10; 20; 30 |]
    let seq1 = seq { 1 .. 100 }

    let doubled = nums |> List.map (fun x -> x * 2)
    let filtered = nums |> List.filter (fun x -> x > 2)
    let sum = nums |> List.sum

    let dict = dict [ (1, "one"); (2, "two") ]
    let map = Map.ofList [ ("a", 1); ("b", 2) ]

    // --- pipe and compose ---
    let square x = x * x
    let increment x = x + 1
    let squareThenInc = square >> increment
    let incThenSquare = increment >> square
    let composedResult = nums |> List.map square |> List.filter (fun n -> n > 4)

    // --- string literals ---
    let normalStr = "hello world"
    let urlStr = "https://example.com/fsharp?q=highlight"
    let verbatimStr = @"C:\Users\test\file.txt"
    let tripleStr = """this is
    a triple-quoted
    string"""
    let interpStr = $"Name: {z}, Age: {x}"
    let interpVerb = $@"Path: C:\Users\{z}\file.txt"
    let interpTriple = $"""triple value: {x}
    and url: {urlStr}"""
    let charA = 'A'
    let charNewline = '\n'

    // --- numeric literals ---
    let hex = 0xFF
    let bin = 0b1010
    let oct = 0o77
    let floatVal = 1.0e10
    let decimalVal = 19.99m
    let nativeNumber = 42n
    let unsignedNumber = 42u

    // --- control flow ---
    let loopExample () =
        for i in 1 .. 10 do
            printfn "%d" i

        for i = 0 to 9 do
            printfn "%d" i

        while counter < 10 do
            counter <- counter + 1

    // --- exception handling ---
    let safeDivide x y =
        try
            x / y
        with
        | :? DivideByZeroException -> 0

    let safeOp () =
        try
            let result = 10 / 2
            result
        finally
            printfn "cleanup"

    // --- interface ---
    type IRepository<'T> =
        abstract member Find: int -> 'T option
        abstract member GetAll: unit -> 'T list
        abstract member Add: 'T -> unit

    type ITransformer<'Input, 'Output> =
        abstract member Transform: 'Input -> 'Output

    [<RequireQualifiedAccess>]
    type ApiResult<'T> =
        | Success of 'T
        | Failure of string

    [<CLIMutable>]
    type Settings<'T> = {
        Name: string
        Value: 'T option
    }

    // --- class with inheritance ---
    type Animal(name: string) =
        member _.Name = name
        abstract member Speak: unit -> string
        default _.Speak() = "..."

    type Dog(name: string) =
        inherit Animal(name)
        override _.Speak() = "Woof!"

    // --- computation expression ---
    type MaybeBuilder () =
        member _.Bind (x, f) =
            match x with
            | Some v -> f v
            | None -> None
        member _.Return x = Some x

    let maybe = MaybeBuilder ()

    let compute () =
        maybe {
            let! a = Some 10
            let! b = Some 20
            return a + b
        }

    // --- async workflow ---
    let asyncFetch () = async {
        let! data = async { return 42 }
        return data * 2
    }

    // --- active pattern ---
    let (|Even|Odd|) n =
        if n % 2 = 0 then Even
        else Odd

    let describeParity n =
        match n with
        | Even -> "even"
        | Odd -> "odd"

    let (|Small|Medium|Large|) value =
        if value < 10 then Small
        elif value < 100 then Medium
        else Large

    let classifySize value =
        match value with
        | Small -> "small"
        | Medium -> "medium"
        | Large -> "large"

    let genericMap<'Key, 'Value> (items: ('Key * 'Value) list) =
        items |> Map.ofList

    let resizeArray = ResizeArray<string>()
    let lazyText = lazy (sprintf "lazy-%d" x)

    // --- FSI directive ---
    #r "nuget: FSharp.Data"
    #time "on"
    #if INTERACTIVE
    #load "demo.fsx"
    #endif

    /// Summary for main entry
    let main () =
        let person = { Name = "Alice"; Age = 30; Email = Some "alice@test.com" }
        let config = { Name = "theme"; Value = Some "dark" }
        let result = ApiResult.Success person.Name
        let transformed = genericMap [ ("one", 1); ("two", 2) ]
        let advancedValue = ``quoted function`` ``value with spaces``
        printfn "Hello %s" person.Name
        printfn "%A %A %A %A" config result transformed advancedValue

        (* block comment
           spanning multiple lines
           https://example.com/docs *)
        // line comment
        // www.example.org/fsharp/sample
        /// doc comment with https://learn.microsoft.com/dotnet/fsharp/

        0
