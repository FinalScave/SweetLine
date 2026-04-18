(ns sweetline.example.clj
  (:require
   [clojure.string :as str]
   [clojure.set :as set]
   [clojure.walk :as walk]
   [clojure.java.io :as io])
  (:import
   (java.time Instant)
   (java.util UUID)))

; Clojure example coverage
; The file is intentionally long enough to cover nested forms, metadata,
; reader macros, strings, chars, numbers, maps, vectors, sets, and regexes.

(def ^:private default-tags
  #{:alpha :beta :gamma :release/alpha})

(def ^String ^:private platform-name "SweetLine")

(defrecord Person [name age tags])

(deftype DemoBox [^String text]
  Object
  (toString [_]
    text))

(defprotocol Renderable
  (render [this])
  (render-with [this opts]))

(extend-protocol Renderable
  Person
  (render [this]
    (str (:name this) "@" (:age this)))
  (render-with [this opts]
    (str (render this) " " (pr-str opts))))

(defn normalize-name
  [value]
  (-> value
      str/trim
      str/lower-case
      (str/replace #"[^a-z0-9-]+" "-")
      (str/replace #"--+" "-")))

(defn parse-tags
  [value]
  (->> (str/split value #",")
       (map str/trim)
       (remove str/blank?)
       (map keyword)
       set))

(defn maybe-person
  [name age tags]
  (when-let [clean-name (not-empty (normalize-name name))]
    (->Person clean-name age tags)))

(defn tagged-person
  [name age]
  (let [identifier (keyword (normalize-name name))
        tags (conj default-tags identifier)]
    (maybe-person name age tags)))

(defn with-meta-sample
  []
  ^{:source "example" :kind :sample}
  {:created #inst "2024-01-01T00:00:00.000-00:00"
   :id #uuid "123e4567-e89b-12d3-a456-426614174000"
   :namespaced-keyword ::sample/name
   :quoted-symbol 'sweetline.example/value
   :qualified-symbol clojure.core/+})

(defn transform-form
  [form]
  (walk/postwalk
   (fn [node]
     (cond
       (keyword? node) (keyword (namespace node) (name node))
       (string? node) (str/upper-case node)
       :else node))
   form))

(defn seq->summary
  [items]
  (let [pairs (map-indexed (fn [idx item] [idx item]) items)]
    (loop [remaining pairs
           acc []]
      (if-let [[idx item] (first remaining)]
        (recur (rest remaining)
               (conj acc {:index idx
                          :item item
                          :quoted `(sample ~item)
                          :ratio 22/7
                          :base-two 2r1011
                          :hex 0x2A
                          :decimal 1_024
                          :bigint 42N
                          :bigdec 3.14M}))
        acc))))

(defn classify-age
  [age]
  (case age
    0 :newborn
    1 :infant
    2 :toddler
    3 :child
    :adult))

(defn choose-greeting
  [person]
  (cond
    (nil? person) "Nobody"
    (= (:age person) 0) "Hello, tiny human."
    (:name person) (str "Hello, " (:name person) ".")
    :else "Hello, stranger."))

(defn format-person
  [person]
  (if-let [name (:name person)]
    (str "#<Person " name " "
         (classify-age (:age person)) " "
         (pr-str (:tags person)) ">")
    "nil-person"))

(defn render-data
  [items]
  (->> items
       (map (fn [item]
              (if (string? item)
                (str "\"" item "\"")
                (pr-str item))))
       (str/join ", ")))

(defn read-resource
  [path]
  (with-open [r (io/reader path)]
    (doall (line-seq r))))

(defn print-lines
  [items]
  (doseq [line items]
    (println line)))

(defn dispatch-example
  [x]
  (let [m {:a 1 :b 2
           :nested {:c 3 :d [1 2 3]}
           :tags #{:alpha :beta}}
        v [1 2 3 4]
        s #{:x :y :z}
        l '(1 2 3)
        chars [\a \newline \u03A9 \o141]]
    (cond
      (map? x) (merge m x)
      (vector? x) (concat v x)
      (set? x) (set/union s x)
      (seq? x) (concat l x)
      :else chars)))

(defn reader-macro-samples
  []
  [(let [m #{"alpha" "beta"}]
     (count m))
   (#(str "anon-" % "-" %2) "one" "two")
   (if true
     (list 'quote `syntax `(~'unquote ~'splicing))
     (list false nil))
   (map #(str % "-item") ["a" "b" "c"])])

(defn nested-forms
  [x]
  (let [base {:name "Ada"
              :age 36
              :tags (:tags (tagged-person "Ada" 36))
              :status (if (> x 3) :ready :waiting)}]
    (-> base
        (assoc :path (str "/tmp/" (normalize-name "Ada Lovelace")))
        (update :tags conj :release/alpha)
        (update :status identity))))

(defn -main
  [& args]
  (let [items (or (seq args) ["Alpha" "Beta" "Gamma"])
        persons [(tagged-person "Ada Lovelace" 36)
                 (tagged-person "Grace Hopper" 85)]
        chosen (maybe-person "Clojure Reader" 12 #{:alpha :beta})
        summary (seq->summary items)]
    (println (choose-greeting chosen))
    (println (render-data items))
    (println (format-person (first persons)))
    (println (pr-str (with-meta-sample)))
    (println (pr-str summary))
    (println (pr-str (transform-form '(+ 1 2 3))))
    (println (pr-str (dispatch-example [1 2 3])))
    (println (pr-str (reader-macro-samples)))
    (println (pr-str (nested-forms 4)))
    (println (str "Instant: " Instant/now))))
