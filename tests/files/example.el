;;; sweetline-example.el --- Emacs Lisp sample for SweetLine -*- lexical-binding: t; -*-

;;; Commentary:
;; This sample exercises the syntax with realistic package-style forms:
;; custom groups, defcustom, defvar, structs, classes, generic methods,
;; functions, macros, keywords, quoted symbols, and string/number literals.

;;; Code:

(require 'cl-lib)
(require 'subr-x)
(require 'eieio)

(defgroup sweetline-example nil
  "Sample package for SweetLine syntax coverage."
  :group 'applications
  :prefix "sweetline-example-")

(defcustom sweetline-example-max-results 25
  "Maximum number of entries to keep in the sample index."
  :type 'integer
  :group 'sweetline-example)

(defcustom sweetline-example-enable-cache t
  "Whether the sample keeps an in-memory cache."
  :type 'boolean
  :group 'sweetline-example)

(defconst sweetline-example-version "1.2.2"
  "Version string used by the sample package.")

(defconst sweetline-example--default-tags
  '("demo" "sample" "syntax")
  "Default tags attached to generated entries.")

(defvar sweetline-example--cache (make-hash-table :test 'equal)
  "Cache of sample entries keyed by normalized title.")

(defvar-local sweetline-example--buffer-state nil
  "Buffer-local state used when scanning sample text.")

(cl-defstruct sweetline-example-entry
  title
  tags
  score
  source
  timestamp)

(defclass sweetline-example-source ()
  ((name :initarg :name :initform "demo" :type string
         :documentation "Source name.")
   (root :initarg :root :initform default-directory :type string
         :documentation "Base directory.")
   (pattern :initarg :pattern :initform "\\.el\\'" :type string
            :documentation "File-name pattern."))
  "Object describing an input source.")

(cl-defgeneric sweetline-example-render (object)
  "Render OBJECT as a display string.")

(cl-defmethod sweetline-example-render ((object sweetline-example-source))
  (format "source:%s root:%s"
          (slot-value object 'name)
          (slot-value object 'root)))

(defmacro sweetline-example-with-temp-index (&rest body)
  "Execute BODY while building a temporary sample index."
  `(let ((index (make-hash-table :test 'equal)))
     ,@body
     index))

(defun sweetline-example-normalize-title (title)
  "Normalize TITLE for cache lookups."
  (let* ((trimmed (string-trim title))
         (downcased (downcase trimmed)))
    (replace-regexp-in-string "[^[:alnum:]]+" "-" downcased)))

(defun sweetline-example-score-tags (tags)
  "Assign a deterministic score for TAGS."
  (let ((score 0))
    (dolist (tag tags score)
      (pcase tag
        ("demo" (setq score (+ score 10)))
        ("syntax" (setq score (+ score 20)))
        ("sample" (setq score (+ score 5)))
        (_ (setq score (+ score 1))))))

(defun sweetline-example-make-entry (title score &optional source)
  "Create a new entry for TITLE with SCORE and SOURCE."
  (make-sweetline-example-entry
   :title title
   :tags (append sweetline-example--default-tags (list source))
   :score score
   :source (or source "buffer")
   :timestamp (format-time-string "%Y-%m-%dT%H:%M:%SZ" (current-time) t)))

(defun sweetline-example-format-entry (entry)
  "Format ENTRY as a single line."
  (let ((title (sweetline-example-entry-title entry))
        (score (sweetline-example-entry-score entry))
        (source (sweetline-example-entry-source entry)))
    (format "%s [%s] score=%d"
            title source score)))

(defun sweetline-example-index-line (line source)
  "Turn LINE into a sample entry from SOURCE."
  (let* ((title (sweetline-example-normalize-title line))
         (score (sweetline-example-score-tags (split-string line "[[:space:]]+" t)))
         (entry (sweetline-example-make-entry title score source)))
    (puthash title entry sweetline-example--cache)
    entry))

(defun sweetline-example-collect-lines ()
  "Collect non-empty lines from the current buffer."
  (save-excursion
    (goto-char (point-min))
    (let (lines)
      (while (re-search-forward "^\\([^;].*\\)$" nil t)
        (push (match-string 1) lines))
      (nreverse lines))))

(defun sweetline-example-build-index (&optional source)
  "Build an index from the current buffer or SOURCE."
  (let ((origin (or source "buffer"))
        (entries '()))
    (dolist (line (sweetline-example-collect-lines))
      (push (sweetline-example-index-line line origin) entries))
    (nreverse entries)))

(defun sweetline-example-find-entry (title)
  "Return the cached entry for TITLE."
  (gethash (sweetline-example-normalize-title title) sweetline-example--cache))

(defun sweetline-example-filter-entries (entries predicate)
  "Filter ENTRIES with PREDICATE."
  (cl-remove-if-not predicate entries))

(defun sweetline-example-save-report (entries file)
  "Write ENTRIES to FILE."
  (with-temp-buffer
    (insert (format "count=%d\n" (length entries)))
    (dolist (entry entries)
      (insert (sweetline-example-format-entry entry) "\n"))
    (write-region (point-min) (point-max) file nil 'silent)))

(defun sweetline-example-read-report (file)
  "Read FILE and return a list of raw lines."
  (condition-case err
      (with-temp-buffer
        (insert-file-contents file)
        (split-string (buffer-string) "\n" t))
    (file-error
     (message "Failed to read %s: %S" file err)
     nil)))

(defun sweetline-example-demo-data ()
  "Return sample data for ad hoc tests."
  (list
   '("First entry" :source "demo" :score 10)
   '("Second entry" :source "buffer" :score 8)
   '("Third entry" :source "file" :score 5)))

(defun sweetline-example-run (title)
  "Run the sample workflow for TITLE."
  (interactive "sTitle: ")
  (let* ((normalized (sweetline-example-normalize-title title))
         (entry (or (sweetline-example-find-entry title)
                    (sweetline-example-make-entry normalized 12 "interactive")))
         (rendered (sweetline-example-render
                    (sweetline-example-source :name "demo" :root default-directory))))
    (message "%s | %s" rendered (sweetline-example-format-entry entry))
    entry))

(defun sweetline-example-parse-buffer ()
  "Parse the current buffer and keep cache state."
  (let ((entries (sweetline-example-build-index "buffer")))
    (setq sweetline-example--buffer-state entries)
    entries))

(defun sweetline-example-reset-cache ()
  "Reset the sample cache."
  (interactive)
  (clrhash sweetline-example--cache)
  (setq sweetline-example--buffer-state nil)
  (message "Cache cleared"))

(defun sweetline-example-summarize ()
  "Summarize the current cache."
  (let ((keys '())
        (entries '()))
    (maphash
     (lambda (key value)
       (push key keys)
       (push value entries))
     sweetline-example--cache)
    (list :count (hash-table-count sweetline-example--cache)
          :keys (sort keys #'string<)
          :entries (nreverse entries))))

(defun sweetline-example-with-temp-buffer (text callback)
  "Call CALLBACK with TEXT inserted into a temporary buffer."
  (with-temp-buffer
    (insert text)
    (funcall callback (buffer-string))))

(defun sweetline-example-quote-demo ()
  "Return a quoted sample for syntax testing."
  '(sweetline-example example "quoted" :keyword))

(defun sweetline-example-function-ref ()
  "Return a function reference."
  #'sweetline-example-normalize-title)

(defun sweetline-example-choose-mode (mode)
  "Return a mode label for MODE."
  (pcase mode
    ('fast "fast")
    ('accurate "accurate")
    (_ "unknown")))

(defun sweetline-example-lookup (title)
  "Look up TITLE and report the cached score."
  (let ((entry (sweetline-example-find-entry title)))
    (if entry
        (format "%s => %d"
                (sweetline-example-entry-title entry)
                (sweetline-example-entry-score entry))
      "missing")))

(provide 'sweetline-example)

;;; example.el ends here
