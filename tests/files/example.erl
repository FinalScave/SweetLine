-module(sweetline_example).
-moduledoc("SweetLine Erlang example").
-doc("Erlang syntax coverage for SweetLine").

-export([new/2, render/1, sum/1, average/1, bit_examples/0, record_examples/0]).
-export_type([person/0, state/0]).
-behaviour(gen_server).

-define(DEFAULT_LIMIT, 4).
-define(TAG_LIST, [alpha, beta, gamma]).
-define(DEFAULT_PERSON(Name, Age), #person{name = Name, age = Age, tags = ?TAG_LIST}).

-record(person, {
    name = <<"">>,
    age = 0,
    tags = [],
    meta = #{}
}).

-type person() :: #person{
    name := binary(),
    age := non_neg_integer(),
    tags := [atom()],
    meta := map()
}.

-type state() :: #{limit => non_neg_integer(), items => [person()]}.

-spec new(string(), non_neg_integer()) -> person().
new(Name, Age) when is_list(Name), is_integer(Age), Age >= 0 ->
    Name1 = normalize_name(Name),
    ?DEFAULT_PERSON(list_to_binary(Name1), Age);
new(Name, Age) ->
    new(io_lib:format("~s", [Name]), Age).

-spec normalize_name(string()) -> string().
normalize_name(Name) ->
    Lower = string:lowercase(string:trim(Name)),
    string:replace(Lower, " ", "-", all).

-spec render(person()) -> iodata().
render(#person{name = Name, age = Age, tags = Tags, meta = Meta}) ->
    Summary =
        case Age of
            0 -> newborn;
            1 -> infant;
            2 -> toddler;
            _ when Age < 18 -> child;
            _ -> adult
        end,
    io_lib:format("~s (~p, ~p, ~p)", [Name, Summary, Tags, Meta]).

-spec sum([integer()]) -> integer().
sum(List) ->
    lists:sum(List).

-spec average([integer()]) -> float().
average([]) ->
    0.0;
average(List) ->
    sum(List) / length(List).

-spec bit_examples() -> list().
bit_examples() ->
    Bin = <<1, 2, 3, 4>>,
    Utf = <<"hello"/utf8>>,
    Match = case Bin of
        <<Head:8, Tail/binary>> -> {Head, Tail};
        _ -> error
    end,
    [Bin, Utf, Match, <<16#FF:8>>, <<42:8, 0:8>>, <<1:1, 0:1, 1:1>>].

-spec record_examples() -> list().
record_examples() ->
    P1 = #person{name = <<"Ada">>, age = 36, tags = [alpha, beta]},
    P2 = P1#person{name = <<"Grace">>, age = 85},
    [P1#person.name, P2#person.age, ?DEFAULT_LIMIT, ?TAG_LIST].

-spec classify(non_neg_integer()) -> atom().
classify(Age) ->
    if
        Age =:= 0 -> newborn;
        Age < 13 -> child;
        Age < 18 -> teen;
        true -> adult
    end.

-spec clause_demo(atom(), term()) -> term().
clause_demo(Tag, Value) when Tag =:= ok ->
    {ok, Value};
clause_demo(Tag, Value) when Tag =:= error ->
    {error, Value};
clause_demo(Tag, Value) ->
    {Tag, Value}.

-spec fun_demo(list()) -> list().
fun_demo(List) ->
    Fun = fun(X) when is_integer(X) -> X * 2;
             (X) -> X
          end,
    lists:map(Fun, List).

-spec receive_demo() -> atom().
receive_demo() ->
    Self = self(),
    Self ! {ping, 1},
    receive
        {ping, N} when N > 0 ->
            ping;
        {ping, _} ->
            pong
    after
        1000 ->
            timeout
    end.

-spec try_demo(term()) -> term().
try_demo(Value) ->
    try
        maybe_crash(Value)
    of
        ok -> ok
    catch
        error:Reason ->
            {error, Reason};
        throw:Reason ->
            {throw, Reason}
    after
        cleanup
    end.

maybe_crash(ok) ->
    ok;
maybe_crash(_) ->
    erlang:error(badarg).

cleanup ->
    ok.

-spec list_demo(list()) -> list().
list_demo(List) ->
    [X || X <- List, X > 0, X =< 10].

-spec map_demo() -> map().
map_demo() ->
    #{name => <<"Example">>,
      age => 12,
      tags => [alpha, beta],
      nested => #{one => 1, two => 2},
      atom => ok,
      quoted => 'quoted atom',
      number => 2#1011,
      base => 16#ff,
      float => 3.14}.

-spec macro_demo() -> term().
macro_demo() ->
    ?DEFAULT_LIMIT + 1.

-ifdef(TEST).
-spec test_only() -> atom().
test_only() ->
    test.
-else.
-spec test_only() -> atom().
test_only() ->
    prod.
-endif.

-spec driver() -> ok.
driver() ->
    Values = [new("Ada Lovelace", 36), new("Grace Hopper", 85)],
    Rendered = [render(V) || V <- Values],
    io:format("~p~n", [Rendered]),
    io:format("~p~n", [sum([1, 2, 3, 4])]),
    io:format("~p~n", [average([1, 2, 3, 4])]),
    io:format("~p~n", [bit_examples()]),
    io:format("~p~n", [record_examples()]),
    io:format("~p~n", [classify(18)]),
    io:format("~p~n", [clause_demo(ok, done)]),
    io:format("~p~n", [fun_demo([1, 2, 3, foo])]),
    io:format("~p~n", [receive_demo()]),
    io:format("~p~n", [try_demo(ok)]),
    ok.
