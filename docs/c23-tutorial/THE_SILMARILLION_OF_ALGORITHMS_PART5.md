# The Silmarillion of Algorithms
## Part V: The Palantír Protocol — Communication Across the Depths

*"The Elves had their Palantíri, the Seeing Stones of old.
But the Dwarves, ever practical, sought not to see but to speak
across the vast distances of their underground realms.
And so they harnessed the power of falling water
to send messages through copper wire, encoded in pulses
of lightning — the Palantír Protocol."*

---

# Part V-A: The Waters of Power — Hydroelectric Generation

*In which we learn how the Dwarves harnessed the underground
rivers and waterfalls to generate the electrical power
needed to operate their telegraph network.*

## The Philosophy of Stored Energy

Deep beneath the Misty Mountains, the Dwarves discovered vast
underground lakes and rivers, fed by the melting snows above.
They realized that falling water could turn great wheels,
and turning wheels could generate the spark of lightning.

This is **Hydroelectric Power** — converting the potential energy
of elevated water into electrical current.

```
The Dwarven Hydroelectric Station:

          ~~~~~ Upper Lake (Reservoir) ~~~~~
                        │
                        │ Penstock (Water pipe)
                        │ Height = h meters
                        ▼
              ┌─────────────────────┐
              │     Turbine         │ ← Water spins the wheel
              │    ╭───────╮        │
              │    │ ≋≋≋≋≋ │←──────────── Flowing water
              │    │ ╱ ◯ ╲ │        │
              │    ╰───────╯        │
              │        │            │
              │    ┌───┴───┐        │
              │    │Generator│       │ ← Spinning creates electricity
              │    └───────┘        │
              └─────────────────────┘
                        │
                        ▼
              ═══ To Telegraph Lines ═══
```

### The Physics of Power

The power generated depends on three factors:
1. **Head (h)**: Height difference between reservoir and turbine
2. **Flow Rate (Q)**: Volume of water per second (m³/s)
3. **Efficiency (η)**: How well we convert water energy to electricity

```
Power = η × ρ × g × h × Q

Where:
  η = efficiency (typically 0.85-0.95)
  ρ = water density (1000 kg/m³)
  g = gravity (9.81 m/s²)
  h = head height (meters)
  Q = flow rate (m³/s)
```

```c
/*
 * Hydroelectric Power Simulation
 *
 * Models a Dwarven underground power station
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

/* Physical constants */
#define WATER_DENSITY   1000.0   /* kg/m³ */
#define GRAVITY         9.81     /* m/s² */
#define TURBINE_EFF     0.90     /* 90% efficiency */
#define GENERATOR_EFF   0.95     /* 95% efficiency */
#define OVERALL_EFF     (TURBINE_EFF * GENERATOR_EFF)

/*
 * Power Station State
 */
typedef struct {
    char name[50];

    /* Physical parameters */
    double head_height;      /* meters */
    double max_flow_rate;    /* m³/s */
    double current_flow;     /* m³/s */

    /* Reservoir */
    double reservoir_volume; /* m³ (current) */
    double reservoir_max;    /* m³ (capacity) */
    double inflow_rate;      /* m³/s (from underground rivers) */

    /* Output */
    double power_output;     /* Watts */
    double voltage;          /* Volts */
    double current;          /* Amperes */

    /* Statistics */
    double total_energy;     /* Joules generated */
    double uptime_hours;
} PowerStation;

/*
 * Initialize a power station
 */
PowerStation *power_station_create(const char *name,
                                    double head_height,
                                    double max_flow,
                                    double reservoir_capacity,
                                    double initial_fill,
                                    double inflow) {
    PowerStation *ps = malloc(sizeof(PowerStation));
    if (!ps) return NULL;

    strncpy(ps->name, name, sizeof(ps->name) - 1);
    ps->head_height = head_height;
    ps->max_flow_rate = max_flow;
    ps->current_flow = 0;
    ps->reservoir_max = reservoir_capacity;
    ps->reservoir_volume = initial_fill;
    ps->inflow_rate = inflow;
    ps->power_output = 0;
    ps->voltage = 0;
    ps->current = 0;
    ps->total_energy = 0;
    ps->uptime_hours = 0;

    return ps;
}

/*
 * Calculate theoretical power output
 */
double calculate_power(double head, double flow, double efficiency) {
    return efficiency * WATER_DENSITY * GRAVITY * head * flow;
}

/*
 * Update power station state for one time step
 */
void power_station_update(PowerStation *ps, double dt_seconds,
                          double demand_watts) {
    /* Update reservoir from inflow */
    ps->reservoir_volume += ps->inflow_rate * dt_seconds;
    if (ps->reservoir_volume > ps->reservoir_max) {
        ps->reservoir_volume = ps->reservoir_max;  /* Overflow */
    }

    /* Calculate flow needed to meet demand */
    double required_flow = demand_watts /
        (OVERALL_EFF * WATER_DENSITY * GRAVITY * ps->head_height);

    /* Limit to max flow and available water */
    ps->current_flow = required_flow;
    if (ps->current_flow > ps->max_flow_rate) {
        ps->current_flow = ps->max_flow_rate;
    }

    /* Check if enough water in reservoir */
    double water_needed = ps->current_flow * dt_seconds;
    if (water_needed > ps->reservoir_volume) {
        /* Not enough water — reduce flow */
        ps->current_flow = ps->reservoir_volume / dt_seconds;
        water_needed = ps->reservoir_volume;
    }

    /* Drain reservoir */
    ps->reservoir_volume -= water_needed;

    /* Calculate actual power output */
    ps->power_output = calculate_power(ps->head_height,
                                        ps->current_flow,
                                        OVERALL_EFF);

    /* Calculate voltage and current (assuming 48V DC system) */
    ps->voltage = 48.0;
    ps->current = ps->power_output / ps->voltage;

    /* Update statistics */
    ps->total_energy += ps->power_output * dt_seconds;
    ps->uptime_hours += dt_seconds / 3600.0;
}

/*
 * Get reservoir fill percentage
 */
double power_station_fill_percent(PowerStation *ps) {
    return (ps->reservoir_volume / ps->reservoir_max) * 100.0;
}

/*
 * Print power station status
 */
void power_station_print_status(PowerStation *ps) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║  POWER STATION: %-36s ║\n", ps->name);
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Head Height:     %8.1f m                         ║\n",
           ps->head_height);
    printf("║  Flow Rate:       %8.3f m³/s (max: %.3f)        ║\n",
           ps->current_flow, ps->max_flow_rate);
    printf("║  Reservoir:       %8.1f m³ (%.1f%% full)          ║\n",
           ps->reservoir_volume, power_station_fill_percent(ps));
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Power Output:    %8.1f W (%.2f kW)              ║\n",
           ps->power_output, ps->power_output / 1000.0);
    printf("║  Voltage:         %8.1f V                         ║\n",
           ps->voltage);
    printf("║  Current:         %8.2f A                         ║\n",
           ps->current);
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Total Energy:    %8.1f kWh                       ║\n",
           ps->total_energy / 3600000.0);
    printf("║  Uptime:          %8.2f hours                     ║\n",
           ps->uptime_hours);
    printf("╚══════════════════════════════════════════════════════╝\n");
}
```

### The Power Grid — Connecting Multiple Stations

The Dwarves built multiple power stations along their underground
rivers to ensure reliability. When one station's reservoir ran low,
others could compensate.

```c
/*
 * Power Grid — Multiple stations working together
 */

#define MAX_STATIONS 10

typedef struct {
    PowerStation *stations[MAX_STATIONS];
    int station_count;

    double total_demand;      /* Current demand (Watts) */
    double total_supply;      /* Current supply (Watts) */
    double grid_frequency;    /* Simulated grid frequency */
} PowerGrid;

PowerGrid *power_grid_create(void) {
    PowerGrid *grid = calloc(1, sizeof(PowerGrid));
    grid->grid_frequency = 50.0;  /* 50 Hz standard */
    return grid;
}

void power_grid_add_station(PowerGrid *grid, PowerStation *ps) {
    if (grid->station_count < MAX_STATIONS) {
        grid->stations[grid->station_count++] = ps;
    }
}

/*
 * Distribute demand across stations based on capacity
 */
void power_grid_update(PowerGrid *grid, double dt_seconds,
                        double total_demand) {
    grid->total_demand = total_demand;

    /* Calculate total available capacity */
    double total_capacity = 0;
    for (int i = 0; i < grid->station_count; i++) {
        PowerStation *ps = grid->stations[i];
        double max_power = calculate_power(ps->head_height,
                                            ps->max_flow_rate,
                                            OVERALL_EFF);
        total_capacity += max_power;
    }

    /* Distribute demand proportionally */
    grid->total_supply = 0;
    for (int i = 0; i < grid->station_count; i++) {
        PowerStation *ps = grid->stations[i];
        double max_power = calculate_power(ps->head_height,
                                            ps->max_flow_rate,
                                            OVERALL_EFF);
        double share = (max_power / total_capacity) * total_demand;

        power_station_update(ps, dt_seconds, share);
        grid->total_supply += ps->power_output;
    }

    /* Adjust frequency based on supply/demand balance */
    double balance = grid->total_supply / grid->total_demand;
    grid->grid_frequency = 50.0 * balance;

    /* Clamp frequency to reasonable range */
    if (grid->grid_frequency < 45.0) grid->grid_frequency = 45.0;
    if (grid->grid_frequency > 55.0) grid->grid_frequency = 55.0;
}

void power_grid_print_status(PowerGrid *grid) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("              DWARVEN POWER GRID STATUS                    \n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  Total Demand:  %.2f kW\n", grid->total_demand / 1000.0);
    printf("  Total Supply:  %.2f kW\n", grid->total_supply / 1000.0);
    printf("  Grid Frequency: %.2f Hz\n", grid->grid_frequency);
    printf("  Status: %s\n",
           grid->total_supply >= grid->total_demand ? "STABLE" : "BROWNOUT");
    printf("═══════════════════════════════════════════════════════════\n");

    for (int i = 0; i < grid->station_count; i++) {
        power_station_print_status(grid->stations[i]);
    }
}
```

### Energy Storage — The Pumped Storage System

For times of low demand, the Dwarves pumped water back up to
the reservoir, storing energy for peak demand periods.

```c
/*
 * Pumped Storage System
 *
 * During low demand: pump water UP to store energy
 * During high demand: release water DOWN to generate power
 */

typedef enum {
    MODE_GENERATING,    /* Water flowing down, generating power */
    MODE_PUMPING,       /* Using power to pump water up */
    MODE_IDLE           /* Neither generating nor pumping */
} StorageMode;

typedef struct {
    PowerStation base;    /* Inherits from PowerStation */

    StorageMode mode;
    double pump_efficiency;   /* Typically 0.75-0.85 */
    double pump_power;        /* Power consumed when pumping (Watts) */
    double pump_rate;         /* Water pumped (m³/s) at full power */
} PumpedStorage;

void pumped_storage_set_mode(PumpedStorage *ps, StorageMode mode,
                              double available_power) {
    ps->mode = mode;

    switch (mode) {
        case MODE_GENERATING:
            /* Normal power generation */
            ps->base.current_flow = ps->base.max_flow_rate;
            break;

        case MODE_PUMPING:
            /* Pump water up using available power */
            ps->pump_power = available_power;
            ps->pump_rate = (available_power * ps->pump_efficiency) /
                            (WATER_DENSITY * GRAVITY * ps->base.head_height);
            break;

        case MODE_IDLE:
            ps->base.current_flow = 0;
            ps->pump_power = 0;
            ps->pump_rate = 0;
            break;
    }
}

void pumped_storage_update(PumpedStorage *ps, double dt_seconds) {
    switch (ps->mode) {
        case MODE_GENERATING:
            /* Drain reservoir, generate power */
            power_station_update(&ps->base, dt_seconds, 1e9);  /* Max demand */
            break;

        case MODE_PUMPING:
            /* Fill reservoir using pump */
            ps->base.reservoir_volume += ps->pump_rate * dt_seconds;
            if (ps->base.reservoir_volume > ps->base.reservoir_max) {
                ps->base.reservoir_volume = ps->base.reservoir_max;
            }
            ps->base.power_output = -ps->pump_power;  /* Negative = consuming */
            break;

        case MODE_IDLE:
            ps->base.power_output = 0;
            break;
    }
}
```

---

## The Telegraph Power Requirements

The Dwarven telegraph system required specific power characteristics:

```c
/*
 * Telegraph Power Requirements
 *
 * The telegraph operates on 48V DC with varying current based on
 * how many keys are being pressed simultaneously across the network.
 */

#define TELEGRAPH_VOLTAGE       48.0    /* Volts */
#define KEY_DOWN_CURRENT        0.1     /* Amperes per active key */
#define RELAY_CURRENT           0.05    /* Amperes per relay */
#define LINE_LOSS_PER_KM        0.01    /* Amperes lost per km */

typedef struct {
    int active_keys;          /* Number of keys currently pressed */
    int active_relays;        /* Number of relays in the circuit */
    double total_line_length; /* km of wire in use */

    double power_required;    /* Watts needed */
} TelegraphLoad;

double calculate_telegraph_load(TelegraphLoad *load) {
    double total_current =
        (load->active_keys * KEY_DOWN_CURRENT) +
        (load->active_relays * RELAY_CURRENT) +
        (load->total_line_length * LINE_LOSS_PER_KM);

    load->power_required = TELEGRAPH_VOLTAGE * total_current;
    return load->power_required;
}

/*
 * Simulation: Khazad-dûm telegraph network
 *
 * 7 stations, each with a telegraph key
 * Average of 2 keys active at any time (one sending, one receiving)
 * 50 relays in the network
 * 100 km total wire length
 */
void simulate_telegraph_power(void) {
    /* Create power grid */
    PowerGrid *grid = power_grid_create();

    /* Add power stations */
    PowerStation *main_station = power_station_create(
        "Khazad-dûm Main Falls",
        100.0,    /* 100m head */
        0.5,      /* 0.5 m³/s max flow */
        10000.0,  /* 10,000 m³ reservoir */
        8000.0,   /* 80% full */
        0.3       /* 0.3 m³/s inflow */
    );

    PowerStation *backup = power_station_create(
        "Moria Deep Spring",
        50.0,     /* 50m head */
        0.2,      /* 0.2 m³/s max flow */
        5000.0,   /* 5,000 m³ reservoir */
        5000.0,   /* 100% full */
        0.15      /* 0.15 m³/s inflow */
    );

    power_grid_add_station(grid, main_station);
    power_grid_add_station(grid, backup);

    /* Calculate telegraph load */
    TelegraphLoad telegraph = {
        .active_keys = 2,
        .active_relays = 50,
        .total_line_length = 100.0
    };

    double load = calculate_telegraph_load(&telegraph);
    printf("Telegraph network requires: %.2f Watts\n", load);

    /* Simulate 1 hour of operation */
    double dt = 60.0;  /* 1 minute steps */
    for (int t = 0; t < 60; t++) {
        /* Vary load slightly to simulate traffic */
        telegraph.active_keys = 1 + (rand() % 5);
        load = calculate_telegraph_load(&telegraph);

        power_grid_update(grid, dt, load);
    }

    power_grid_print_status(grid);
}
```

---

## Scavenger Hunt — Clue #23

*"The concept of potential energy converting to kinetic energy,
and then to electrical energy, mirrors the transformation of data.
In Sedgewick's introduction, he speaks of 'levels of abstraction'
in algorithm design. Just as water flows through turbines at different
levels, algorithms operate at different levels of abstraction.
What are these levels, and why do they matter?"*

🔍 **Your Quest**: Review Sedgewick's Algorithms in C, Chapter 1.
Find where he discusses abstraction levels. How does a high-level
algorithm differ from its implementation?

---

## Exercises — The Trials of Power

### Trial 1: The Load Balancer
```c
/*
 * Implement a load balancer that distributes telegraph demand
 * across multiple power stations based on:
 * 1. Current reservoir levels
 * 2. Efficiency ratings
 * 3. Distance to load center (line losses)
 */
```

### Trial 2: The Surge Protector
```c
/*
 * Implement protection against power surges:
 * 1. Detect when supply exceeds demand by >10%
 * 2. Automatically engage pumped storage
 * 3. Protect telegraph equipment from overvoltage
 */
```

---

*Thus ends the first chapter of the Palantír Protocol.
We have harnessed the waters of power.
In the next chapter, we shall learn the secret language
of dots and dashes — the Dwarven Morse Code.*

---

# Part V-B: Iglishmêk Pulses — The Dwarven Morse Code

*In which we learn the ancient Dwarven system of encoding
messages as short and long pulses, a secret language
known only to the telegraph operators of Khazad-dûm.*

## The Philosophy of Binary Encoding

The Dwarves realized that any message could be reduced to
a sequence of just two states: the key pressed (pulse) or released
(silence). By varying the duration of pulses, they created an
entire alphabet.

**Iglishmêk** (Dwarvish: "secret speaking") was their name for
this system of short pulses (dots) and long pulses (dashes).

```
The Two States:

  ─────      HIGH (key pressed, current flowing)
       │
       │
  ─────┘     LOW (key released, no current)

  Duration determines meaning:

  Short pulse (dit):  ─   (1 unit)
  Long pulse (dah):   ───  (3 units)
  Gap between parts:      (1 unit)
  Gap between letters:    (3 units)
  Gap between words:      (7 units)
```

## The International Morse Code

Before we create the Dwarven variant, let us learn the standard
Morse Code upon which it is based:

```
╔═══════════════════════════════════════════════════════════════════════╗
║                     INTERNATIONAL MORSE CODE                          ║
╠═══════════════════════════════════════════════════════════════════════╣
║  A  ● ━      N  ━ ●       1  ● ━ ━ ━ ━    Punctuation:               ║
║  B  ━ ● ● ●  O  ━ ━ ━     2  ● ● ━ ━ ━    .  ● ━ ● ━ ● ━             ║
║  C  ━ ● ━ ●  P  ● ━ ━ ●   3  ● ● ● ━ ━    ,  ━ ━ ● ● ━ ━             ║
║  D  ━ ● ●    Q  ━ ━ ● ━   4  ● ● ● ● ━    ?  ● ● ━ ━ ● ●             ║
║  E  ●        R  ● ━ ●     5  ● ● ● ● ●    '  ● ━ ━ ━ ━ ●             ║
║  F  ● ● ━ ●  S  ● ● ●     6  ━ ● ● ● ●    !  ━ ● ━ ● ━ ━             ║
║  G  ━ ━ ●    T  ━         7  ━ ━ ● ● ●    /  ━ ● ● ━ ●               ║
║  H  ● ● ● ●  U  ● ● ━     8  ━ ━ ━ ● ●    (  ━ ● ━ ━ ●               ║
║  I  ● ●      V  ● ● ● ━   9  ━ ━ ━ ━ ●    )  ━ ● ━ ━ ● ━             ║
║  J  ● ━ ━ ━  W  ● ━ ━     0  ━ ━ ━ ━ ━    &  ● ━ ● ● ●               ║
║  K  ━ ● ━    X  ━ ● ● ━                   :  ━ ━ ━ ● ● ●             ║
║  L  ● ━ ● ●  Y  ━ ● ━ ━                   ;  ━ ● ━ ● ━ ●             ║
║  M  ━ ━      Z  ━ ━ ● ●                   =  ━ ● ● ● ━               ║
╠═══════════════════════════════════════════════════════════════════════╣
║  ● = dit (short, 1 unit)    ━ = dah (long, 3 units)                   ║
║  Gap between symbols: 1 unit                                          ║
║  Gap between letters: 3 units                                         ║
║  Gap between words: 7 units                                           ║
╚═══════════════════════════════════════════════════════════════════════╝
```

### Implementing Morse Code in C

```c
/*
 * Morse Code Encoding and Decoding
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * Morse code lookup table
 * Using '.' for dit and '-' for dah
 */
typedef struct {
    char character;
    const char *morse;
} MorseEntry;

static const MorseEntry MORSE_TABLE[] = {
    /* Letters */
    {'A', ".-"},    {'B', "-..."},  {'C', "-.-."},  {'D', "-.."},
    {'E', "."},     {'F', "..-."},  {'G', "--."},   {'H', "...."},
    {'I', ".."},    {'J', ".---"},  {'K', "-.-"},   {'L', ".-.."},
    {'M', "--"},    {'N', "-."},    {'O', "---"},   {'P', ".--."},
    {'Q', "--.-"},  {'R', ".-."},   {'S', "..."},   {'T', "-"},
    {'U', "..-"},   {'V', "...-"},  {'W', ".--"},   {'X', "-..-"},
    {'Y', "-.--"},  {'Z', "--.."},

    /* Numbers */
    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"},
    {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."},
    {'8', "---.."}, {'9', "----."},

    /* Punctuation */
    {'.', ".-.-.-"}, {',', "--..--"}, {'?', "..--.."}, {'\'', ".----."},
    {'!', "-.-.--"}, {'/', "-..-."}, {'(', "-.--."}, {')', "-.--.-"},
    {'&', ".-..."}, {':', "---..."}, {';', "-.-.-."}, {'=', "-...-"},
    {'+', ".-.-."}, {'-', "-....-"}, {'_', "..--.-"}, {'"', ".-..-."},
    {'$', "...-..-"}, {'@', ".--.-."},

    {0, NULL}  /* Sentinel */
};

/*
 * Convert character to Morse code
 */
const char *char_to_morse(char c) {
    c = toupper(c);

    for (int i = 0; MORSE_TABLE[i].character != 0; i++) {
        if (MORSE_TABLE[i].character == c) {
            return MORSE_TABLE[i].morse;
        }
    }

    return NULL;  /* Character not found */
}

/*
 * Convert Morse code to character
 */
char morse_to_char(const char *morse) {
    for (int i = 0; MORSE_TABLE[i].character != 0; i++) {
        if (strcmp(MORSE_TABLE[i].morse, morse) == 0) {
            return MORSE_TABLE[i].character;
        }
    }

    return '\0';  /* Morse code not found */
}

/*
 * Encode text to Morse code
 * Output format: letters separated by ' ', words separated by ' / '
 */
char *text_to_morse(const char *text) {
    /* Calculate required length */
    size_t len = 0;
    for (const char *p = text; *p; p++) {
        if (*p == ' ') {
            len += 3;  /* " / " */
        } else {
            const char *m = char_to_morse(*p);
            if (m) {
                len += strlen(m) + 1;  /* morse + space */
            }
        }
    }

    char *result = malloc(len + 1);
    if (!result) return NULL;

    char *out = result;
    bool first = true;

    for (const char *p = text; *p; p++) {
        if (*p == ' ') {
            strcpy(out, "/ ");
            out += 2;
            first = true;
        } else {
            const char *m = char_to_morse(*p);
            if (m) {
                if (!first) {
                    *out++ = ' ';
                }
                strcpy(out, m);
                out += strlen(m);
                first = false;
            }
        }
    }

    *out = '\0';
    return result;
}

/*
 * Decode Morse code to text
 * Input format: letters separated by ' ', words separated by ' / '
 */
char *morse_to_text(const char *morse) {
    /* Make a copy we can modify */
    char *copy = strdup(morse);
    if (!copy) return NULL;

    /* Count words to estimate result size */
    size_t max_len = strlen(morse);
    char *result = malloc(max_len + 1);
    if (!result) {
        free(copy);
        return NULL;
    }

    char *out = result;
    char *token = strtok(copy, " ");

    while (token) {
        if (strcmp(token, "/") == 0) {
            *out++ = ' ';
        } else {
            char c = morse_to_char(token);
            if (c) {
                *out++ = c;
            }
        }
        token = strtok(NULL, " ");
    }

    *out = '\0';
    free(copy);
    return result;
}

/*
 * Print Morse code with visual timing
 */
void print_morse_visual(const char *morse) {
    printf("\n");
    printf("Audio visualization:\n");
    printf("────────────────────\n");

    for (const char *p = morse; *p; p++) {
        switch (*p) {
            case '.':
                printf("▄");      /* Short beep */
                break;
            case '-':
                printf("▄▄▄");    /* Long beep */
                break;
            case ' ':
                printf(" ");      /* Gap */
                break;
            case '/':
                printf("   ");    /* Word gap */
                break;
        }
    }
    printf("\n\n");
}

/*
 * Example usage
 */
void morse_demo(void) {
    const char *message = "KHAZAD DUM";

    printf("Original: %s\n", message);

    char *morse = text_to_morse(message);
    printf("Morse:    %s\n", morse);

    print_morse_visual(morse);

    char *decoded = morse_to_text(morse);
    printf("Decoded:  %s\n", decoded);

    free(morse);
    free(decoded);
}
```

### The Morse Tree — A Binary Decoding Structure

The Dwarven scholars discovered that Morse code could be represented
as a binary tree, where left branches represent dots and right
branches represent dashes:

```
                          [ROOT]
                         /      \
                       .          -
                      /            \
                    E                T
                   / \              / \
                  .   -            .   -
                 /     \          /     \
                I       A        N       M
               / \     / \      / \     / \
              .   -   .   -    .   -   .   -
             /     \ /     \  /     \ /     \
            S      U R     W D      K G      O
```

```c
/*
 * Morse Tree for efficient decoding
 */

typedef struct MorseNode {
    char character;           /* '\0' if not a valid endpoint */
    struct MorseNode *dot;    /* Left child (.) */
    struct MorseNode *dash;   /* Right child (-) */
} MorseNode;

/*
 * Create a new node
 */
MorseNode *morse_node_create(char c) {
    MorseNode *node = malloc(sizeof(MorseNode));
    if (node) {
        node->character = c;
        node->dot = NULL;
        node->dash = NULL;
    }
    return node;
}

/*
 * Insert a character into the tree
 */
void morse_tree_insert(MorseNode *root, char character, const char *code) {
    MorseNode *current = root;

    for (const char *p = code; *p; p++) {
        if (*p == '.') {
            if (!current->dot) {
                current->dot = morse_node_create('\0');
            }
            current = current->dot;
        } else if (*p == '-') {
            if (!current->dash) {
                current->dash = morse_node_create('\0');
            }
            current = current->dash;
        }
    }

    current->character = character;
}

/*
 * Build the complete Morse tree
 */
MorseNode *morse_tree_build(void) {
    MorseNode *root = morse_node_create('\0');

    for (int i = 0; MORSE_TABLE[i].character != 0; i++) {
        morse_tree_insert(root, MORSE_TABLE[i].character,
                          MORSE_TABLE[i].morse);
    }

    return root;
}

/*
 * Decode a single Morse code using the tree — O(code length)
 */
char morse_tree_decode(MorseNode *root, const char *code) {
    MorseNode *current = root;

    for (const char *p = code; *p; p++) {
        if (*p == '.' && current->dot) {
            current = current->dot;
        } else if (*p == '-' && current->dash) {
            current = current->dash;
        } else {
            return '\0';  /* Invalid code */
        }
    }

    return current->character;
}

/*
 * Free the tree
 */
void morse_tree_destroy(MorseNode *node) {
    if (node) {
        morse_tree_destroy(node->dot);
        morse_tree_destroy(node->dash);
        free(node);
    }
}
```

---

## The Dwarven Extensions — Iglishmêk

The Dwarves extended standard Morse code with special sequences
for common mining and smithing terms:

```c
/*
 * Dwarven Morse Extensions (Iglishmêk)
 *
 * Special codes for common Dwarven words and phrases
 */

typedef struct {
    const char *dwarven;      /* Khuzdul word */
    const char *meaning;      /* Common tongue */
    const char *morse;        /* Special Morse code */
} IglishmekEntry;

static const IglishmekEntry IGLISHMEK_TABLE[] = {
    /* Greetings and common phrases */
    {"KHAZAD", "Dwarves", "-.- .... .- --.. .- -.."},
    {"AIMENU", "Upon you", ".- .. -- . -. ..-"},
    {"BARUK", "Axes", "-... .- .-. ..- -.-"},

    /* Mining terms (single prosigns) */
    {"ORE", "Ore found", "--- .-. ."},
    {"MITHRIL", "Mithril!", "-- .. - .... .-. .. .-.."},
    {"GOLD", "Gold vein", "--. --- .-.. -.."},
    {"DANGER", "Cave-in!", "-.. .- -. --. . .-."},

    /* Emergency prosigns */
    {"SOS", "Emergency", "... --- ..."},
    {"STOP", "Full stop", "... - --- .--."},
    {"BREAK", "Break", "-... .-. . .- -.-"},

    /* Acknowledgments */
    {"ROGAKH", "Understood", ".-. --- --. .- -.- ...."},

    {NULL, NULL, NULL}
};

/*
 * Look up Dwarven prosign
 */
const char *lookup_iglishmek(const char *word) {
    for (int i = 0; IGLISHMEK_TABLE[i].dwarven != NULL; i++) {
        if (strcasecmp(IGLISHMEK_TABLE[i].dwarven, word) == 0) {
            return IGLISHMEK_TABLE[i].morse;
        }
    }
    return NULL;
}

/*
 * Print Iglishmêk reference card
 */
void print_iglishmek_reference(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              IGLISHMÊK - DWARVEN MORSE EXTENSIONS            ║\n");
    printf("╠════════════════╤═══════════════════╤═════════════════════════╣\n");
    printf("║    Khuzdul     │      Meaning      │         Morse           ║\n");
    printf("╠════════════════╪═══════════════════╪═════════════════════════╣\n");

    for (int i = 0; IGLISHMEK_TABLE[i].dwarven != NULL; i++) {
        printf("║  %-12s  │  %-15s  │  %-21s  ║\n",
               IGLISHMEK_TABLE[i].dwarven,
               IGLISHMEK_TABLE[i].meaning,
               IGLISHMEK_TABLE[i].morse);
    }

    printf("╚════════════════╧═══════════════════╧═════════════════════════╝\n");
}
```

---

## Timing and Audio Generation

The true art of Morse code is in the rhythm. The Dwarves were
precise about timing:

```c
/*
 * Morse Code Timing Generator
 *
 * Generates timing pulses for audio or visual output
 */

#define UNIT_MS         100     /* Base unit: 100 milliseconds */
#define DIT_DURATION    (1 * UNIT_MS)
#define DAH_DURATION    (3 * UNIT_MS)
#define SYMBOL_GAP      (1 * UNIT_MS)
#define LETTER_GAP      (3 * UNIT_MS)
#define WORD_GAP        (7 * UNIT_MS)

typedef enum {
    PULSE_ON,       /* Key pressed, current flowing */
    PULSE_OFF       /* Key released, silence */
} PulseState;

typedef struct {
    PulseState state;
    int duration_ms;
} MorsePulse;

typedef struct {
    MorsePulse *pulses;
    int count;
    int capacity;
} MorseSignal;

/*
 * Create a signal buffer
 */
MorseSignal *morse_signal_create(int capacity) {
    MorseSignal *sig = malloc(sizeof(MorseSignal));
    if (!sig) return NULL;

    sig->pulses = malloc(capacity * sizeof(MorsePulse));
    if (!sig->pulses) {
        free(sig);
        return NULL;
    }

    sig->count = 0;
    sig->capacity = capacity;
    return sig;
}

/*
 * Add a pulse to the signal
 */
void morse_signal_add(MorseSignal *sig, PulseState state, int duration) {
    if (sig->count >= sig->capacity) return;

    sig->pulses[sig->count].state = state;
    sig->pulses[sig->count].duration_ms = duration;
    sig->count++;
}

/*
 * Convert Morse string to timing signal
 */
MorseSignal *morse_to_signal(const char *morse) {
    MorseSignal *sig = morse_signal_create(strlen(morse) * 4);
    if (!sig) return NULL;

    for (const char *p = morse; *p; p++) {
        switch (*p) {
            case '.':
                morse_signal_add(sig, PULSE_ON, DIT_DURATION);
                morse_signal_add(sig, PULSE_OFF, SYMBOL_GAP);
                break;

            case '-':
                morse_signal_add(sig, PULSE_ON, DAH_DURATION);
                morse_signal_add(sig, PULSE_OFF, SYMBOL_GAP);
                break;

            case ' ':
                /* Replace last symbol gap with letter gap */
                if (sig->count > 0) {
                    sig->pulses[sig->count - 1].duration_ms = LETTER_GAP;
                }
                break;

            case '/':
                /* Word gap */
                if (sig->count > 0) {
                    sig->pulses[sig->count - 1].duration_ms = WORD_GAP;
                }
                break;
        }
    }

    return sig;
}

/*
 * Calculate total signal duration
 */
int morse_signal_duration(MorseSignal *sig) {
    int total = 0;
    for (int i = 0; i < sig->count; i++) {
        total += sig->pulses[i].duration_ms;
    }
    return total;
}

/*
 * Print signal as ASCII art timeline
 */
void morse_signal_print(MorseSignal *sig) {
    printf("\nSignal Timeline:\n");
    printf("TIME (ms):  ");

    int time = 0;
    for (int i = 0; i < sig->count; i++) {
        printf("%d", time);
        int duration = sig->pulses[i].duration_ms;
        time += duration;

        /* Padding */
        for (int j = 0; j < duration / 25; j++) {
            printf(" ");
        }
    }
    printf("\n");

    printf("LEVEL:      ");
    for (int i = 0; i < sig->count; i++) {
        char c = (sig->pulses[i].state == PULSE_ON) ? '#' : '_';
        int duration = sig->pulses[i].duration_ms / 25;
        for (int j = 0; j < duration; j++) {
            printf("%c", c);
        }
    }
    printf("\n");
}

void morse_signal_destroy(MorseSignal *sig) {
    if (sig) {
        free(sig->pulses);
        free(sig);
    }
}
```

---

## Learning Morse Code — The Farnsworth Method

The Dwarves taught new operators using the Farnsworth method:
send characters at full speed, but with extra gaps between them.

```c
/*
 * Farnsworth Timing for Learning
 *
 * Character speed: 18 WPM (standard)
 * Effective speed: 5 WPM (with extra gaps)
 */

typedef struct {
    int char_wpm;       /* Character speed in WPM */
    int effective_wpm;  /* Overall speed in WPM */

    /* Calculated timings */
    int dit_ms;
    int dah_ms;
    int symbol_gap_ms;
    int letter_gap_ms;
    int word_gap_ms;
} FarnsworthTiming;

/*
 * Calculate Farnsworth timing
 *
 * Standard: 1 WPM = 50 dit-units per word "PARIS"
 * Dit duration at X WPM = 1200 / X milliseconds
 */
void calculate_farnsworth(FarnsworthTiming *timing,
                           int char_wpm, int effective_wpm) {
    timing->char_wpm = char_wpm;
    timing->effective_wpm = effective_wpm;

    /* Character timing (at char_wpm) */
    int dit_unit = 1200 / char_wpm;
    timing->dit_ms = dit_unit;
    timing->dah_ms = dit_unit * 3;
    timing->symbol_gap_ms = dit_unit;

    /* If effective WPM is slower, stretch the gaps */
    if (effective_wpm < char_wpm) {
        /* Calculate delay to add */
        int ta = (60000.0 / effective_wpm - 37.2 * dit_unit) / 19.0;

        timing->letter_gap_ms = 3 * ta;
        timing->word_gap_ms = 7 * ta;
    } else {
        timing->letter_gap_ms = dit_unit * 3;
        timing->word_gap_ms = dit_unit * 7;
    }
}

void print_farnsworth_timing(FarnsworthTiming *timing) {
    printf("\nFarnsworth Timing:\n");
    printf("  Character speed: %d WPM\n", timing->char_wpm);
    printf("  Effective speed: %d WPM\n", timing->effective_wpm);
    printf("  Dit:        %4d ms\n", timing->dit_ms);
    printf("  Dah:        %4d ms\n", timing->dah_ms);
    printf("  Symbol gap: %4d ms\n", timing->symbol_gap_ms);
    printf("  Letter gap: %4d ms\n", timing->letter_gap_ms);
    printf("  Word gap:   %4d ms\n", timing->word_gap_ms);
}
```

---

## Scavenger Hunt — Clue #24

*"Morse code is a classic example of a prefix code — no code
is a prefix of another. This property is essential for unambiguous
decoding. In Sedgewick's discussion of data compression (Chapter 22),
he explains Huffman coding, another prefix code. What makes a prefix
code uniquely decodable, and how is the Morse tree related to a
Huffman tree?"*

🔍 **Your Quest**: Read Sedgewick's Algorithms in C, Chapter 22.
Find the section on prefix codes and Huffman trees. How does the
binary tree structure guarantee unique decoding?

---

## Exercises — The Trials of the Code

### Trial 1: The Encoder
```c
/*
 * Create a complete Morse encoder that:
 * 1. Accepts arbitrary text input
 * 2. Handles unknown characters gracefully
 * 3. Generates timing signals for playback
 * 4. Supports Farnsworth timing for learning
 */
```

### Trial 2: The Decoder
```c
/*
 * Create a Morse decoder that:
 * 1. Uses the binary tree for efficient lookup
 * 2. Handles timing variations (±20%)
 * 3. Detects and corrects simple errors
 * 4. Learns operator's personal timing style
 */
```

### Trial 3: The Practice Tool
```c
/*
 * Create a Morse code practice system:
 * 1. Random word generator
 * 2. Speed training (gradually increase WPM)
 * 3. Score tracking
 * 4. Common error analysis
 */
```

---

*Thus ends the second chapter of the Palantír Protocol.
We have learned the secret language of dots and dashes.
In the final chapter, we shall build the telegraph system
that carries these signals across the mountain kingdoms.*

---

# Part V-C: The Signal Lines — Telegraph Implementation

*In which we learn to build the complete telegraph system:
the key, the relay, the sounder, and the network
that connects all the halls of Khazad-dûm.*

## The Telegraph Components

```
The Dwarven Telegraph System:

  SENDER                    RECEIVER
  ══════                    ════════

  ┌───────┐                ┌───────────┐
  │ KEY   │──────┬─────────│  RELAY    │
  │ ┌─┐   │      │         │  ╭───╮    │
  │ │ ├───┤      │         │  │ ≋ │────┼──┐
  │ └─┘   │      │         │  ╰───╯    │  │
  └───────┘      │         └───────────┘  │
      │          │              │         │
      │      ┌───┴───┐          │    ┌────┴────┐
      │      │ POWER │          │    │ SOUNDER │
      │      │  48V  │          │    │  CLICK  │
      │      └───────┘          │    └─────────┘
      │          │              │
      └──────────┴──────────────┘
               LINE
```

### The Telegraph Key

```c
/*
 * Telegraph Key — The sender's interface
 */

typedef enum {
    KEY_UP,     /* Key released */
    KEY_DOWN    /* Key pressed */
} KeyState;

typedef struct {
    KeyState state;
    uint64_t state_change_time;  /* When state last changed (ms) */
    uint64_t key_down_start;     /* When current press started */

    /* Statistics */
    int total_presses;
    uint64_t total_down_time;
} TelegraphKey;

/*
 * Get current time in milliseconds
 */
uint64_t get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

TelegraphKey *telegraph_key_create(void) {
    TelegraphKey *key = malloc(sizeof(TelegraphKey));
    if (!key) return NULL;

    key->state = KEY_UP;
    key->state_change_time = get_time_ms();
    key->key_down_start = 0;
    key->total_presses = 0;
    key->total_down_time = 0;

    return key;
}

void telegraph_key_press(TelegraphKey *key) {
    if (key->state == KEY_UP) {
        key->state = KEY_DOWN;
        key->state_change_time = get_time_ms();
        key->key_down_start = key->state_change_time;
        key->total_presses++;
    }
}

void telegraph_key_release(TelegraphKey *key) {
    if (key->state == KEY_DOWN) {
        key->state = KEY_UP;
        uint64_t now = get_time_ms();
        key->total_down_time += now - key->key_down_start;
        key->state_change_time = now;
    }
}

/*
 * Get duration of current state
 */
uint64_t telegraph_key_state_duration(TelegraphKey *key) {
    return get_time_ms() - key->state_change_time;
}
```

### The Telegraph Line

```c
/*
 * Telegraph Line — The transmission medium
 */

typedef struct {
    char name[50];
    double length_km;           /* Length in kilometers */
    double resistance_per_km;   /* Ohms per km */
    double capacitance_per_km;  /* Farads per km */

    /* Current state */
    bool current_flowing;
    double voltage_at_receiver;

    /* Statistics */
    uint64_t total_on_time;
    uint64_t message_count;
} TelegraphLine;

TelegraphLine *telegraph_line_create(const char *name,
                                      double length_km) {
    TelegraphLine *line = malloc(sizeof(TelegraphLine));
    if (!line) return NULL;

    strncpy(line->name, name, sizeof(line->name) - 1);
    line->length_km = length_km;
    line->resistance_per_km = 10.0;   /* 10 ohms/km typical */
    line->capacitance_per_km = 0.05e-6; /* 0.05 µF/km typical */
    line->current_flowing = false;
    line->voltage_at_receiver = 0;
    line->total_on_time = 0;
    line->message_count = 0;

    return line;
}

/*
 * Calculate line characteristics
 */
double telegraph_line_resistance(TelegraphLine *line) {
    return line->length_km * line->resistance_per_km;
}

double telegraph_line_delay_ms(TelegraphLine *line) {
    /* Signal propagation delay (approximately speed of light in wire) */
    double speed_km_per_ms = 200.0;  /* 200,000 km/s */
    return line->length_km / speed_km_per_ms;
}

/*
 * Update line state based on sender
 */
void telegraph_line_update(TelegraphLine *line, bool key_down,
                            double source_voltage) {
    line->current_flowing = key_down;

    if (key_down) {
        /* Calculate voltage drop along line */
        double resistance = telegraph_line_resistance(line);
        double relay_resistance = 100.0;  /* Receiver relay */
        double current = source_voltage / (resistance + relay_resistance);
        line->voltage_at_receiver = current * relay_resistance;
    } else {
        line->voltage_at_receiver = 0;
    }
}
```

### The Relay and Sounder

```c
/*
 * Telegraph Relay — Receives weak signals, produces strong local signals
 */

typedef struct {
    double activation_voltage;  /* Minimum voltage to activate */
    double release_voltage;     /* Voltage below which relay releases */

    bool is_activated;
    uint64_t activation_time;

    /* Connected sounder */
    struct TelegraphSounder *sounder;
} TelegraphRelay;

/*
 * Telegraph Sounder — Makes the clicking sound
 */
typedef struct TelegraphSounder {
    bool is_clicking;
    uint64_t last_click_start;
    uint64_t last_click_end;

    /* For decoding */
    MorsePulse *pulse_buffer;
    int pulse_count;
    int pulse_capacity;
} TelegraphSounder;

TelegraphSounder *telegraph_sounder_create(int buffer_capacity) {
    TelegraphSounder *sounder = malloc(sizeof(TelegraphSounder));
    if (!sounder) return NULL;

    sounder->is_clicking = false;
    sounder->last_click_start = 0;
    sounder->last_click_end = 0;
    sounder->pulse_buffer = malloc(buffer_capacity * sizeof(MorsePulse));
    sounder->pulse_count = 0;
    sounder->pulse_capacity = buffer_capacity;

    return sounder;
}

TelegraphRelay *telegraph_relay_create(double activation_v,
                                        double release_v) {
    TelegraphRelay *relay = malloc(sizeof(TelegraphRelay));
    if (!relay) return NULL;

    relay->activation_voltage = activation_v;
    relay->release_voltage = release_v;
    relay->is_activated = false;
    relay->activation_time = 0;
    relay->sounder = NULL;

    return relay;
}

void telegraph_relay_connect_sounder(TelegraphRelay *relay,
                                      TelegraphSounder *sounder) {
    relay->sounder = sounder;
}

/*
 * Update relay based on line voltage
 */
void telegraph_relay_update(TelegraphRelay *relay, double voltage) {
    uint64_t now = get_time_ms();

    bool was_activated = relay->is_activated;

    /* Hysteresis: different thresholds for activation and release */
    if (!relay->is_activated && voltage >= relay->activation_voltage) {
        relay->is_activated = true;
        relay->activation_time = now;
    } else if (relay->is_activated && voltage < relay->release_voltage) {
        relay->is_activated = false;
    }

    /* Update connected sounder */
    if (relay->sounder) {
        if (relay->is_activated && !was_activated) {
            /* Just activated — start click */
            relay->sounder->is_clicking = true;
            relay->sounder->last_click_start = now;

            /* Record the silence that just ended */
            if (relay->sounder->last_click_end > 0) {
                uint64_t silence = now - relay->sounder->last_click_end;
                if (relay->sounder->pulse_count < relay->sounder->pulse_capacity) {
                    relay->sounder->pulse_buffer[relay->sounder->pulse_count].state = PULSE_OFF;
                    relay->sounder->pulse_buffer[relay->sounder->pulse_count].duration_ms = silence;
                    relay->sounder->pulse_count++;
                }
            }
        } else if (!relay->is_activated && was_activated) {
            /* Just deactivated — end click */
            relay->sounder->is_clicking = false;
            relay->sounder->last_click_end = now;

            /* Record the click that just ended */
            uint64_t click_duration = now - relay->sounder->last_click_start;
            if (relay->sounder->pulse_count < relay->sounder->pulse_capacity) {
                relay->sounder->pulse_buffer[relay->sounder->pulse_count].state = PULSE_ON;
                relay->sounder->pulse_buffer[relay->sounder->pulse_count].duration_ms = click_duration;
                relay->sounder->pulse_count++;
            }
        }
    }
}
```

### The Complete Telegraph Station

```c
/*
 * Complete Telegraph Station
 */

typedef struct {
    char station_id[20];
    char station_name[50];

    TelegraphKey *key;
    TelegraphRelay *relay;
    TelegraphSounder *sounder;

    /* Connected lines (can send/receive on multiple) */
    TelegraphLine **lines;
    int line_count;

    /* Message buffers */
    char outgoing_message[1024];
    char incoming_message[1024];
    int incoming_pos;

    /* Morse tree for decoding */
    MorseNode *morse_tree;
} TelegraphStation;

TelegraphStation *telegraph_station_create(const char *id,
                                            const char *name) {
    TelegraphStation *station = malloc(sizeof(TelegraphStation));
    if (!station) return NULL;

    strncpy(station->station_id, id, sizeof(station->station_id) - 1);
    strncpy(station->station_name, name, sizeof(station->station_name) - 1);

    station->key = telegraph_key_create();
    station->sounder = telegraph_sounder_create(1000);
    station->relay = telegraph_relay_create(10.0, 5.0);
    telegraph_relay_connect_sounder(station->relay, station->sounder);

    station->lines = NULL;
    station->line_count = 0;

    station->outgoing_message[0] = '\0';
    station->incoming_message[0] = '\0';
    station->incoming_pos = 0;

    station->morse_tree = morse_tree_build();

    return station;
}

void telegraph_station_add_line(TelegraphStation *station,
                                 TelegraphLine *line) {
    station->lines = realloc(station->lines,
                              (station->line_count + 1) * sizeof(TelegraphLine *));
    station->lines[station->line_count++] = line;
}

/*
 * Send a message (converts to Morse and sends)
 */
void telegraph_station_send(TelegraphStation *station,
                             const char *message) {
    char *morse = text_to_morse(message);
    if (!morse) return;

    strncpy(station->outgoing_message, morse,
            sizeof(station->outgoing_message) - 1);

    printf("\n[%s] Sending: %s\n", station->station_id, message);
    printf("[%s] Morse: %s\n", station->station_id, morse);

    /* Simulate sending each symbol */
    for (const char *p = morse; *p; p++) {
        switch (*p) {
            case '.':
                telegraph_key_press(station->key);
                /* In real system: wait DIT_DURATION */
                telegraph_key_release(station->key);
                break;

            case '-':
                telegraph_key_press(station->key);
                /* In real system: wait DAH_DURATION */
                telegraph_key_release(station->key);
                break;

            case ' ':
                /* Letter gap */
                break;

            case '/':
                /* Word gap */
                break;
        }
    }

    free(morse);
}

/*
 * Decode received pulses into text
 */
void telegraph_station_decode_received(TelegraphStation *station) {
    TelegraphSounder *sounder = station->sounder;
    char morse_buffer[256];
    int morse_pos = 0;

    printf("\n[%s] Decoding received pulses...\n", station->station_id);

    for (int i = 0; i < sounder->pulse_count; i++) {
        MorsePulse *pulse = &sounder->pulse_buffer[i];

        if (pulse->state == PULSE_ON) {
            /* Distinguish dit from dah */
            if (pulse->duration_ms < 200) {
                morse_buffer[morse_pos++] = '.';
            } else {
                morse_buffer[morse_pos++] = '-';
            }
        } else {
            /* Silence — check for letter/word gap */
            if (pulse->duration_ms > 500) {
                /* Word gap */
                morse_buffer[morse_pos++] = ' ';
                morse_buffer[morse_pos++] = '/';
                morse_buffer[morse_pos++] = ' ';
            } else if (pulse->duration_ms > 200) {
                /* Letter gap */
                morse_buffer[morse_pos++] = ' ';
            }
            /* Symbol gaps are implicit */
        }
    }

    morse_buffer[morse_pos] = '\0';
    printf("[%s] Received Morse: %s\n", station->station_id, morse_buffer);

    char *decoded = morse_to_text(morse_buffer);
    if (decoded) {
        strncpy(station->incoming_message, decoded,
                sizeof(station->incoming_message) - 1);
        printf("[%s] Decoded: %s\n", station->station_id, decoded);
        free(decoded);
    }
}

void telegraph_station_destroy(TelegraphStation *station) {
    if (station) {
        free(station->key);
        free(station->sounder->pulse_buffer);
        free(station->sounder);
        free(station->relay);
        free(station->lines);
        morse_tree_destroy(station->morse_tree);
        free(station);
    }
}
```

---

## The Telegraph Network

Now we connect multiple stations into a network:

```c
/*
 * Telegraph Network — Connects all stations
 */

#define MAX_STATIONS 20

typedef struct {
    TelegraphStation *stations[MAX_STATIONS];
    int station_count;

    TelegraphLine *lines[MAX_STATIONS * MAX_STATIONS];
    int line_count;

    /* Network routing (which lines connect which stations) */
    int adjacency[MAX_STATIONS][MAX_STATIONS];  /* Line index or -1 */

    /* Power supply */
    PowerStation *power;
} TelegraphNetwork;

TelegraphNetwork *telegraph_network_create(void) {
    TelegraphNetwork *net = calloc(1, sizeof(TelegraphNetwork));
    if (!net) return NULL;

    /* Initialize adjacency matrix */
    for (int i = 0; i < MAX_STATIONS; i++) {
        for (int j = 0; j < MAX_STATIONS; j++) {
            net->adjacency[i][j] = -1;
        }
    }

    return net;
}

int telegraph_network_add_station(TelegraphNetwork *net,
                                   TelegraphStation *station) {
    if (net->station_count >= MAX_STATIONS) return -1;

    net->stations[net->station_count] = station;
    return net->station_count++;
}

void telegraph_network_connect(TelegraphNetwork *net,
                                int station_a, int station_b,
                                double distance_km) {
    if (station_a < 0 || station_a >= net->station_count) return;
    if (station_b < 0 || station_b >= net->station_count) return;

    char line_name[100];
    snprintf(line_name, sizeof(line_name), "%s-%s Line",
             net->stations[station_a]->station_id,
             net->stations[station_b]->station_id);

    TelegraphLine *line = telegraph_line_create(line_name, distance_km);
    net->lines[net->line_count] = line;

    /* Add to adjacency */
    net->adjacency[station_a][station_b] = net->line_count;
    net->adjacency[station_b][station_a] = net->line_count;

    /* Connect line to stations */
    telegraph_station_add_line(net->stations[station_a], line);
    telegraph_station_add_line(net->stations[station_b], line);

    net->line_count++;
}

/*
 * Find station by ID
 */
int telegraph_network_find_station(TelegraphNetwork *net,
                                    const char *station_id) {
    for (int i = 0; i < net->station_count; i++) {
        if (strcmp(net->stations[i]->station_id, station_id) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * Route a message through the network
 * Uses the shortest path between stations
 */
void telegraph_network_route_message(TelegraphNetwork *net,
                                      const char *from_id,
                                      const char *to_id,
                                      const char *message) {
    int from_idx = telegraph_network_find_station(net, from_id);
    int to_idx = telegraph_network_find_station(net, to_id);

    if (from_idx < 0 || to_idx < 0) {
        printf("Station not found!\n");
        return;
    }

    /* For now, assume direct connection */
    int line_idx = net->adjacency[from_idx][to_idx];
    if (line_idx < 0) {
        printf("No direct connection. Multi-hop routing not implemented.\n");
        return;
    }

    printf("\n");
    printf("════════════════════════════════════════════════════════\n");
    printf("  MESSAGE TRANSMISSION\n");
    printf("════════════════════════════════════════════════════════\n");
    printf("  From: %s (%s)\n",
           net->stations[from_idx]->station_name,
           net->stations[from_idx]->station_id);
    printf("  To:   %s (%s)\n",
           net->stations[to_idx]->station_name,
           net->stations[to_idx]->station_id);
    printf("  Line: %s (%.1f km)\n",
           net->lines[line_idx]->name,
           net->lines[line_idx]->length_km);
    printf("════════════════════════════════════════════════════════\n");

    /* Send the message */
    telegraph_station_send(net->stations[from_idx], message);

    /* Simulate transmission and decoding */
    printf("\n[Transmission in progress...]\n");

    /* The receiving station would decode */
    /* telegraph_station_decode_received(net->stations[to_idx]); */
}

/*
 * Print network status
 */
void telegraph_network_print_status(TelegraphNetwork *net) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║             DWARVEN TELEGRAPH NETWORK STATUS                  ║\n");
    printf("╠═══════════════════════════════════════════════════════════════╣\n");
    printf("║  Stations: %-3d                                               ║\n",
           net->station_count);
    printf("║  Lines:    %-3d                                               ║\n",
           net->line_count);
    printf("╠═══════════════════════════════════════════════════════════════╣\n");
    printf("║  STATIONS:                                                    ║\n");

    for (int i = 0; i < net->station_count; i++) {
        printf("║    [%s] %-40s      ║\n",
               net->stations[i]->station_id,
               net->stations[i]->station_name);
    }

    printf("╠═══════════════════════════════════════════════════════════════╣\n");
    printf("║  LINES:                                                       ║\n");

    for (int i = 0; i < net->line_count; i++) {
        printf("║    %-45s %6.1f km ║\n",
               net->lines[i]->name,
               net->lines[i]->length_km);
    }

    printf("╚═══════════════════════════════════════════════════════════════╝\n");
}
```

---

## Complete Demo: The Khazad-dûm Network

```c
/*
 * Build and demonstrate the Khazad-dûm telegraph network
 */
void demo_khazad_dum_network(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════════════════\n");
    printf("     KHAZAD-DÛM TELEGRAPH NETWORK DEMONSTRATION              \n");
    printf("══════════════════════════════════════════════════════════════\n");

    /* Create the network */
    TelegraphNetwork *net = telegraph_network_create();

    /* Create stations */
    TelegraphStation *great_gates = telegraph_station_create(
        "GG", "Great Gates of Moria");
    TelegraphStation *bridge = telegraph_station_create(
        "BR", "Bridge of Khazad-dûm");
    TelegraphStation *deep = telegraph_station_create(
        "DP", "The Deeps");
    TelegraphStation *mines = telegraph_station_create(
        "MN", "Mithril Mines");
    TelegraphStation *eastern = telegraph_station_create(
        "ES", "Eastern Doors");

    /* Add stations to network */
    int gg_idx = telegraph_network_add_station(net, great_gates);
    int br_idx = telegraph_network_add_station(net, bridge);
    int dp_idx = telegraph_network_add_station(net, deep);
    int mn_idx = telegraph_network_add_station(net, mines);
    int es_idx = telegraph_network_add_station(net, eastern);

    /* Connect stations */
    telegraph_network_connect(net, gg_idx, br_idx, 5.0);   /* 5 km */
    telegraph_network_connect(net, br_idx, dp_idx, 3.0);   /* 3 km */
    telegraph_network_connect(net, dp_idx, mn_idx, 8.0);   /* 8 km */
    telegraph_network_connect(net, br_idx, es_idx, 12.0);  /* 12 km */
    telegraph_network_connect(net, mn_idx, es_idx, 6.0);   /* 6 km */

    /* Print network status */
    telegraph_network_print_status(net);

    /* Send a message */
    telegraph_network_route_message(net, "GG", "BR",
        "BARUK KHAZAD KHAZAD AIMENU");

    printf("\n");
    printf("══════════════════════════════════════════════════════════════\n");
    printf("     DEMONSTRATION COMPLETE                                   \n");
    printf("══════════════════════════════════════════════════════════════\n");
}
```

---

## Scavenger Hunt — Clue #25

*"The telegraph network is itself a graph! The stations are vertices,
and the lines are edges. All of our graph algorithms apply.
In Bhargava's Chapter 6, he shows how to find the shortest path
between friends. How would you find the shortest path between
two telegraph stations, and what does 'shortest' mean in this context?"*

🔍 **Your Quest**: Review Grokking Algorithms Chapter 6.
Apply BFS to find the path with fewest hops, or Dijkstra to
find the path with least total line length.

---

## Exercises — The Trials of the Signal Lines

### Trial 1: The Network Simulator
```c
/*
 * Build a complete network simulator that:
 * 1. Simulates message transmission in real-time
 * 2. Calculates propagation delays
 * 3. Handles line failures gracefully
 * 4. Routes around failed lines using graph algorithms
 */
```

### Trial 2: The Message Queue
```c
/*
 * Implement a priority queue for messages:
 * 1. Emergency messages (DANGER, SOS) get highest priority
 * 2. Regular messages queue in FIFO order
 * 3. Stations can batch multiple messages
 * 4. Calculate estimated delivery times
 */
```

### Trial 3: The Full Integration
```c
/*
 * Integrate all PartsDB data with the telegraph:
 * 1. Send inventory alerts between warehouses
 * 2. Request parts transfers via telegraph
 * 3. Confirm deliveries with acknowledgment codes
 * 4. Generate daily status reports in Morse
 */
```

---

*Thus ends the Palantír Protocol.
We have harnessed the waters of power,
learned the secret language of Iglishmêk,
and built a network to span the mountain kingdoms.

In the final part, we shall embark on the Nauglamír Hunt —
a scavenger hunt through the great algorithm texts,
applying all we have learned to master the art.*

---

[Continue to Part VI: The Nauglamír Hunt — The Grand Challenge →](./THE_SILMARILLION_OF_ALGORITHMS_PART6.md)

---
