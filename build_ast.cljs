(ns northcode.funlang2.ast_builder
  (:require [cljs.reader :as reader]))

(def args (drop 3 process.argv))

(def fs (js/require "fs"))

(def test-node {:name "expr"
                :types [
                {:name "bin_exp"
                :fields [
                         ["expr" "left"]
                         ["token" "operator"]
                         ["expr" "right"]
                         ]}
                {:name "integer"
                :fields [
                         ["int" "value"]
                         ]}
                        
                        ]})

(defn make-fields [node]
  (let [fields (:fields node)]
    (apply str (map (fn [field]
              (str (first field) "* " (second field) ";"))
            fields))))

(defn make-type [type]
    (apply str "\nstruct " (:name type) " {"
           (make-fields type)
           "};"))

(defn make-types [node]
  (let [types (:types node)]
    (apply str
   (map make-type types)))) 

(defn make-node [node]
  (apply str
         (make-types node)
         "\nstruct " (:name node) "{"
         "\nenum " (:name node) "_types {"
         (apply str (map #(str "t_" (:name node) "_" (:name %) ", ") (:types node)))
         "} type; "
         "\nunion {"
         (apply str (map #(str (:name %) " d_" (:name %) "; ") (:types node)))
         "}"
         "};"))


;; (println (make-node (reader/read-string (.readFileSync fs "ast.edn"))))
(println args)
(doseq [file args] (.readFile fs file (fn [err, data]
                                     (println (make-node (reader/read-string (str data))))
                                     )))
