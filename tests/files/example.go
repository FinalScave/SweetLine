// Go sample
package main

import (
	"fmt"
	"strings"
	"sync"
)

// struct and interface
type Animal interface {
	Speak() string
}

type Dog struct {
	Name string
	Age  int
}

func (d *Dog) Speak() string {
	return fmt.Sprintf("%s says Woof!", d.Name)
}

type Pair[T comparable] struct {
	First  T
	Second T
}

type Cache[K comparable, V any] struct {
	data map[K]V
}

func NewPair[T comparable](a, b T) Pair[T] {
	return Pair[T]{First: a, Second: b}
}

// constant and iota
const (
	StatusOK    = iota
	StatusError
	StatusPending
)

func process(items []string, count int) (result int, err error) {
	total := len(items)
	if total == 0 {
		return 0, fmt.Errorf("empty list")
	}

	result := make([]string, 0, total)
	for _, item := range items {
		upper := strings.ToUpper(item)
		result = append(result, upper)
	}

	println(total)
	return cap(result), nil
}

func main() {
	dog := &Dog{Name: "Buddy", Age: 3}
	fmt.Println(dog.Speak())

	// numeric literal
	hex := 0xFF
	bin := 0b1010
	oct := 0o77
	pi := 3.14
	comp := 2.0 + 3i

	ok := true
	var nothing *int = nil

	// goroutine and channel
	ch := make(chan string, 10)
	var wg sync.WaitGroup

	go func() {
		defer wg.Done()
		ch <- "hello"
	}()

	// raw string
	raw := `line 1
line 2`

	// switch statement
	switch {
	case ok && nothing == nil:
		fmt.Println(raw)
	default:
		close(ch)
		panic("unexpected")
	}

	/* multi-line comment
	   can span multiple lines */
	_ = false
}
