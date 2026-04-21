import { Fragment } from "react";

type Tone = "ok" | "warn" | "off";

type QuickLink = {
  readonly id: string;
  readonly label: string;
  readonly href: string;
};

interface PanelProps {
  title: string;
  count: number;
  tone?: Tone;
  links: readonly QuickLink[];
}

interface HeaderProps {
  title: string;
  subtitle?: string;
  tone: Tone;
}

const palette: Record<Tone, string> = {
  ok: "#15803d",
  warn: "#b45309",
  off: "#475569"
};

const links = [
  { id: "syntax", label: "Syntax JSON", href: "https://sweetline.dev/docs/syntax" },
  { id: "tests", label: "Highlight tests", href: "https://sweetline.dev/docs/tests" },
  { id: "routing", label: "Routing", href: "https://sweetline.dev/docs/routing" }
] as const satisfies readonly QuickLink[];

function formatCount(value: number): string {
  return value.toLocaleString("en-US");
}

function Header({ title, subtitle, tone }: HeaderProps): JSX.Element {
  const accent = palette[tone];

  return (
    <header className="panel-header" data-tone={tone} style={{ borderColor: accent, color: accent }}>
      <div className="panel-copy">
        <p className="eyebrow">typed TSX</p>
        <h2>{title}</h2>
        <p className="panel-subtitle">{subtitle ?? "No subtitle"}</p>
      </div>
      <svg className="panel-graphic" viewBox="0 0 24 24" aria-hidden="true">
        <path d="M4 14l4-4 4 4 8-8" />
      </svg>
    </header>
  );
}

function QuickLinkList({ items }: { items: readonly QuickLink[] }): JSX.Element {
  return (
    <ul className="quick-links">
      <li className="quick-link">
        <a href={items[0].href}>{items[0].label}</a>
      </li>
      <li className="quick-link">
        <a href={items[1].href}>{items[1].label}</a>
      </li>
      <li className="quick-link">
        <a href={items[2].href}>{items[2].label}</a>
      </li>
    </ul>
  );
}

function Panel({ title, count, tone = "ok", links }: PanelProps): JSX.Element {
  const badge = tone === "warn" ? "needs review" : "ready";

  return (
    <article className="panel" data-title={title}>
      <Header title={title} subtitle="Routes and tokens stay aligned." tone={tone} />
      <div className="panel-body">
        {/*
          TS expressions inside JSX should still get the host TypeScript syntax rules.
        */}
        <p className="panel-stat">
          <strong>{formatCount(count)}</strong>
          <span className="panel-badge">{badge}</span>
        </p>
        <div className="panel-actions">
          <button type="button" className="ghost" aria-pressed={tone === "warn"}>
            inspect
          </button>
          <button type="button" className="solid" data-kind="primary">
            open
          </button>
        </div>
        <QuickLinkList items={links} />
      </div>
      <footer className="panel-footer">
        <small>{links[0].href}</small>
      </footer>
    </article>
  );
}

type DashboardProps = {
  readonly user: {
    readonly name: string;
    readonly team: "core" | "syntax";
  };
};

export default function TsxDashboard({ user }: DashboardProps): JSX.Element {
  const panelTone: Tone = user.team === "core" ? "ok" : "warn";
  const stats = {
    compiled: 24,
    routed: 2,
    reviewed: 7
  } as const;

  return (
    <>
      <section className="tsx-dashboard" data-user={user.name}>
        <Header
          title="SweetLine TSX"
          subtitle={`Owner: ${user.name}`}
          tone={panelTone}
        />

        <div className="summary-strip">
          <span className="summary-pill">{stats.compiled}</span>
          <span className="summary-pill">{stats.routed}</span>
          <span className="summary-pill">{stats.reviewed}</span>
        </div>

        <main className="panel-grid">
          <Panel title="Compilation" count={stats.compiled} tone="ok" links={links} />
          <Panel title="Routing" count={stats.routed} tone="warn" links={links} />
          <Panel title="Review" count={stats.reviewed} tone="off" links={links} />
        </main>

        <aside className="status-callout" style={{ backgroundColor: "#f8fafc", borderColor: palette[panelTone] }}>
          <p className="status-copy">
            Keep .react.tsx routed here while bare .tsx continues to go through the existing TypeScript syntax.
          </p>
          <Fragment>
            <span className="status-note">This file is intentionally named example.tsx for exact-name routing.</span>
          </Fragment>
        </aside>

        <footer className="tsx-footer">
          <small data-team={user.team}>Latest docs: https://sweetline.dev/docs/tsx</small>
        </footer>
      </section>
    </>
  );
}
