<script>
  import { onMount } from 'svelte';

  const CUSTOM_KEY = 'gc_custom_schools_v1';
  const STATE_KEY = 'gc_ui_state_v1';
  const CONFIG_KEY = 'gc_config_v1';
  const PROFILES_KEY = 'gc_profiles_v1';
  const DEFAULT_PROFILE = { name: 'Default', maxGrade: 20, passGrade: 10.45 };

  let baseSchools = $state({});
  let customSchools = $state({});
  let config = $state({ maxGrade: 20, passGrade: 10.45 });
  let profiles = $state([DEFAULT_PROFILE]);

  let selectedSchoolName = $state('');
  let selectedCourseName = $state('');
  let grades = $state(['', '', '', '', '', '']);

  let showAdvanced = $state(false);
  let showConfig = $state(false);
  let showCustom = $state(false);
  let showMeasurementTool = $state(false);
  let profileName = $state('');

  let newSchoolName = $state('');
  let newCourseSchool = $state('');
  let newCourseName = $state('');
  let newCourseWeights = $state([15, 15, 15, 15, 20, 20]);
  let measurementValues = $state([0, 0, 0, 0, 0, 0]);
  let hydrated = $state(false);

  let allSchoolNames = $derived([...Object.keys(baseSchools), ...Object.keys(customSchools)]);
  
  let availableCourses = $derived.by(() => {
    if (!selectedSchoolName) return [];
    const base = baseSchools[selectedSchoolName] || [];
    const custom = customSchools[selectedSchoolName] || [];
    return [...base, ...custom];
  });

  let currentCourse = $derived(availableCourses.find(c => c.name === selectedCourseName));

  let zeroGradeIndexes = $derived.by(() => grades.reduce((acc, g, i) => {
    const value = parseFloat(g);
    if (!g || Number.isNaN(value) || value <= 0) acc.push(i);
    return acc;
  }, []));

  const severitySaturation = {
    ok: 55,
    warn: 65,
    hard: 70,
    crit: 80,
    neutral: 50
  };

  let computation = $derived.by(() => {
    if (!currentCourse) return { average: 0, needed: 0, message: 'Selecciona un curso', severity: 'neutral', hue: 120, ratio: 0, sat: severitySaturation.neutral };

    const weights = currentCourse.weights;
    const totalWeight = weights.reduce((a, b) => a + b, 0);

    if (totalWeight <= 0) return { average: 0, needed: 0, message: 'Peso total es 0', severity: 'hard', hue: 0, ratio: 1, sat: severitySaturation.hard };

    let usedWeight = 0;
    let weightedSum = 0;

    weights.forEach((w, i) => {
      const rawGrade = parseFloat(grades[i]);
      let g = (!Number.isNaN(rawGrade) && rawGrade > 0) ? rawGrade : 0;
      if (!g) g = measurementValues[i] || 0;
      if (g > 0) {
        usedWeight += w;
        weightedSum += Math.min(g, config.maxGrade) * (w / 100);
      }
    });

    const missingWeight = Math.max(totalWeight - usedWeight, 0);
    const average = weightedSum;

    let needed = 0;
    let message = '';
    let severity = 'ok';

    if (missingWeight <= 0.01) {
      if (average >= config.passGrade - 0.001) {
        needed = 0;
        message = '¡Aprobado!';
        severity = 'ok';
      } else {
        needed = 0;
        message = 'Imposible aprobar';
        severity = 'crit';
      }
    } else {
      needed = (config.passGrade - average) * 100 / missingWeight;
      if (needed <= 0) {
        message = 'Ya aprobaste.';
        severity = 'ok';
      } else if (needed > config.maxGrade) {
        message = `Imposible llegar (necesitas ${needed.toFixed(2)})`;
        severity = 'crit';
      } else if (needed > config.maxGrade * 0.75) {
        message = `Necesitas ${needed.toFixed(2)} (muy alto)`;
        severity = 'hard';
      } else if (needed > config.maxGrade * 0.5) {
        message = `Necesitas ${needed.toFixed(2)} (alto)`;
        severity = 'warn';

      } else if (needed > config.maxGrade * 0.35) {
        message = `Necesitas ${needed.toFixed(2)} (moderado)`;
        severity = 'warn';
      } else if (needed > config.maxGrade * 0.2) {
        message = `Necesitas ${needed.toFixed(2)} (bajo)`;
        severity = 'ok';
      } else if (needed > config.maxGrade * 0.05) {
        message = `Necesitas ${needed.toFixed(2)} (muy bajo)`;
        severity = 'good';
      }
      else {
        message = `Necesitas ${needed.toFixed(2)}`;
        severity = 'ok';
      }
    }

    const ratio = Math.min(Math.max((needed > 0 ? needed : 0) / config.maxGrade, 0), 1);
    const hue = 120 - ratio * 120;
    const sat = severitySaturation[severity] ?? severitySaturation.ok;
    const panelLight = 24 - ratio * 12;
    const panelText = 92 - ratio * 20;
    return { average, needed, message, severity, hue, ratio, sat, panelLight, panelText };
  });

  onMount(async () => {
    try {
      const res = await fetch('data.json');
      if (res.ok) {
        const data = await res.json();
        const bs = {};
        data.schools.forEach(s => {
          bs[s.name] = s.courses;
        });
        baseSchools = bs;
        if (!selectedSchoolName && Object.keys(bs).length > 0) {
             selectedSchoolName = Object.keys(bs)[0];
        }
           if (!newCourseSchool && Object.keys(bs).length > 0) {
             newCourseSchool = Object.keys(bs)[0];
           }
      }
    } catch (e) {
      console.error(e);
    }

    const savedCustom = localStorage.getItem(CUSTOM_KEY);
    if (savedCustom) customSchools = JSON.parse(savedCustom);

    const savedConfig = localStorage.getItem(CONFIG_KEY);
    if (savedConfig) config = JSON.parse(savedConfig);

    const savedProfiles = localStorage.getItem(PROFILES_KEY);
    if (savedProfiles) profiles = JSON.parse(savedProfiles);
    if (!profiles.some(p => p.name === DEFAULT_PROFILE.name)) {
      profiles = [DEFAULT_PROFILE, ...profiles];
    }

    const savedState = localStorage.getItem(STATE_KEY);
    if (savedState) {
      const s = JSON.parse(savedState);
      if (s.school) selectedSchoolName = s.school;
      if (s.course) selectedCourseName = s.course;
      if (s.grades) grades = s.grades;
    }

    hydrated = true;
  });

  $effect(() => {
    if (!hydrated) return;
    const state = { school: selectedSchoolName, course: selectedCourseName, grades };
    localStorage.setItem(STATE_KEY, JSON.stringify(state));
  });

  $effect(() => {
    if (!hydrated) return;
    localStorage.setItem(CUSTOM_KEY, JSON.stringify(customSchools));
  });

  $effect(() => {
    if (!hydrated) return;
    localStorage.setItem(CONFIG_KEY, JSON.stringify(config));
  });

  $effect(() => {
    if (!hydrated) return;
    localStorage.setItem(PROFILES_KEY, JSON.stringify(profiles));
  });

  function addSchool() {
    if (!newSchoolName.trim()) return;
    if (baseSchools[newSchoolName] || customSchools[newSchoolName]) return;
    customSchools[newSchoolName] = [];
    selectedSchoolName = newSchoolName;
    newCourseSchool = newSchoolName;
    newSchoolName = '';
  }

  function addCourse() {
    if (!newCourseSchool || !newCourseName.trim()) return;
    if (!customSchools[newCourseSchool]) customSchools[newCourseSchool] = [];

    const newCourse = {
      name: newCourseName,
      weights: [...newCourseWeights]
    };

    const idx = customSchools[newCourseSchool].findIndex(c => c.name === newCourseName);
    if (idx >= 0) {
      customSchools[newCourseSchool][idx] = newCourse;
    } else {
      customSchools[newCourseSchool].push(newCourse);
    }
    
    selectedSchoolName = newCourseSchool;
    selectedCourseName = newCourseName;
    newCourseName = '';
  }
  
  function deleteCustomCourse(school, courseName) {
      if (customSchools[school]) {
          customSchools[school] = customSchools[school].filter(c => c.name !== courseName);
          if (customSchools[school].length === 0) {
              delete customSchools[school];
              // Force update if needed, but $state handles it
              customSchools = { ...customSchools };
              if (selectedSchoolName === school) selectedSchoolName = '';
          }
      }
  }
  
  function clearCustom() {
      customSchools = {};
      selectedSchoolName = '';
      newCourseSchool = '';
  }

    function clearGrades() {
      grades = ['', '', '', '', '', ''];
    }

  function saveProfile() {
      if (!profileName.trim()) return;
      const idx = profiles.findIndex(p => p.name === profileName);
      const p = { name: profileName, maxGrade: config.maxGrade, passGrade: config.passGrade };
      if (idx >= 0) profiles[idx] = p;
      else profiles.push(p);
      profileName = '';
  }
  
  function loadProfile(e) {
      const name = e.target.value;
      const p = profiles.find(p => p.name === name);
      if (p) {
          config.maxGrade = p.maxGrade;
          config.passGrade = p.passGrade;
      }
  }

  const weightKind = (i) => {
    const isContinua = (i % 2) === 0;
    const index = Math.floor(i / 2) + 1;
    return `${isContinua ? 'Continua' : 'Examen'} ${index}`;
  };
  
  function deleteProfile() {
      // Need a way to select profile to delete. Using the select value?
      // The select is for loading. Let's add a small UI for managing profiles if needed.
      // For now, just save/load is fine as per request.
  }

    function updateMeasurement(i, value) {
      measurementValues = measurementValues.map((v, idx) => idx === i ? Number(value) : v);
    }
</script>

<div class="page">
  <header class="hero">
    <div class="hero__content">
      <h1>Calculadora de notas</h1>
      <p class="lede">Selecciona escuela y curso, ingresa tus notas y ve al instante tu promedio y lo que necesitas para aprobar.</p>
    </div>
  </header>

  <main class="shell">

    <section class="panel">
      <div class="panel__header">
        <div>
          <p class="label">Selección</p>
          <h2>Escuela y curso</h2>
        </div>
        <div class="actions">
          <button class="ghost" on:click={() => { selectedSchoolName = ''; selectedCourseName = ''; grades = ['', '', '', '', '', '']; }}>Reiniciar</button>
        </div>
      </div>

      <div class="grid grid--2">
        <label class="field">
          <span>Escuela</span>
          <select bind:value={selectedSchoolName}>
            {#each allSchoolNames as name}
              <option value={name}>{name}</option>
            {/each}
          </select>
        </label>
        <label class="field">
          <span>Curso</span>
          <select bind:value={selectedCourseName}>
            {#each availableCourses as course}
              <option value={course.name}>{course.name}</option>
            {/each}
          </select>
        </label>
      </div>

      <div class="weights">
        {#if currentCourse}
          {#each currentCourse.weights as w, i}
            <div class="weight-pill">{weightKind(i)} • {w}%</div>
          {/each}
        {/if}
      </div>
      <p class="hint">
        {#if currentCourse}
          {#if Math.abs(100 - currentCourse.weights.reduce((a,b)=>a+b,0)) > 1}
            <span style="color: var(--warn)">Los pesos suman {currentCourse.weights.reduce((a,b)=>a+b,0)}% (no 100%).</span>
          {:else}
            Notas en 0 se omiten y se usan para calcular lo que necesitas.
          {/if}
        {:else}
          No hay curso seleccionado.
        {/if}
      </p>
    </section>

    <section class="panel">
      <div class="panel__header">
        <div>
          <h2>Notas</h2>
        </div>
        <button class="ghost" on:click={clearGrades}>Limpiar notas</button>
      </div>

      <div class="grade-list" role="list">
        {#each grades as g, i}
          <div class="grade-card" role="listitem">
            <div class="grade-card__label">
              <span class="pill">{weightKind(i)}</span>
            </div>
            <label class="field">
              <input type="number" bind:value={grades[i]} min="0" max={config.maxGrade} step="0.1" placeholder="0 = sin nota" />
            </label>
          </div>
        {/each}
      </div>
    </section>

    <section class="panel panel--accent" style={`--result-hue:${computation.hue}; --result-sat:${computation.sat}; --result-light:${computation.panelLight}%; --result-text:${computation.panelText}%;`}>
      <div class="results">
        <div>
          <p class="label">Promedio ponderado</p>
          <p class="metric">{computation.average.toFixed(2)}</p>
        </div>
        <div>
          <p class="label">Necesitas (mínimo)</p>
          <p class="metric">{computation.needed > 0 ? computation.needed.toFixed(2) : '–'}</p>
        </div>
        <div class="callout" data-severity={computation.severity} style={`--result-hue:${computation.hue}; --result-sat:${computation.sat};`}>
          <div class="callout__badge">{computation.severity === 'crit' ? 'Crítico' : computation.severity === 'hard' ? 'Alto' : computation.severity === 'warn' ? 'Atento' : 'OK'}</div>
          <div>{computation.message}</div>
        </div>
      </div>
    </section>

    <div class="measurement-anchor">
      <button class="primary" on:click={() => showMeasurementTool = !showMeasurementTool}>
        {showMeasurementTool ? 'Cerrar medición avanzada' : 'Medición avanzada'}
      </button>
      <button class="ghost" aria-expanded={showAdvanced} on:click={() => showAdvanced = !showAdvanced}>
        {showAdvanced ? 'Cerrar configuración avanzada' : 'Configuración avanzada'}
      </button>
    </div>

    {#if showMeasurementTool}
    <section class="panel measurement-panel">
      <div class="panel__header">
        <div>
          <h2>Medicion avanzada</h2>
        </div>
      </div>
      {#if zeroGradeIndexes.length === 0}
        <p class="hint">Ya no quedan notas por medir.</p>
      {:else}
        <div class="measurement-list">
          {#each zeroGradeIndexes as idx}
            <div class="measurement-card">
              <div class="radial-gauge" style={`--gauge:${(measurementValues[idx] / config.maxGrade) * 360}deg; --gauge-hue:${120 - (measurementValues[idx] / config.maxGrade) * 120};`}>
                <span>{measurementValues[idx].toFixed(0)}</span>
              </div>
              <div class="measurement-body">
                <p class="muted">Nota {idx + 1} • {weightKind(idx)}</p>
                <input type="range" min="0" max={config.maxGrade} step="1" value={measurementValues[idx]} on:input={(event) => updateMeasurement(idx, event.target.value)} />
              </div>
            </div>
          {/each}
        </div>
      {/if}
    </section>
    {/if}
        {#if showAdvanced}
    <section class="panel panel--muted">
      <div class="panel__header">
        <div>
          <p class="label">Avanzado</p>
          <h2>Configuración</h2>
        </div>
      </div>

      <div class="accordion">
        <button class="accordion__trigger" aria-expanded={showConfig} on:click={() => showConfig = !showConfig}>
          <span>Presets</span>
          <span class="chevron">{showConfig ? '−' : '+'}</span>
        </button>
        {#if showConfig}
        <div class="accordion__body">
          <div class="grid grid--3 compact">

            <label class="field">
              <span>Nota máxima</span>
              <input type="number" bind:value={config.maxGrade} min="1" step="0.1" />
            </label>
            <label class="field">
              <span>Mínimo para aprobar</span>
              <input type="number" bind:value={config.passGrade} min="1" step="0.01" />
            </label>
            <label class="field">
              <span>Perfil</span>
              <div class="profile-row">
                <select on:change={loadProfile}>
                  <option value="">Cargar perfil...</option>
                  {#each profiles as p}
                    <option value={p.name}>{p.name}</option>
                  {/each}
                </select>
              </div>
              <div class="profile-row">
                  <input type="text" bind:value={profileName} placeholder="Nombre nuevo" />
                  <button class="ghost small" on:click={saveProfile}>Guardar</button>
              </div>
            </label>
          </div>
          <p class="hint">Estos valores solo se guardan en este navegador.</p>
        </div>
        {/if}

        <button class="accordion__trigger" aria-expanded={showCustom} on:click={() => showCustom = !showCustom}>
          <span>Escuelas y cursos personalizados</span>
          <span class="chevron">{showCustom ? '−' : '+'}</span>
        </button>
        {#if showCustom}
        <div class="accordion__body">
          <div class="grid grid--2 stack">
            <div class="card card--border">
              <h3>Agregar escuela</h3>
              <label class="field">
                <span>Nombre</span>
                <input type="text" bind:value={newSchoolName} placeholder="Ej: Ciencia de Datos" />
              </label>
              <button class="primary" on:click={addSchool}>Agregar</button>
            </div>

            <div class="card card--border">
              <h3>Agregar curso</h3>
              <div class="grid grid--2 compact">
                <label class="field">
                  <span>Escuela</span>
                  <select bind:value={newCourseSchool}>
                     {#each allSchoolNames as name}
                       <option value={name}>{name}</option>
                     {/each}
                  </select>
                </label>
                <label class="field">
                  <span>Nombre del curso</span>
                  <input type="text" bind:value={newCourseName} placeholder="Ej: Algoritmos II" />
                </label>
              </div>
              <div class="weights-editor">
                 {#each newCourseWeights as w, i}
                   <input type="number" bind:value={newCourseWeights[i]} placeholder={`W${i+1}`} />
                 {/each}
              </div>
              <p class="hint">Los pesos deben sumar 100.</p>
              <button class="primary" on:click={addCourse}>Agregar</button>
            </div>
          </div>

          <div class="card card--border">
            <div class="panel__header">
              <div>
                <p class="label">Personalizado</p>
                <h3>Tus cursos guardados</h3>
              </div>
              <button class="ghost" on:click={clearCustom}>Vaciar todo</button>
            </div>
            {#if Object.keys(customSchools).length === 0}
              <div class="list-empty">Aún no tienes cursos personalizados.</div>
            {:else}
              {#each Object.entries(customSchools) as [school, courses]}
                {#each courses as course}
                  <div class="custom-item">
                    <div class="custom-meta">
                      <div class="name">{course.name}</div>
                      <div class="school">{school} • pesos {course.weights.reduce((a,b)=>a+b,0)}%</div>
                    </div>
                    <button class="ghost" on:click={() => deleteCustomCourse(school, course.name)}>Borrar</button>
                  </div>
                {/each}
              {/each}
            {/if}
          </div>
        </div>
        {/if}
      </div>
    </section>
    {/if}

  </main>
</div>


<style>
  .config__title { display: flex; align-items: center; gap: 12px; }
  .config__body { margin-top: 12px; }
  .profile-row { display: flex; gap: 6px; align-items: center; margin-bottom: 6px; }
  .weights-editor { display: grid; grid-template-columns: repeat(auto-fit, minmax(60px, 1fr)); gap: 8px; margin: 10px 0; }
  .weights-editor input { text-align: right; }
  .custom-item {
    display: flex; justify-content: space-between; align-items: center;
    padding: 10px 12px;
    border: 1px solid var(--border);
    border-radius: 12px;
    margin-top: 8px;
    background: rgba(255, 255, 255, 0.02);
  }
  .custom-meta { display: flex; flex-direction: column; gap: 4px; }
  .custom-meta .name { font-weight: 700; }
  .custom-meta .school { color: var(--muted); font-size: 13px; }
  .list-empty { color: var(--muted); font-style: italic; }
  .stack { gap: 14px; }
</style>
