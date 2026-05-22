from ./math/tree import Pair
from lib/result import (Result, safe_div)
import (lib/list/map, lib/list/fold)
import ./effects/io as effects_io

#{
This file exercises the Bend syntax highlighter with Imp and Fun forms.
The code is compact, but it includes data definitions, folds, bends, and HVM snippets.
#}

type Option(T):
  Some { value: T }
  None

type Result(T):
  Ok { value: T }
  Err { message: String }

type Tree(T):
  Node { value: T, ~left: Tree(T), ~right: Tree(T) }
  Leaf { value: T }

object Pair(A, B) {
  fst: A,
  snd: B
}

object Pixel {
  x: u24,
  y: u24,
  shade: u24
}

def checked clamp(value: u24, low: u24, high: u24) -> u24:
  if value < low:
    return low
  elif value > high:
    return high
  else:
    return value

def Option/map(opt: Option(A), fn: A -> B) -> Option(B):
  match opt:
    case Option/Some:
      return Option/Some { value: fn(opt.value) }
    case Option/None:
      return Option/None

def Result/bind(res: Result(A), next: A -> Result(B)) -> Result(B):
  match res:
    case Result/Ok:
      nxt = undefer(next)
      return nxt(res.value)
    case Result/Err:
      return Result/Err { message: res.message }

def Result/wrap(value: A) -> Result(A):
  return Result/Ok { value: value }

def checked distance(a: Pair(f24, f24), b: Pair(f24, f24)) -> f24:
  open Pair: a
  open Pair: b
  dx = b.fst - a.fst
  dy = b.snd - a.snd
  return (dx * dx + dy * dy) ** 0.5

def tree_sum(tree: Tree(u24)) -> u24:
  fold tree:
    case Tree/Node:
      return tree.value + tree.left + tree.right
    case Tree/Leaf:
      return tree.value

def tree_depth(tree: Tree(u24)) -> u24:
  match tree:
    case Tree/Node:
      left = tree_depth(tree.left)
      right = tree_depth(tree.right)
      return 1 + max(left, right)
    case Tree/Leaf:
      return 0

def build_balanced(depth: u24, seed: u24) -> Tree(u24):
  bend d = 0, value = seed:
    when d < depth:
      left = fork(d + 1, value * 2)
      right = fork(d + 1, value * 2 + 1)
      tree = Tree/Node { value: value, left: left, right: right }
    else:
      tree = Tree/Leaf { value: value }
  return tree

def make_pixels(width: u24, height: u24) -> Tree(Pixel):
  total = width * height
  bend index = 0:
    when index < total:
      next = index + 1
      color = fork(next)
    else:
      px = index % width
      py = index / width
      color = Pixel { x: px, y: py, shade: clamp(px ^ py, 0, 255) }
  return color

def render(depth: u24) -> Any:
  tree = build_balanced(depth, 1)
  pixels = make_pixels(16, 16)
  return (tree_sum(tree), pixels)

def choose_status(code: u24) -> String:
  switch code:
    case 0:
      return "ready"
    case 1:
      return "busy"
    case _:
      return "unknown"

def with_result(a: u24, b: u24) -> Result(u24):
  with Result:
    left <- safe_div(a, b)
    right <- safe_div(a + b, 2)
    return wrap(left + right)

def use_lambda(list: List(u24)) -> List(u24):
  bump = lambda x: x + 1
  return map(list, bump)

def unscoped_once($x):
  return $x

def symbol_and_char() -> (u24, u24, String):
  tag = `run`
  letter = 'A'
  text = "Bend docs live at https://github.com/HigherOrderCO/Bend"
  return (tag, letter, text)

hvm link_ports: (a (b *)) & (c a) ~ (d e) & (e b) ~ (d c)
hvm u24_to_u24 -> (u24 -> u24): ($([u24] ret) ret)

Identity : A -> A
Identity x = x

MakePair : A -> B -> Pair(A, B)
MakePair fst snd = Pair { fst: fst, snd: snd }

SwapPair : Pair(A, B) -> Pair(B, A)
SwapPair pair =
  let opened = pair
  (Pair opened.snd opened.fst)

type FunList = Nil | (Cons head ~tail)

FunLength : FunList -> u24
FunLength Nil = 0
FunLength (Cons head tail) = (+ 1 (FunLength tail))

FunMapMaybe : Option(A) -> (A -> B) -> Option(B)
FunMapMaybe Option/None fn = Option/None
FunMapMaybe (Option/Some value) fn = (Option/Some (fn value))

FunTree : u24 -> Tree(u24)
FunTree depth = bend d = 0, value = 1 {
  when (< d depth): (Tree/Node value (fork (+ d 1) (* value 2)) (fork (+ d 1) (+ (* value 2) 1)))
  else: (Tree/Leaf value)
}

FunWith : Result(u24)
FunWith = with Result {
  ask left = (safe_div 8 2)
  ask right = (safe_div 9 3)
  (wrap (+ left right))
}

def main() -> String:
  tree = build_balanced(4, 1)
  total = tree_sum(tree)
  status = choose_status(total % 3)
  return status
