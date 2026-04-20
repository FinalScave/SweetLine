#lang racket

;; Racket example: module forms, macros, structs, classes, and matching.
#| This block comment is nested:
   #| nested comment |#
   It should stay comment text.
|#

(require racket/list
         racket/match
         racket/string
         racket/class
         rackunit)

(provide point? point-x point-y point
         segment? segment-start segment-end segment
         distance segment-length normalize-point
         summarize-points define-stable-vector)

(struct point (x y)
  #:transparent
  #:mutable)

(struct segment (start end)
  #:transparent)

(define-syntax-rule (define-stable-vector name values ...)
  (define name (vector values ...)))

(define (square n)
  (* n n))

(define (distance a b)
  (define dx (- (point-x a) (point-x b)))
  (define dy (- (point-y a) (point-y b)))
  (sqrt (+ (square dx) (square dy))))

(define (normalize-point p)
  (match p
    [(point x y)
     (point (if (zero? x) 0.0 x)
            (if (zero? y) 0.0 y))]
    [_ p]))

(define (segment-length s)
  (match s
    [(segment start end)
     (distance start end)]))

(define (summarize-points ps)
  (define values
    (for/list ([p (in-list ps)]
               #:when (point? p))
      (format "~a,~a" (point-x p) (point-y p))))
  (string-join values " | "))

(define (parse-point text)
  (match (string-split text ",")
    [(list x y)
     (point (string->number x) (string->number y))]
    [_ (error 'parse-point "expected x,y text")]))

(define logger%
  (class object%
    (super-new)
    (init-field [prefix "demo"])
    (define/public (emit msg)
      (displayln (string-append prefix ": " msg)))
    (define/public (emit-point p)
      (emit (format "point ~a,~a" (point-x p) (point-y p))))))

(define (make-logger prefix)
  (new logger% [prefix prefix]))

(define (run-demo)
  (define logger (make-logger "racket"))
  (define p1 (point 10 20))
  (define p2 (point 13 24))
  (define s (segment p1 p2))
  (define nums '(1 2 3 4 5))
  (define log-lines
    (summarize-points (list p1 p2 (normalize-point (point 0 8)))))
  (send logger emit log-lines)
  (send logger emit-point p1)
  (define-stable-vector demo-values 1 2 3 4)
  (define folded
    (for/fold ([acc 0])
              ([n (in-list nums)])
      (+ acc n)))
  (define with-params
    (parameterize ([current-output-port (current-output-port)])
      (displayln (format "folded=~a" folded))
      folded))
  (define quasi
    `(segment ,(point-x p1) ,@(list (point-y p1) (point-y p2))))
  (check-equal? (segment-length s) 5)
  (check-true (point? (parse-point "7,9")))
  (check-equal? with-params 15)
  (values log-lines quasi))

(define (run-analytics data)
  (match data
    [(list* first rest)
     (define count (length rest))
     (define total (apply + data))
     (define average (/ total (max 1 (length data))))
     (values first count average)]
    [_ (values #f 0 0)]))

(module+ test
  (define-values (lines quasi) (run-demo))
  (check-true (string-contains? lines "10,20"))
  (check-equal? quasi '(segment 10 20 24))
  (check-equal? (run-analytics '(2 4 6 8)) (values 2 3 5))

  #| Another nested comment block.
     #| deeper |#
     The reader should ignore it.
  |#

  (define payload
    (hash 'name "sample"
          'enabled #t
          'threshold 42
          #:mode 'fast))
  (check-equal? (hash-ref payload 'threshold) 42)
  (check-true (hash-ref payload 'enabled))
  (define text #<<EOF
Racket heredoc-like multiline text can be modeled as a string literal here.
It should exercise multi-line string handling and punctuation nearby.
EOF
    )
  (check-true (string-contains? text "multiline"))
  (define regex-match (regexp-match #px"racket" "hello racket"))
  (check-true (pair? regex-match))
  (define byte-str #"racket-bytes")
  (check-equal? (bytes-length byte-str) 13)
  (define quoted-sym 'hello-world)
  (check-equal? quoted-sym 'hello-world)
  (define kw-demo
    (list #:alpha 1 #:beta 2 #:gamma 3))
  (check-equal? kw-demo (list #:alpha 1 #:beta 2 #:gamma 3))
  (void))
