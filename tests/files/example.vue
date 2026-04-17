<template>
  <!-- Vue demo docs: https://example.com/vue-guide -->
  <section class="dashboard" :data-theme="theme" :style="{ '--accent': panelAccent }">
    <header class="dashboard__header">
      <div>
        <p class="dashboard__eyebrow">{{ eyebrow }}</p>
        <h1>{{ title }}</h1>
        <p class="dashboard__subtitle">Track release notes at <a :href="profileUrl" target="_blank" rel="noreferrer">{{ profileUrl }}</a></p>
      </div>
      <div class="dashboard__actions">
        <button class="primary" @click="toggleTheme">Theme: {{ theme }}</button>
        <button class="ghost" @click.stop="refresh">Refresh</button>
      </div>
    </header>
    <label class="dashboard__search">
      <span>Filter items</span>
      <input v-model.trim="query" type="search" placeholder="Search cards" @keyup.enter="refresh" />
    </label>

    <Transition name="fade-slide" mode="out-in">
      <p v-if="loading" key="loading" class="dashboard__status">Loading {{ loadingLabel }}...</p>
      <p v-else-if="filteredItems.length === 0" key="empty" class="dashboard__status">No items for "{{ query }}"</p>
      <p v-else key="ready" class="dashboard__status">Showing {{ filteredItems.length }} of {{ items.length }} items</p>
    </Transition>

    <ul class="dashboard__grid">
      <li v-for="item in filteredItems" :key="item.id" :class="['dashboard__card', { 'is-active': item.active }]" :data-tone="item.tone">
        <component :is="item.tag" class="dashboard__tag">{{ item.tag.toUpperCase() }}</component>
        <div class="dashboard__card-head">
          <div>
            <h2>{{ item.label }}</h2>
            <p>{{ item.description }}</p>
          </div>
          <button class="ghost" @click="select(item.id)">Select</button>
        </div>

        <dl class="dashboard__meta">
          <div><dt>Status</dt><dd>{{ item.active ? 'Active' : 'Queued' }}</dd></div>
          <div><dt>Latency</dt><dd>{{ item.latency }}ms</dd></div>
        </dl>

        <footer class="dashboard__card-footer">
          <a :href="item.url" target="_blank" rel="noreferrer" @click.prevent="track(item.url)">Open docs</a>
          <slot name="actions" :item="item"><button class="primary" @click="favorite(item.id)">Favorite</button></slot>
        </footer>
      </li>
    </ul>

    <template v-if="highlightedItem">
      <aside class="dashboard__drawer">
        <h3>Focused item</h3>
        <p>{{ highlightedItem.label }}</p>
        <small>{{ highlightedItem.url }}</small>
      </aside>
    </template>

    <Teleport to="body">
      <Transition name="fade-slide"><div v-if="toast" class="dashboard__toast" role="status">{{ toast }}</div></Transition>
    </Teleport>
  </section>
</template>

<script setup lang="ts">
import { computed, onMounted, ref, watch } from 'vue';

type Tone = 'primary' | 'success' | 'warning';
type Item = { id: number; label: string; description: string; url: string; latency: number; tone: Tone; tag: 'span' | 'strong'; active: boolean; };

const eyebrow = ref('SweetLine Vue demo');
const title = ref('Operations overview');
const theme = ref<'day' | 'night'>('day');
const query = ref('');
const loading = ref(false);
const loadingLabel = ref('metrics');
const toast = ref('');
const profileUrl = ref('https://example.com/profile');
const panelAccent = computed(() => (theme.value === 'night' ? '#38bdf8' : '#2563eb'));
const items = ref<Item[]>([
  { id: 1, label: 'Deploy queue', description: 'Pending releases across regions.', url: 'https://example.com/deploys', latency: 128, tone: 'primary', tag: 'span', active: true },
  { id: 2, label: 'Search cluster', description: 'Replica lag and cache health.', url: 'https://example.com/search', latency: 264, tone: 'warning', tag: 'strong', active: false },
  { id: 3, label: 'Billing sync', description: 'Webhook throughput and retries.', url: 'https://example.com/billing', latency: 94, tone: 'success', tag: 'span', active: true }
]);

const filteredItems = computed(() => items.value.filter((item) => item.label.toLowerCase().includes(query.value.toLowerCase())));
const highlightedItem = computed(() => filteredItems.value.find((item) => item.active) ?? null);

function toggleTheme() { theme.value = theme.value === 'night' ? 'day' : 'night'; }

function refresh() {
  loading.value = true;
  loadingLabel.value = query.value || 'dashboard';
  window.setTimeout(() => {
    loading.value = false;
    toast.value = `Updated from ${profileUrl.value}`;
  }, 120);
}

function track(url: string) { console.log(`track ${url}`); toast.value = `Tracked ${url}`; }
function select(id: number) { toast.value = `Selected card ${id}`; }
function favorite(id: number) { toast.value = `Favorited card ${id}`; }

onMounted(() => { console.log('mounted vue demo', profileUrl.value); });
watch(query, (value) => { if (value.length > 10) toast.value = `Long query: ${value}`; });
</script>

<style scoped lang="scss">
$panel-gap: 1rem;
$panel-border: #dbe4ff;
$panel-shadow: rgba(15, 23, 42, 0.12);

.dashboard {
  display: grid;
  gap: 1.25rem;
  padding: clamp(1rem, 4vw, 2rem);
  color: #0f172a;
  background: linear-gradient(180deg, #ffffff, #f8fafc);

  &[data-theme='night'] { color: #e2e8f0; background: radial-gradient(circle at top, #1e293b 0%, #0f172a 68%); }
  &__header, &__actions, &__card-head, &__card-footer { display: flex; align-items: center; justify-content: space-between; gap: $panel-gap; }

  &__grid {
    display: grid;
    gap: $panel-gap;
    grid-template-columns: repeat(auto-fit, minmax(18rem, 1fr));
    list-style: none;
    padding: 0;
    margin: 0;
  }

  &__card {
    position: relative;
    display: grid;
    gap: 0.875rem;
    padding: 1.125rem;
    border: 1px solid $panel-border;
    border-radius: 18px;
    box-shadow: 0 18px 40px $panel-shadow;
    background: linear-gradient(180deg, rgba(255, 255, 255, 0.96), rgba(248, 250, 252, 0.92)), url('https://images.example.com/noise.webp');
    &.is-active::before { content: 'Source https://example.com/vue-badge'; color: white; background: linear-gradient(90deg, var(--accent), #1d4ed8); }
  }

  &__tag { inline-size: fit-content; padding: 0.3rem 0.75rem; border-radius: 999px; color: var(--accent); background: rgba(37, 99, 235, 0.12); }
  &__search input:focus-visible, button:focus-visible, a:focus-visible { outline: none; box-shadow: 0 0 0 3px rgba(37, 99, 235, 0.16); }
  &__toast { position: fixed; inset: auto 1.5rem 1.5rem auto; padding: 0.75rem 1rem; border-radius: 999px; color: white; background: #0f172a; }
  :deep(code) { font-family: 'JetBrains Mono', monospace; }
}

.fade-slide-enter-active, .fade-slide-leave-active { transition: all 180ms ease; }
.fade-slide-enter-from, .fade-slide-leave-to { opacity: 0; transform: translateY(6px); }
</style>
