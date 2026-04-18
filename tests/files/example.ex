defmodule SweetLine.Example do
  @moduledoc """
  Elixir example coverage for SweetLine.

  The sample exercises module attributes, atoms, sigils, do/end blocks,
  pattern matching, guards, struct updates, interpolation, and charlists.
  """

  alias Enum, as: E
  import Kernel, except: [inspect: 1]
  require Integer

  @behaviour Enumerable
  @enforce_keys [:name, :tags]
  defstruct [:name, :age, :tags, :status, :meta]

  @type t :: %__MODULE__{
          name: String.t(),
          age: non_neg_integer(),
          tags: [atom()],
          status: atom(),
          meta: map()
        }

  @doc "Create a new example struct."
  @spec new(binary(), non_neg_integer()) :: t()
  def new(name, age) when is_binary(name) and is_integer(age) and age >= 0 do
    %__MODULE__{
      name: normalize_name(name),
      age: age,
      tags: [:alpha, :beta, :release],
      status: :ready,
      meta: %{source: :example, version: "1.2.2"}
    }
  end

  @spec normalize_name(binary()) :: binary()
  def normalize_name(name) do
    name
    |> String.trim()
    |> String.downcase()
    |> String.replace(~r/[^a-z0-9]+/u, "-")
    |> String.replace(~r/-+/u, "-")
    |> String.trim("-")
  end

  @spec render(t()) :: binary()
  def render(%__MODULE__{name: name, age: age, tags: tags} = item) do
    joined =
      tags
      |> E.map(&Atom.to_string/1)
      |> E.join(", ")

    summary =
      case age do
        0 -> "newborn"
        1 -> "infant"
        2 -> "toddler"
        _ when age < 13 -> "child"
        _ when age < 18 -> "teen"
        _ -> "adult"
      end

    "#{name} (#{summary}) [#{joined}] #{Kernel.inspect(item.meta)}"
  end

  @spec read_number(binary()) :: integer()
  def read_number(text) do
    with {value, ""} <- Integer.parse(text),
         true <- value >= 0 do
      value
    else
      _ -> -1
    end
  end

  @spec choose_status(t()) :: atom()
  def choose_status(%__MODULE__{status: status, age: age}) do
    cond do
      status == :done -> :done
      age == 0 -> :fresh
      age < 18 -> :young
      true -> :adult
    end
  end

  @spec rescue_flow(binary()) :: binary()
  def rescue_flow(text) do
    try do
      case Integer.parse(text) do
        {value, ""} when value > 10 ->
          "big: #{value}"

        {value, ""} ->
          "small: #{value}"

        _ ->
          raise ArgumentError, message: "not a number"
      end
    rescue
      e in ArgumentError ->
        "error: #{e.message}"
    catch
      :exit, reason ->
        "exit: #{Kernel.inspect(reason)}"
    after
      :ok
    end
  end

  @spec sigils() :: list()
  def sigils do
    regex = ~r/foo|bar/i
    string = ~s(this is a string with "quotes" and #{1 + 2})
    words = ~w(foo bar baz)a
    chars = ~c("char" list)
    upper = ~S(no interpolation here #{1 + 2})
    [
      regex,
      string,
      words,
      chars,
      upper,
      ~r/\w+\s+\d+/u,
      ~w(alpha beta gamma)c
    ]
  end

  @spec anonymous(binary(), binary()) :: binary()
  def anonymous(left, right) do
    fun = fn
      a, b when is_binary(a) and is_binary(b) ->
        a <> "::" <> b
    end

    fun.(left, right)
  end

  @spec maybe_update(t(), map()) :: t()
  def maybe_update(item, params) do
    case params do
      %{name: name, age: age} when is_binary(name) and is_integer(age) ->
        %{item | name: normalize_name(name), age: age}

      %{status: status} when is_atom(status) ->
        %{item | status: status}

      %{"name" => name, "tags" => tags} ->
        %{item | name: normalize_name(name), tags: Enum.map(tags, &String.to_atom/1)}

      _ ->
        item
    end
  end

  @spec demo_strings() :: [binary()]
  def demo_strings do
    [
      "plain string",
      """
      triple quoted
      string with #{:interpolation}
      """,
      'charlist text',
      "escaped \"quote\" and \\n newline",
      "https://hexdocs.pm/elixir/syntax-reference.html"
    ]
  end

  @impl true
  def count(_data), do: {:ok, 1}

  @impl true
  def member?(_data, _value), do: {:ok, false}

  @impl true
  def reduce(_data, {:cont, acc}, fun), do: {:done, fun.(acc), acc}
  def reduce(_data, {:halt, acc}, _fun), do: {:halted, acc}
  def reduce(data, {:suspend, acc}, fun), do: {:suspended, acc, &reduce(data, &1, fun)}

  defp private_join(left, right) when is_binary(left) and is_binary(right) do
    left <> "::" <> right
  end

  defp private_join(left, right), do: to_string(left) <> "::" <> to_string(right)

  def guard_example(value) when is_integer(value) and value in 1..10 do
    {:ok, value}
  end

  def guard_example(value) when is_binary(value) and byte_size(value) > 3 do
    {:text, value}
  end

  def guard_example(_value), do: :error

  def pipeline_example(list) do
    list
    |> Enum.map(&normalize_name/1)
    |> Enum.filter(&String.length(&1) > 0)
    |> Enum.join(", ")
  end

  def with_example(params) do
    with {:ok, name} <- fetch_value(params, :name),
         {:ok, age} <- fetch_value(params, :age),
         true <- is_binary(name),
         true <- is_integer(age) do
      {:ok, new(name, age)}
    else
      {:error, reason} -> {:error, reason}
      false -> {:error, :invalid}
    end
  end

  defp fetch_value(map, key) when is_map(map) do
    case Map.fetch(map, key) do
      {:ok, value} -> {:ok, value}
      :error -> {:error, :missing}
    end
  end

  defp fetch_value(_map, _key), do: {:error, :invalid}

  def sample_map do
    %{
      ready: true,
      empty: nil,
      nested: %{alpha: 1, beta: [1, 2, 3]},
      url: "https://example.org/docs",
      atoms: [:foo, :"bar baz", :release/alpha],
      tuple: {:ok, :done},
      list: [1, 2, 3, 4],
      binary: <<1, 2, 3, 4>>,
      char: ?a
    }
  end
end
