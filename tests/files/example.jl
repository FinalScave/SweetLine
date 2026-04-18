# Julia sample for SweetLine
# Official docs:
# https://docs.julialang.org/en/v1/manual/strings/
# https://docs.julialang.org/en/v1/manual/metaprogramming/

module SweetLineDemo

using Printf
using Statistics: mean
import Base: show

#= Block comment example.
   The parser should keep this as a comment until =#.
=#

"""
Trace a value and return the wrapped expression.
"""
macro trace_value(expr)
    quote
        local value = $(expr)
        @show value
        value
    end
end

abstract type AbstractShape end

struct Point{T<:Real}
    x::T
    y::T
end

mutable struct Counter{T<:Integer}
    value::T
end

struct Payload{T}
    name::String
    data::Vector{T}
end

function show(io::IO, p::Point)
    print(io, "Point(", p.x, ", ", p.y, ")")
end

function normalize_name(name::AbstractString, fallback::AbstractString = "unknown")
    cleaned = strip(lowercase(String(name)))
    isempty(cleaned) && return fallback
    cleaned = replace(cleaned, r"[^a-z0-9._-]+" => "_")
    cleaned = replace(cleaned, r"_+" => "_")
    return isempty(cleaned) ? fallback : cleaned
end

function clamp_value(value::Real, min_value::Real = 0, max_value::Real = 100)
    if value < min_value
        return min_value
    elseif value > max_value
        return max_value
    else
        return value
    end
end

function distance(a::Point{T}, b::Point{T}) where {T<:Real}
    dx = a.x - b.x
    dy = a.y - b.y
    return sqrt(dx^2 + dy^2)
end

function score(values::AbstractVector{<:Real}; scale::Real = 1.0)
    total = zero(eltype(values))
    for value in values
        total += value
    end
    return total * scale
end

function windowed_mean(values::AbstractVector{<:Real}, window::Int = 3)
    if length(values) < window
        return Float64[]
    end

    result = Float64[]
    index = 1
    while index <= length(values) - window + 1
        push!(result, mean(values[index:index + window - 1]))
        index += 1
    end
    return result
end

function render_report(items::AbstractVector{<:Real}, title::AbstractString = "report")
    lines = String[]
    for (index, item) in enumerate(items)
        push!(lines, @sprintf("%s[%d] = %.2f", title, index, item))
    end
    return join(lines, "\n")
end

function evaluate_commands(name::AbstractString)
    plain = "Hello, $name!"
    raw_text = raw"c:\temp\logs\example.txt"
    regex = r"^[A-Za-z_]\w*$"
    bytes = b"ABC\x20DEF"
    templated = """Hello, $(uppercase(name))
This string spans multiple lines.
"""
    command = `printf "%s %s" $name $(uppercase(name))`
    return (plain, raw_text, regex, bytes, templated, command)
end

function transform!(counter::Counter{T}, values::AbstractVector{T}) where {T<:Integer}
    for value in values
        if value < 0
            continue
        end
        counter.value += value
        if counter.value > typemax(T) - 10
            break
        end
    end
    return counter
end

function pipeline()
    numbers = [1, 2, 3, 4, 5, 8, 13, 21, 34, 55]
    names = ["alpha", "beta", "gamma", "delta"]
    flags = Bool[true, false, true, true]
    matrix = reshape(collect(1:12), 3, 4)

    point_a = Point(1.5, 2.5)
    point_b = Point(5.0, 7.5)
    counter = Counter(0)

    @time begin
        @show distance(point_a, point_b)
        @show score(numbers, scale = 1.25)
        @assert !isempty(names)
        transform!(counter, [1, 2, 3, 4, 5])
    end

    let total = sum(numbers), average = mean(numbers)
        println("total = $(total), average = $(round(average, digits = 2))")
    end

    try
        open("sweetline-demo.txt", "w") do io
            println(io, render_report(numbers, "numbers"))
            println(io, "matrix size = $(size(matrix))")
            println(io, "flags = $(join(string.(flags), \",\"))")
        end
    catch err
        @show err
    finally
        println("finished writing demo output")
    end

    if all(flags)
        println("all flags are true")
    elseif any(!, flags)
        println("at least one flag is false")
    else
        println("flags are mixed")
    end

    for value in numbers
        if value % 2 == 0
            continue
        end
        println("odd value: ", value)
    end

    result = @trace_value sum(windowed_mean(numbers, 4))
    return (
        point_a = point_a,
        point_b = point_b,
        counter = counter,
        result = result,
        text = evaluate_commands("SweetLine")
    )
end

function demo_keywords()
    data = Dict{Symbol, Any}(
        :name => "SweetLine",
        :count => 42,
        :enabled => true,
        :missing_value => nothing
    )
    begin
        local_value = get(data, :count, 0)
        global_value = clamp_value(local_value, 0, 100)
        return (local_value, global_value)
    end
end

function main()
    payload = Payload("example", [1, 2, 3, 4, 5])
    @show normalize_name(payload.name)
    @show clamp_value(123, 10, 120)
    @show pipeline()
    @show demo_keywords()
    println("raw literal: ", raw"c:\users\demo\file.txt")
end

end # module SweetLineDemo
