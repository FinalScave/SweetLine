(* OCaml example: modules, records, variants, objects, and attributes. *)

[@@@ocaml.warning "-27-32-33"]

module type SERIALIZABLE = sig
  type t
  val to_string : t -> string
end

module Point : SERIALIZABLE = struct
  type t = {
    x : float;
    y : float;
  }

  let origin = { x = 0.0; y = 0.0 }

  let make x y = { x; y }

  let distance a b =
    let dx = a.x -. b.x in
    let dy = a.y -. b.y in
    sqrt (dx *. dx +. dy *. dy)

  let to_string p =
    Printf.sprintf "(%g, %g)" p.x p.y
end

type color =
  | Red
  | Green
  | Blue
  | RGB of int * int * int

type 'a tree =
  | Leaf
  | Node of 'a tree * 'a * 'a tree

exception Parse_error of string

let rec map_tree f = function
  | Leaf -> Leaf
  | Node (left, value, right) ->
      Node (map_tree f left, f value, map_tree f right)

let rec fold_tree f acc = function
  | Leaf -> acc
  | Node (left, value, right) ->
      let acc_left = fold_tree f acc left in
      let acc_value = f acc_left value in
      fold_tree f acc_value right

let describe_color = function
  | Red -> "red"
  | Green -> "green"
  | Blue -> "blue"
  | RGB (r, g, b) ->
      Printf.sprintf "rgb(%d, %d, %d)" r g b

let parse_color = function
  | "red" -> Ok Red
  | "green" -> Ok Green
  | "blue" -> Ok Blue
  | text ->
      let parts = String.split_on_char ',' text in
      match parts with
      | [r; g; b] ->
          (try
             let r = int_of_string r in
             let g = int_of_string g in
             let b = int_of_string b in
             Ok (RGB (r, g, b))
           with Failure _ ->
             Error (Parse_error ("invalid RGB value: " ^ text)))
      | _ -> Error (Parse_error ("unknown color: " ^ text))

class virtual counter start = object (self)
  val mutable value = start

  method value = value

  method private bump delta =
    value <- value + delta;
    value

  method tick = self#bump 1
  method reset n = value <- n
end

class logging_counter start = object
  inherit counter start

  method tick_and_log =
    let next = self#tick in
    Printf.printf "tick -> %d\n%!" next;
    next
end

let apply_serializable (type a) (module S : SERIALIZABLE with type t = a) (value : a) =
  S.to_string value

let rec sum_tree = function
  | Leaf -> 0
  | Node (left, value, right) ->
      sum_tree left + value + sum_tree right

let rec build_tree values =
  match values with
  | [] -> Leaf
  | [x] -> Node (Leaf, x, Leaf)
  | x :: xs ->
      let left = build_tree xs in
      Node (left, x, Leaf)

let handle_color text =
  match parse_color text with
  | Ok color -> describe_color color
  | Error (Parse_error message) -> "error: " ^ message

let quoted_banner =
  {demo|OCaml raw strings keep "quotes", {braces}, and | pipes intact.|demo}

let demo_points () =
  let p1 = Point.make 10.5 20.25 in
  let p2 = Point.make 13.5 24.75 in
  let d = Point.distance p1 p2 in
  Printf.printf "distance = %g\n" d;
  d

let demo_tree () =
  let tree = Node (Node (Leaf, 1, Leaf), 2, Node (Leaf, 3, Leaf)) in
  let mapped = map_tree (fun x -> x * 2) tree in
  let total = sum_tree mapped in
  Printf.printf "tree total = %d\n" total;
  total

let demo_objects () =
  let c = new logging_counter 10 in
  ignore (c#tick_and_log);
  c#reset 7;
  ignore (c#tick);
  c#value

let rec repeat n f x =
  if n <= 0 then x else repeat (n - 1) f (f x)

let rec factorial n =
  if n <= 1 then 1 else n * factorial (n - 1)

let () =
  let _ = demo_points () in
  let _ = demo_tree () in
  let _ = demo_objects () in
  let _ = handle_color "12,34,56" in
  let _ = handle_color "unknown" in
  let nums = [1; 2; 3; 4; 5] in
  let total = List.fold_left ( + ) 0 nums in
  let text = String.concat ", " (List.map string_of_int nums) in
  Printf.printf "nums=[%s] total=%d\n" text total;
  assert (factorial 6 = 720);
  begin
    match build_tree nums with
    | Leaf -> Printf.printf "empty tree\n"
    | Node (_, v, _) -> Printf.printf "root=%d\n" v
  end;
  let _ =
    try
      int_of_string "not-an-int"
    with Failure _ ->
      -1
  in
  ()
