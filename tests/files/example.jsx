import { Fragment, useId, useMemo } from "react";
import { Icons } from "./ui/icons";

const site = {
  owner: "SweetLine",
  baseUrl: "https://sweetline.dev/docs",
  status: "preview"
};

const cards = [
  {
    id: "intro",
    title: "Syntax coverage",
    href: "https://sweetline.dev/docs/jsx",
    tags: ["jsx", "routing", "tokens"],
    visits: 1280
  },
  {
    id: "tsx",
    title: "Typed props",
    href: "https://sweetline.dev/docs/tsx",
    tags: ["tsx", "types", "examples"],
    visits: 960
  }
];

function Avatar({ name, tone = "sunrise" }) {
  const accent = tone === "sunrise" ? "#d97706" : "#0369a1";

  return (
    <span
      className="avatar-shell"
      data-tone={tone}
      style={{ borderColor: accent, boxShadow: `0 0 0 2px ${accent}` }}
      title={`Profile for ${name}`}
    >
      {name.slice(0, 2).toUpperCase()}
    </span>
  );
}

function MetricRow({ label, value, href, highlight = false }) {
  return (
    <li className={highlight ? "metric metric-hot" : "metric"} data-label={label}>
      <a href={href} target="_blank" rel="noreferrer" aria-label={`${label} details`}>
        <span className="metric-label">{label}</span>
        <strong className="metric-value">{value.toLocaleString()}</strong>
      </a>
    </li>
  );
}

function Toolbar({ filters, activeFilter, onSelect }) {
  return (
    <div className="toolbar" role="tablist" aria-label="Dashboard filters">
      {/*
        JSX comments live inside braces and should still be tokenized as JS comments.
      */}
      <button type="button" className="ghost" data-active={activeFilter === "all"}>
        All
      </button>
      <Fragment>
        <span className="toolbar-label">Quick filters&nbsp;</span>
      </Fragment>
      <div className="toolbar-pills">
        <button
          type="button"
          className="pill"
          data-kind="summary"
          onClick={onSelect}
          aria-pressed={activeFilter === "summary"}
        >
          summary
        </button>
        <button
          type="button"
          className='pill'
          data-kind="focus"
          onClick={onSelect}
          aria-pressed={activeFilter === "focus"}
        >
          focus
        </button>
        <span className="toolbar-count">{filters.length}</span>
      </div>
    </div>
  );
}

function ArticleCard({ title, href, tags, visits, featured = false }) {
  const cardId = useId();

  return (
    <article className={featured ? "card card-featured" : "card"} data-card={cardId}>
      <header className="card-header">
        <Avatar name={title} tone={featured ? "sunrise" : "ocean"} />
        <div className="card-meta">
          <a href={href} className="card-link" rel="noopener">
            {title}
          </a>
          <small>{site.owner.toUpperCase()}</small>
        </div>
        <Icons.Check className="card-icon" aria-hidden="true" />
      </header>
      <div className="card-body">
        <p className="card-copy">
          Visit the docs at {href} and keep the output in sync with the shipped examples.
        </p>
        <ul className="tag-list">
          <li className="tag tag-static">{tags[0]}</li>
          <li className="tag tag-static">{tags[1]}</li>
          <li className="tag tag-static">{tags[2]}</li>
        </ul>
      </div>
      <footer className="card-footer">
        <span className="card-visits">{visits.toLocaleString()}</span>
        <span className="card-status">{site.status ?? "draft"}</span>
      </footer>
    </article>
  );
}

export default function Dashboard() {
  const activeFilter = "summary";
  const filters = ["all", "summary", "focus"];
  const updatedAt = new Date("2026-04-21T09:00:00Z");
  const summary = useMemo(() => {
    return {
      readers: 1842,
      examples: 2,
      ownerLabel: `${site.owner} UI`
    };
  }, []);

  return (
    <>
      <section className="dashboard" data-owner={site.owner}>
        <header className="hero" style={{ backgroundImage: "linear-gradient(135deg, #fef3c7, #dbeafe)" }}>
          <div className="hero-copy">
            <p className="eyebrow">real JSX syntax</p>
            <h1>{summary.ownerLabel}</h1>
            <p className="hero-summary">
              Highlight tags, fragments, attributes, comments, strings, and embedded expressions.
            </p>
          </div>
          <div className="hero-aside">
            <Avatar name="SL" tone="sunrise" />
            <p className="hero-date">{updatedAt.toLocaleDateString("en-US")}</p>
          </div>
        </header>

        <Toolbar filters={filters} activeFilter={activeFilter} onSelect={() => {}} />

        <main className="content-grid">
          <aside className="metrics">
            <h2>Highlights</h2>
            <ul className="metric-list">
              <MetricRow
                label="Readers"
                value={summary.readers}
                href="https://sweetline.dev/metrics/readers"
                highlight
              />
              <MetricRow
                label="Examples"
                value={summary.examples}
                href="https://sweetline.dev/metrics/examples"
              />
            </ul>
          </aside>

          <section className="cards">
            <ArticleCard {...cards[0]} featured />
            <ArticleCard {...cards[1]} />
          </section>
        </main>

        <footer className="dashboard-footer">
          <small data-url={site.baseUrl}>Source: {site.baseUrl}</small>
        </footer>
      </section>
    </>
  );
}
