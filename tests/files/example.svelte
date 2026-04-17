<script lang="ts">
  import { fade, fly } from 'svelte/transition';
  import { flip } from 'svelte/animate';
  import { onMount } from 'svelte';

  type Item = { id: number; label: string; status: 'active' | 'queued'; latency: number; };

  let query = '';
  let count = 0;
  let toast = '';
  let mounted = false;
  const tooltip = 'https://example.com/help';
  const docsUrl = 'https://example.com/svelte';
  const items: Item[] = [
    { id: 1, label: 'Deploy queue', status: 'active', latency: 128 },
    { id: 2, label: 'Search cluster', status: 'queued', latency: 264 },
    { id: 3, label: 'Billing sync', status: 'active', latency: 94 }
  ];

  $: filteredItems = items.filter((item) => item.label.toLowerCase().includes(query.toLowerCase()));
  $: heading = `${filteredItems.length} services ready`;
  $: highlighted = filteredItems.find((item) => item.status === 'active');

  function increment() { count += 1; }
  function select(id: number) { toast = `Selected ${id}`; }
  function itemUrl(id: number) { return `https://example.com/items/${id}`; }
  async function loadSnapshot() { return Promise.resolve({ status: 'ok', docs: docsUrl }); }

  function tooltipAction(node: HTMLElement, value: string) {
    node.dataset.tooltip = value;
    return { update(next: string) { node.dataset.tooltip = next; } };
  }

  onMount(() => {
    mounted = true;
    console.log('mounted svelte demo', docsUrl);
  });
</script>

<svelte:head>
  <title>SweetLine Svelte demo</title>
  <meta name="description" content="https://example.com/docs" />
</svelte:head>

<!-- Svelte demo comment: https://example.com/docs -->
<section class="dashboard" data-mounted={mounted}>
  <svelte:window on:keydown={increment} />

  <header class="dashboard__header">
    <div>
      <p class="dashboard__eyebrow">SweetLine Svelte demo</p>
      <h1>{heading}</h1>
      <p class="dashboard__subtitle">Explore docs at <a href={docsUrl} target="_blank" rel="noreferrer">{docsUrl}</a></p>
    </div>

    <button class:active={count > 0} class="dashboard__button" on:click={increment} use:tooltipAction={tooltip}>
      Count {count}
    </button>
  </header>

  <label class="dashboard__search">
    <span>Filter items</span>
    <input bind:value={query} placeholder="Filter services" />
  </label>

  {#await loadSnapshot()}
    <p class="dashboard__status">Loading snapshot...</p>
  {:then snapshot}
    <p class="dashboard__status">Snapshot {snapshot.status} from {snapshot.docs}</p>
  {:catch error}
    <p class="dashboard__status">Error: {error.message}</p>
  {/await}

  {#if filteredItems.length > 0}
    <ul class="dashboard__grid" animate:flip>
      {#each filteredItems as item, index (item.id)}
        <li class:selected={index === 0} class:warning={item.status === 'queued'} class="dashboard__card" transition:fly={{ y: 8, duration: 180 }}>
          <div class="dashboard__card-head">
            <div>
              <h2>{item.label}</h2>
              <p>{item.status} service with {item.latency}ms latency.</p>
            </div>
            <a href={itemUrl(item.id)} target="_blank" rel="noreferrer">Open docs</a>
          </div>

          <div class="dashboard__meta">
            <strong>{item.latency}ms</strong>
            <button on:click={() => select(item.id)}>Select</button>
          </div>

          {#if highlighted && highlighted.id === item.id}
            <p in:fade={{ duration: 140 }} class="dashboard__note">Highlighted service for follow-up.</p>
          {/if}
        </li>
      {/each}
    </ul>
  {:else if count === 0}
    <p class="dashboard__status">No services yet.</p>
  {:else}
    <p class="dashboard__status">No matching services for "{query}".</p>
  {/if}

  {#key toast}
    {#if toast}<div class="dashboard__toast" transition:fade>{{ toast }}</div>{/if}
  {/key}

  {@html "<strong>https://example.com/rendered</strong>"}
  {@debug count, query}
</section>

<style lang="scss">
  $panel-gap: 1rem;
  $panel-border: #dbe4ff;

  .dashboard {
    display: grid;
    gap: 1rem;
    padding: clamp(1rem, 4vw, 2rem);
    color: #0f172a;
    background: linear-gradient(180deg, #ffffff, #f8fafc);

    &[data-mounted='true'] { border: 1px solid rgba(37, 99, 235, 0.12); }
    &__header, &__card-head, &__meta { display: flex; align-items: center; justify-content: space-between; gap: $panel-gap; }

    &__grid {
      display: grid;
      gap: 1rem;
      grid-template-columns: repeat(auto-fit, minmax(18rem, 1fr));
      list-style: none;
      padding: 0;
      margin: 0;
    }

    &__card {
      display: grid;
      gap: 0.875rem;
      padding: 1rem;
      border: 1px solid $panel-border;
      border-radius: 18px;
      background: linear-gradient(180deg, rgba(255, 255, 255, 0.96), rgba(239, 246, 255, 0.92)), url('https://images.example.com/noise.webp');
      &.selected { box-shadow: 0 18px 38px rgba(37, 99, 235, 0.16); }
      &.warning { border-color: rgba(245, 158, 11, 0.28); }
    }

    &__button.active, a { color: #2563eb; }
    &__toast { position: fixed; inset: auto 1.5rem 1.5rem auto; padding: 0.75rem 1rem; border-radius: 999px; color: white; background: #0f172a; }
  }
</style>
