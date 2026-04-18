# R sample for SweetLine
# Official docs and package references:
# https://stat.ethz.ch/CRAN/doc/manuals/r-patched/R-lang.pdf
# https://stat.ethz.ch/CRAN/doc/manuals/r-devel/fullrefman.pdf

normalize_name <- function(name, fallback = "unknown") {
    if (is.null(name) || length(name) == 0L) {
        return(fallback)
    }

    cleaned <- trimws(tolower(as.character(name)))
    cleaned <- gsub("[^a-z0-9._-]+", "_", cleaned)
    cleaned <- gsub("_+", "_", cleaned)
    if (nchar(cleaned) == 0L) {
        fallback
    } else {
        cleaned
    }
}

clamp_value <- function(value, min_value = 0, max_value = 100) {
    if (value < min_value) {
        return(min_value)
    }
    if (value > max_value) {
        return(max_value)
    }
    value
}

rolling_mean <- function(values, window = 3L) {
    if (length(values) < window) {
        return(numeric())
    }

    result <- numeric(0)
    index <- 1L
    while (index <= length(values) - window + 1L) {
        slice <- values[index:(index + window - 1L)]
        result <- c(result, mean(slice))
        index <- index + 1L
    }
    result
}

summarize_series <- function(values, label = "series", ...) {
    stats <- list(
        label = label,
        count = length(values),
        total = sum(values),
        average = mean(values),
        median = stats::median(values),
        spread = stats::sd(values),
        minimum = min(values),
        maximum = max(values),
        extra = list(...)
    )

    stats$label <- normalize_name(stats$label)
    stats
}

format_report <- function(records, prefix = "report") {
    lines <- character(0)
    for (i in seq_along(records)) {
        record <- records[[i]]
        line <- sprintf(
            "%s[%d] %s => %.2f",
            prefix,
            i,
            normalize_name(record$name),
            clamp_value(record$score)
        )
        lines <- c(lines, line)
    }
    paste(lines, collapse = "\n")
}

make_bucket <- function(values, threshold = 0.5) {
    buckets <- list(low = numeric(), high = numeric())
    for (value in values) {
        if (value >= threshold) {
            buckets$high <- c(buckets$high, value)
        } else {
            buckets$low <- c(buckets$low, value)
        }
    }
    buckets
}

`plus one` <- function(x) {
    x + 1
}

`trim-and-squash` <- function(text) {
    gsub("\\s+", " ", trimws(text))
}

demo_values <- c(1, 2, 3, 4, 5, 8, 13, 21, 34, 55)
demo_flags <- c(TRUE, FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE)
demo_names <- c("alpha", "beta", "gamma", "delta", "epsilon")

demo_frame <- data.frame(
    name = demo_names,
    score = c(12.5, 88.25, 44.0, 91.75, 63.0),
    category = factor(c("low", "high", "medium", "high", "medium")),
    stringsAsFactors = FALSE
)

demo_list <- list(
    counts = demo_values,
    active = demo_flags,
    info = list(
        created = Sys.Date(),
        owner = "SweetLine",
        url = "https://example.com/report"
    )
)

demo_matrix <- matrix(
    c(1, 2, 3, 4, 5, 6),
    nrow = 2,
    ncol = 3,
    byrow = TRUE
)

demo_formula <- score ~ category
demo_model <- lm(demo_formula, data = demo_frame)
demo_summary <- summarize_series(demo_values, label = "Demo Series", source = "example.r")

message("model class: ", class(demo_model)[1])
warning("this is only a syntax sample")

for (value in demo_values) {
    if (value %% 2L == 0L) {
        next
    }
    if (value > 30L) {
        break
    }
    cat("odd value:", value, "\n")
}

repeat {
    if (length(demo_values) > 0L) {
        break
    }
}

report_rows <- lapply(seq_len(nrow(demo_frame)), function(index) {
    row <- demo_frame[index, ]
    list(
        name = row$name,
        score = row$score,
        category = row$category
    )
})

bucketed <- make_bucket(demo_values / max(demo_values), threshold = 0.4)
rolling <- rolling_mean(demo_values, window = 4L)
clamped <- vapply(demo_values, clamp_value, numeric(1), min_value = 2, max_value = 50)
shifted <- sapply(demo_values, `plus one`)
clean_names <- vapply(demo_names, `trim-and-squash`, character(1))

package_result <- base::paste(clean_names, collapse = ", ")
stats_result <- utils::head(demo_frame, 3)
custom_result <- stats:::sd(demo_values)

if (length(rolling) > 0L && all(demo_flags)) {
    cat("rolling mean:", paste(rolling, collapse = ", "), "\n")
} else if (any(demo_flags)) {
    cat("partial flags detected\n")
} else {
    cat("no flags set\n")
}

series_details <- list(
    summary = demo_summary,
    bucketed = bucketed,
    clamped = clamped,
    shifted = shifted,
    report = format_report(report_rows, prefix = "demo"),
    stats = stats_result,
    package_result = package_result,
    custom_result = custom_result,
    matrix_trace = sum(diag(demo_matrix)),
    special = c(NA, NaN, Inf, -Inf, NULL)
)

series_details$summary$hex <- 0xFF
series_details$summary$formula <- demo_formula
series_details$summary$note <- "See also https://cran.r-project.org"

print(series_details)
