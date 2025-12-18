/**
 * PartsDB CLI - Main Entry Point
 * Text-based interface for Linux and embedded systems (C23)
 */

#include "partsdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 256
#define DEFAULT_DB_PATH "./parts.db"

/* Terminal colors (optional, can be disabled for minimal systems) */
#ifndef PARTSDB_NO_COLOR
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#else
#define COLOR_RESET   ""
#define COLOR_BOLD    ""
#define COLOR_RED     ""
#define COLOR_GREEN   ""
#define COLOR_YELLOW  ""
#define COLOR_BLUE    ""
#define COLOR_CYAN    ""
#endif

static partsdb_t *db = nullptr;

/* Print banner */
static void print_banner(void) {
    printf(COLOR_CYAN);
    printf("╔════════════════════════════════════════╗\n");
    printf("║      PartsDB CLI v%d.%d.%d               ║\n",
           PARTSDB_VERSION_MAJOR, PARTSDB_VERSION_MINOR, PARTSDB_VERSION_PATCH);
    printf("║   Lightweight Parts Database Manager   ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf(COLOR_RESET "\n");
}

/* Print help */
static void print_help(void) {
    printf(COLOR_BOLD "Commands:" COLOR_RESET "\n");
    printf("  " COLOR_GREEN "list" COLOR_RESET " [limit]       - List all parts\n");
    printf("  " COLOR_GREEN "search" COLOR_RESET " <query>     - Search parts by name/number/mfr\n");
    printf("  " COLOR_GREEN "show" COLOR_RESET " <id|number>   - Show part details\n");
    printf("  " COLOR_GREEN "add" COLOR_RESET "                - Add new part (interactive)\n");
    printf("  " COLOR_GREEN "qty" COLOR_RESET " <id> <+/-n>    - Adjust quantity\n");
    printf("  " COLOR_GREEN "low" COLOR_RESET "                - Show low stock items\n");
    printf("  " COLOR_GREEN "cat" COLOR_RESET "                - List categories\n");
    printf("  " COLOR_GREEN "img" COLOR_RESET " <id>           - Show part image (ASCII)\n");
    printf("  " COLOR_GREEN "help" COLOR_RESET "               - Show this help\n");
    printf("  " COLOR_GREEN "quit" COLOR_RESET "               - Exit program\n");
    printf("\n");
}

/* Print part in table format */
static void print_part_row(const part_t *part) {
    const char *qty_color = COLOR_GREEN;
    if (part->quantity <= 0) {
        qty_color = COLOR_RED;
    } else if (part->quantity <= part->min_quantity) {
        qty_color = COLOR_YELLOW;
    }

    printf("│ %-8lld │ %-15.15s │ %-25.25s │ %s%-6d%s │ %-12.12s │\n",
           (long long)part->id,
           part->part_number,
           part->name,
           qty_color, part->quantity, COLOR_RESET,
           part->location[0] ? part->location : "-");
}

/* Print table header */
static void print_table_header(void) {
    printf("┌──────────┬─────────────────┬───────────────────────────┬────────┬──────────────┐\n");
    printf("│ " COLOR_BOLD "ID" COLOR_RESET "       │ " COLOR_BOLD "Part Number" COLOR_RESET "     │ "
           COLOR_BOLD "Name" COLOR_RESET "                      │ " COLOR_BOLD "Qty" COLOR_RESET "    │ "
           COLOR_BOLD "Location" COLOR_RESET "     │\n");
    printf("├──────────┼─────────────────┼───────────────────────────┼────────┼──────────────┤\n");
}

static void print_table_footer(void) {
    printf("└──────────┴─────────────────┴───────────────────────────┴────────┴──────────────┘\n");
}

/* Command: list */
static void cmd_list(int limit) {
    part_t *parts = nullptr;
    size_t count = 0;

    partsdb_error_t err = partsdb_get_all(db, &parts, &count, limit > 0 ? limit : 50, 0);
    if (err != PARTSDB_OK) {
        printf(COLOR_RED "Error: %s\n" COLOR_RESET, partsdb_error_string(err));
        return;
    }

    if (count == 0) {
        printf(COLOR_YELLOW "No parts found.\n" COLOR_RESET);
        return;
    }

    printf("\n");
    print_table_header();
    for (size_t i = 0; i < count; i++) {
        print_part_row(&parts[i]);
    }
    print_table_footer();
    printf(COLOR_CYAN "Found %zu part(s)\n" COLOR_RESET "\n", count);

    partsdb_free_parts(parts);
}

/* Command: search */
static void cmd_search(const char *query) {
    if (query == nullptr || query[0] == '\0') {
        printf(COLOR_RED "Usage: search <query>\n" COLOR_RESET);
        return;
    }

    part_t *parts = nullptr;
    size_t count = 0;

    partsdb_error_t err = partsdb_search(db, query, &parts, &count);
    if (err != PARTSDB_OK) {
        printf(COLOR_RED "Error: %s\n" COLOR_RESET, partsdb_error_string(err));
        return;
    }

    if (count == 0) {
        printf(COLOR_YELLOW "No parts found matching '%s'\n" COLOR_RESET, query);
        return;
    }

    printf("\n");
    print_table_header();
    for (size_t i = 0; i < count; i++) {
        print_part_row(&parts[i]);
    }
    print_table_footer();
    printf(COLOR_CYAN "Found %zu part(s) matching '%s'\n" COLOR_RESET "\n", count, query);

    partsdb_free_parts(parts);
}

/* Command: show */
static void cmd_show(const char *id_or_number) {
    if (id_or_number == nullptr || id_or_number[0] == '\0') {
        printf(COLOR_RED "Usage: show <id|part_number>\n" COLOR_RESET);
        return;
    }

    part_t part;
    partsdb_error_t err;

    /* Try as ID first */
    if (isdigit(id_or_number[0])) {
        int64_t id = atoll(id_or_number);
        err = partsdb_get_by_id(db, id, &part);
    } else {
        err = partsdb_get_by_number(db, id_or_number, &part);
    }

    if (err != PARTSDB_OK) {
        printf(COLOR_RED "Part not found: %s\n" COLOR_RESET, id_or_number);
        return;
    }

    const char *qty_color = COLOR_GREEN;
    if (part.quantity <= 0) {
        qty_color = COLOR_RED;
    } else if (part.quantity <= part.min_quantity) {
        qty_color = COLOR_YELLOW;
    }

    printf("\n" COLOR_BOLD "Part Details:" COLOR_RESET "\n");
    printf("┌────────────────────────────────────────────────────┐\n");
    printf("│ " COLOR_BOLD "%-14s" COLOR_RESET " %s\n", "ID:", "");
    printf("│   %-48lld │\n", (long long)part.id);
    printf("│ " COLOR_BOLD "%-14s" COLOR_RESET " %s\n", "Part Number:", "");
    printf("│   %-48s │\n", part.part_number);
    printf("│ " COLOR_BOLD "%-14s" COLOR_RESET " %s\n", "Name:", "");
    printf("│   %-48s │\n", part.name);
    printf("│ " COLOR_BOLD "%-14s" COLOR_RESET " %s\n", "Category:", "");
    printf("│   %-48s │\n", part.category[0] ? part.category : "N/A");
    printf("│ " COLOR_BOLD "%-14s" COLOR_RESET " %s\n", "Manufacturer:", "");
    printf("│   %-48s │\n", part.manufacturer[0] ? part.manufacturer : "N/A");
    printf("│ " COLOR_BOLD "%-14s" COLOR_RESET " %s\n", "Quantity:", "");
    printf("│   %s%-48d%s │\n", qty_color, part.quantity, COLOR_RESET);
    printf("│ " COLOR_BOLD "%-14s" COLOR_RESET " %s\n", "Min Quantity:", "");
    printf("│   %-48d │\n", part.min_quantity);
    printf("│ " COLOR_BOLD "%-14s" COLOR_RESET " %s\n", "Location:", "");
    printf("│   %-48s │\n", part.location[0] ? part.location : "N/A");
    if (part.description[0]) {
        printf("│ " COLOR_BOLD "%-14s" COLOR_RESET " %s\n", "Description:", "");
        printf("│   %-48.48s │\n", part.description);
    }
    printf("└────────────────────────────────────────────────────┘\n\n");
}

/* Command: add (interactive) */
static void cmd_add(void) {
    part_t part = {0};
    char input[MAX_INPUT];

    printf("\n" COLOR_BOLD "Add New Part" COLOR_RESET "\n");
    printf("────────────────────────\n");

    printf("Part Number: ");
    if (fgets(input, sizeof(input), stdin) == nullptr) return;
    input[strcspn(input, "\n")] = '\0';
    strncpy(part.part_number, input, sizeof(part.part_number) - 1);

    if (part.part_number[0] == '\0') {
        printf(COLOR_RED "Part number is required.\n" COLOR_RESET);
        return;
    }

    printf("Name: ");
    if (fgets(input, sizeof(input), stdin) == nullptr) return;
    input[strcspn(input, "\n")] = '\0';
    strncpy(part.name, input, sizeof(part.name) - 1);

    if (part.name[0] == '\0') {
        printf(COLOR_RED "Name is required.\n" COLOR_RESET);
        return;
    }

    printf("Description (optional): ");
    if (fgets(input, sizeof(input), stdin) == nullptr) return;
    input[strcspn(input, "\n")] = '\0';
    strncpy(part.description, input, sizeof(part.description) - 1);

    printf("Manufacturer (optional): ");
    if (fgets(input, sizeof(input), stdin) == nullptr) return;
    input[strcspn(input, "\n")] = '\0';
    strncpy(part.manufacturer, input, sizeof(part.manufacturer) - 1);

    printf("Initial Quantity [0]: ");
    if (fgets(input, sizeof(input), stdin) == nullptr) return;
    input[strcspn(input, "\n")] = '\0';
    part.quantity = input[0] ? atoi(input) : 0;

    printf("Location (optional): ");
    if (fgets(input, sizeof(input), stdin) == nullptr) return;
    input[strcspn(input, "\n")] = '\0';
    strncpy(part.location, input, sizeof(part.location) - 1);

    int64_t id;
    partsdb_error_t err = partsdb_create(db, &part, &id);
    if (err != PARTSDB_OK) {
        printf(COLOR_RED "Error creating part: %s\n" COLOR_RESET, partsdb_error_string(err));
        return;
    }

    printf(COLOR_GREEN "Part created with ID: %lld\n" COLOR_RESET "\n", (long long)id);
}

/* Command: qty */
static void cmd_qty(const char *id_str, const char *change_str) {
    if (id_str == nullptr || change_str == nullptr) {
        printf(COLOR_RED "Usage: qty <id> <+/-n>\n" COLOR_RESET);
        return;
    }

    int64_t id = atoll(id_str);
    int32_t change = atoi(change_str);

    const char *type = change >= 0 ? "IN" : "OUT";
    if (change < 0) change = -change;  /* Store absolute value, type indicates direction */
    change = (strcmp(type, "OUT") == 0) ? -change : change;

    partsdb_error_t err = partsdb_update_quantity(db, id, change, type);
    if (err != PARTSDB_OK) {
        printf(COLOR_RED "Error: %s\n" COLOR_RESET, partsdb_error_string(err));
        return;
    }

    printf(COLOR_GREEN "Quantity updated.\n" COLOR_RESET);
}

/* Command: low */
static void cmd_low(void) {
    part_t *parts = nullptr;
    size_t count = 0;

    partsdb_error_t err = partsdb_get_low_stock(db, &parts, &count);
    if (err != PARTSDB_OK) {
        printf(COLOR_RED "Error: %s\n" COLOR_RESET, partsdb_error_string(err));
        return;
    }

    if (count == 0) {
        printf(COLOR_GREEN "No low stock items.\n" COLOR_RESET);
        return;
    }

    printf("\n" COLOR_YELLOW "Low Stock Alert:" COLOR_RESET "\n");
    print_table_header();
    for (size_t i = 0; i < count; i++) {
        print_part_row(&parts[i]);
    }
    print_table_footer();
    printf(COLOR_YELLOW "%zu item(s) need attention\n" COLOR_RESET "\n", count);

    partsdb_free_parts(parts);
}

/* Command: cat */
static void cmd_categories(void) {
    category_t *categories = nullptr;
    size_t count = 0;

    partsdb_error_t err = partsdb_get_categories(db, &categories, &count);
    if (err != PARTSDB_OK) {
        printf(COLOR_RED "Error: %s\n" COLOR_RESET, partsdb_error_string(err));
        return;
    }

    if (count == 0) {
        printf(COLOR_YELLOW "No categories found.\n" COLOR_RESET);
        return;
    }

    printf("\n" COLOR_BOLD "Categories:" COLOR_RESET "\n");
    printf("┌──────────┬────────────────────────────────┐\n");
    printf("│ " COLOR_BOLD "ID" COLOR_RESET "       │ " COLOR_BOLD "Name" COLOR_RESET "                           │\n");
    printf("├──────────┼────────────────────────────────┤\n");
    for (size_t i = 0; i < count; i++) {
        printf("│ %-8lld │ %-30s │\n",
               (long long)categories[i].id, categories[i].name);
    }
    printf("└──────────┴────────────────────────────────┘\n\n");

    partsdb_free_categories(categories);
}

/* Command: img */
static void cmd_image(const char *id_str) {
    if (id_str == nullptr) {
        printf(COLOR_RED "Usage: img <id>\n" COLOR_RESET);
        return;
    }

    int64_t id = atoll(id_str);
    embedded_image_t image = {0};

    partsdb_error_t err = partsdb_get_image_embedded(db, id, &image);
    if (err != PARTSDB_OK) {
        printf(COLOR_YELLOW "No image available for part %lld\n" COLOR_RESET, (long long)id);
        return;
    }

    char ascii_buffer[4096];
    partsdb_render_image_ascii(&image, ascii_buffer, sizeof(ascii_buffer), 60);

    printf("\n" COLOR_BOLD "Part Image (ASCII):" COLOR_RESET "\n");
    printf("┌");
    for (int i = 0; i < 62; i++) printf("─");
    printf("┐\n");
    printf("%s", ascii_buffer);
    printf("└");
    for (int i = 0; i < 62; i++) printf("─");
    printf("┘\n");
    printf("Image size: %ux%u, compressed: %zu bytes\n\n",
           image.width, image.height, image.data_size);

    partsdb_free_image(&image);
}

/* Trim whitespace */
static char *trim(char *str) {
    while (isspace(*str)) str++;
    if (*str == '\0') return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';

    return str;
}

/* Parse and execute command */
static bool execute_command(char *line) {
    char *cmd = trim(line);
    if (cmd[0] == '\0') return true;

    char *arg1 = nullptr;
    char *arg2 = nullptr;

    /* Split into command and arguments */
    char *space = strchr(cmd, ' ');
    if (space) {
        *space = '\0';
        arg1 = trim(space + 1);

        space = strchr(arg1, ' ');
        if (space) {
            *space = '\0';
            arg2 = trim(space + 1);
        }
    }

    /* Execute command */
    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "q") == 0) {
        return false;
    } else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0 || strcmp(cmd, "?") == 0) {
        print_help();
    } else if (strcmp(cmd, "list") == 0 || strcmp(cmd, "ls") == 0 || strcmp(cmd, "l") == 0) {
        cmd_list(arg1 ? atoi(arg1) : 0);
    } else if (strcmp(cmd, "search") == 0 || strcmp(cmd, "s") == 0) {
        cmd_search(arg1);
    } else if (strcmp(cmd, "show") == 0 || strcmp(cmd, "get") == 0) {
        cmd_show(arg1);
    } else if (strcmp(cmd, "add") == 0 || strcmp(cmd, "new") == 0) {
        cmd_add();
    } else if (strcmp(cmd, "qty") == 0 || strcmp(cmd, "quantity") == 0) {
        cmd_qty(arg1, arg2);
    } else if (strcmp(cmd, "low") == 0) {
        cmd_low();
    } else if (strcmp(cmd, "cat") == 0 || strcmp(cmd, "categories") == 0) {
        cmd_categories();
    } else if (strcmp(cmd, "img") == 0 || strcmp(cmd, "image") == 0) {
        cmd_image(arg1);
    } else {
        printf(COLOR_RED "Unknown command: %s\n" COLOR_RESET, cmd);
        printf("Type 'help' for available commands.\n");
    }

    return true;
}

int main(int argc, char *argv[]) {
    const char *db_path = DEFAULT_DB_PATH;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--database") == 0) {
            if (i + 1 < argc) {
                db_path = argv[++i];
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [-d <database.db>]\n", argv[0]);
            printf("Options:\n");
            printf("  -d, --database <path>  Path to SQLite database (default: %s)\n", DEFAULT_DB_PATH);
            printf("  -h, --help             Show this help\n");
            return 0;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--command") == 0) {
            /* Single command mode */
            if (i + 1 < argc) {
                partsdb_error_t err = partsdb_open(db_path, &db);
                if (err != PARTSDB_OK) {
                    fprintf(stderr, "Error: %s\n", partsdb_error_string(err));
                    return 1;
                }
                char cmd_buf[MAX_INPUT];
                strncpy(cmd_buf, argv[i + 1], sizeof(cmd_buf) - 1);
                execute_command(cmd_buf);
                partsdb_close(db);
                return 0;
            }
        }
    }

    /* Open database */
    partsdb_error_t err = partsdb_open(db_path, &db);
    if (err != PARTSDB_OK) {
        fprintf(stderr, COLOR_RED "Error opening database '%s': %s\n" COLOR_RESET,
                db_path, partsdb_error_string(err));
        return 1;
    }

    print_banner();
    printf("Database: %s\n", db_path);
    printf("Type 'help' for commands.\n\n");

    /* Main loop */
    char line[MAX_INPUT];
    while (true) {
        printf(COLOR_BLUE "partsdb> " COLOR_RESET);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == nullptr) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (!execute_command(line)) {
            break;
        }
    }

    printf("Goodbye!\n");
    partsdb_close(db);

    return 0;
}
