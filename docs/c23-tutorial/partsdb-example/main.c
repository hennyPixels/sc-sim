/**
 * @file main.c
 * @brief PartsDB Command-Line Interface
 *
 * Demonstrates concepts from the C23 Tutorial:
 * - Part 4:  Input validation, safe parsing
 * - Part 9:  Clean command-line interface (Practice of Programming)
 * - Part 12: CERT C compliant I/O
 * - Part 13: K&R-style simplicity
 *
 * Usage:
 *   partsdb [options] <command> [args]
 *
 * Commands:
 *   list                    List all parts
 *   add <id> <name> <price> Add a new part
 *   find <id>               Find part by ID
 *   search <pattern>        Search parts by name
 *   update <id> ...         Update a part
 *   delete <id>             Delete a part
 *   low-stock <threshold>   Find parts with low stock
 *   stats                   Show database statistics
 *
 * Options:
 *   -f, --file <path>       Database file (default: parts.db)
 *   -h, --help              Show help
 *   -v, --version           Show version
 */

#include "partsdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>

/*============================================================================
 * Constants and Configuration
 *============================================================================*/

#define PROGRAM_NAME "partsdb"
#define DEFAULT_DB_FILE "parts.db"
#define MAX_RESULTS 100

/*============================================================================
 * Error Handling (Part 9: K&R style)
 *============================================================================*/

static const char *prog_name = PROGRAM_NAME;

/* Print error message and exit */
static void
fatal(const char *fmt, ...)
{
    fprintf(stderr, "%s: ", prog_name);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

/* Print warning message */
static void
warn(const char *fmt, ...)
{
    fprintf(stderr, "%s: warning: ", prog_name);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/* Print PartsDB error */
static void
print_db_error(PartsDBError err, const char *context)
{
    fprintf(stderr, "%s: %s: %s\n",
            prog_name, context, partsdb_error_string(err));
}

/*============================================================================
 * Input Parsing (Part 4: Safe parsing)
 *============================================================================*/

/* Parse uint32_t with validation */
static bool
parse_uint32(const char *str, uint32_t *out)
{
    if (!str || !*str) {
        return false;
    }

    /* Skip whitespace */
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (!isdigit((unsigned char)*str)) {
        return false;
    }

    char *end;
    errno = 0;
    unsigned long val = strtoul(str, &end, 10);

    if (errno == ERANGE || val > UINT32_MAX) {
        return false;
    }

    /* Check for trailing garbage */
    while (*end && isspace((unsigned char)*end)) {
        end++;
    }
    if (*end != '\0') {
        return false;
    }

    *out = (uint32_t)val;
    return true;
}

/* Parse double with validation */
static bool
parse_double(const char *str, double *out)
{
    if (!str || !*str) {
        return false;
    }

    char *end;
    errno = 0;
    double val = strtod(str, &end);

    if (errno == ERANGE) {
        return false;
    }

    if (end == str) {
        return false;  /* No conversion */
    }

    /* Check for trailing garbage */
    while (*end && isspace((unsigned char)*end)) {
        end++;
    }
    if (*end != '\0') {
        return false;
    }

    /* Check for special values */
    if (val != val || val < 0.0) {  /* NaN check: val != val */
        return false;
    }

    *out = val;
    return true;
}

/*============================================================================
 * Output Formatting
 *============================================================================*/

static void
print_part(const Part *part)
{
    printf("  ID:          %u\n", part->id);
    printf("  Name:        %s\n", part->name);
    printf("  Description: %s\n",
           part->description[0] ? part->description : "(none)");
    printf("  Quantity:    %u\n", part->quantity);
    printf("  Price:       $%.2f\n", part->unit_price);
    printf("  Status:      %s\n", part->is_active ? "Active" : "Inactive");
}

static void
print_part_row(const Part *part)
{
    printf("%-8u %-30.30s %8u %10.2f %s\n",
           part->id,
           part->name,
           part->quantity,
           part->unit_price,
           part->is_active ? "Active" : "Inactive");
}

static void
print_header(void)
{
    printf("%-8s %-30s %8s %10s %s\n",
           "ID", "Name", "Qty", "Price", "Status");
    printf("%-8s %-30s %8s %10s %s\n",
           "--------", "------------------------------",
           "--------", "----------", "--------");
}

/*============================================================================
 * Commands
 *============================================================================*/

static int
cmd_list(PartsDB *db)
{
    PartsDBIterator *iter = NULL;
    PartsDBError err = partsdb_iter_create(db, &iter);
    if (err != PARTSDB_OK) {
        print_db_error(err, "creating iterator");
        return 1;
    }

    size_t count = partsdb_count(db);
    printf("Parts in database: %zu\n\n", count);

    if (count > 0) {
        print_header();

        Part part;
        while (partsdb_iter_next(iter, &part)) {
            print_part_row(&part);
        }
    }

    partsdb_iter_destroy(iter);
    return 0;
}

static int
cmd_add(PartsDB *db, int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s add <id> <name> <price> [quantity] [description]\n",
                prog_name);
        return 1;
    }

    Part part;
    partsdb_part_init(&part);

    /* Parse ID */
    if (!parse_uint32(argv[0], &part.id)) {
        fatal("invalid ID: %s", argv[0]);
    }

    /* Parse name (Part 4: Safe string copy) */
    if (strlen(argv[1]) >= PARTSDB_MAX_NAME_LEN) {
        warn("name truncated to %d characters", PARTSDB_MAX_NAME_LEN - 1);
    }
    strncpy(part.name, argv[1], PARTSDB_MAX_NAME_LEN - 1);
    part.name[PARTSDB_MAX_NAME_LEN - 1] = '\0';

    /* Parse price */
    if (!parse_double(argv[2], &part.unit_price)) {
        fatal("invalid price: %s", argv[2]);
    }

    /* Optional: quantity */
    if (argc > 3) {
        if (!parse_uint32(argv[3], &part.quantity)) {
            fatal("invalid quantity: %s", argv[3]);
        }
    }

    /* Optional: description */
    if (argc > 4) {
        strncpy(part.description, argv[4], PARTSDB_MAX_DESC_LEN - 1);
        part.description[PARTSDB_MAX_DESC_LEN - 1] = '\0';
    }

    /* Validate before insert */
    char errmsg[100];
    if (!partsdb_validate_part(&part, errmsg, sizeof(errmsg))) {
        fatal("invalid part: %s", errmsg);
    }

    /* Insert */
    PartsDBError err = partsdb_insert(db, &part);
    if (err != PARTSDB_OK) {
        print_db_error(err, "inserting part");
        return 1;
    }

    printf("Added part %u: %s\n", part.id, part.name);
    return 0;
}

static int
cmd_find(PartsDB *db, const char *id_str)
{
    uint32_t id;
    if (!parse_uint32(id_str, &id)) {
        fatal("invalid ID: %s", id_str);
    }

    Part part;
    PartsDBError err = partsdb_find_by_id(db, id, &part);

    if (err == PARTSDB_ERR_NOT_FOUND) {
        printf("Part %u not found\n", id);
        return 1;
    }

    if (err != PARTSDB_OK) {
        print_db_error(err, "finding part");
        return 1;
    }

    printf("Part found:\n");
    print_part(&part);
    return 0;
}

static int
cmd_search(PartsDB *db, const char *pattern)
{
    if (!pattern || !*pattern) {
        fatal("search pattern cannot be empty");
    }

    Part results[MAX_RESULTS];
    size_t found;

    PartsDBError err = partsdb_search_by_name(db, pattern, results,
                                               MAX_RESULTS, &found);
    if (err != PARTSDB_OK) {
        print_db_error(err, "searching");
        return 1;
    }

    printf("Search for '%s': %zu result(s)\n\n", pattern, found);

    if (found > 0) {
        print_header();
        for (size_t i = 0; i < found; i++) {
            print_part_row(&results[i]);
        }
    }

    return 0;
}

static int
cmd_update(PartsDB *db, int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s update <id> <field>=<value> ...\n", prog_name);
        fprintf(stderr, "Fields: name, price, quantity, description, active\n");
        return 1;
    }

    /* Get ID */
    uint32_t id;
    if (!parse_uint32(argv[0], &id)) {
        fatal("invalid ID: %s", argv[0]);
    }

    /* Find existing part */
    Part part;
    PartsDBError err = partsdb_find_by_id(db, id, &part);
    if (err != PARTSDB_OK) {
        print_db_error(err, "finding part");
        return 1;
    }

    /* Apply updates */
    for (int i = 1; i < argc; i++) {
        char *eq = strchr(argv[i], '=');
        if (!eq) {
            fatal("invalid field=value: %s", argv[i]);
        }

        *eq = '\0';
        const char *field = argv[i];
        const char *value = eq + 1;

        if (strcmp(field, "name") == 0) {
            strncpy(part.name, value, PARTSDB_MAX_NAME_LEN - 1);
            part.name[PARTSDB_MAX_NAME_LEN - 1] = '\0';
        } else if (strcmp(field, "price") == 0) {
            if (!parse_double(value, &part.unit_price)) {
                fatal("invalid price: %s", value);
            }
        } else if (strcmp(field, "quantity") == 0) {
            if (!parse_uint32(value, &part.quantity)) {
                fatal("invalid quantity: %s", value);
            }
        } else if (strcmp(field, "description") == 0) {
            strncpy(part.description, value, PARTSDB_MAX_DESC_LEN - 1);
            part.description[PARTSDB_MAX_DESC_LEN - 1] = '\0';
        } else if (strcmp(field, "active") == 0) {
            part.is_active = (strcmp(value, "true") == 0 ||
                              strcmp(value, "yes") == 0 ||
                              strcmp(value, "1") == 0);
        } else {
            fatal("unknown field: %s", field);
        }
    }

    /* Update */
    err = partsdb_update(db, &part);
    if (err != PARTSDB_OK) {
        print_db_error(err, "updating part");
        return 1;
    }

    printf("Updated part %u\n", id);
    return 0;
}

static int
cmd_delete(PartsDB *db, const char *id_str)
{
    uint32_t id;
    if (!parse_uint32(id_str, &id)) {
        fatal("invalid ID: %s", id_str);
    }

    PartsDBError err = partsdb_delete(db, id);

    if (err == PARTSDB_ERR_NOT_FOUND) {
        printf("Part %u not found\n", id);
        return 1;
    }

    if (err != PARTSDB_OK) {
        print_db_error(err, "deleting part");
        return 1;
    }

    printf("Deleted part %u\n", id);
    return 0;
}

static int
cmd_low_stock(PartsDB *db, const char *threshold_str)
{
    uint32_t threshold;
    if (!parse_uint32(threshold_str, &threshold)) {
        fatal("invalid threshold: %s", threshold_str);
    }

    Part results[MAX_RESULTS];
    size_t found;

    PartsDBError err = partsdb_find_low_stock(db, threshold, results,
                                               MAX_RESULTS, &found);
    if (err != PARTSDB_OK) {
        print_db_error(err, "finding low stock");
        return 1;
    }

    printf("Parts with quantity < %u: %zu result(s)\n\n", threshold, found);

    if (found > 0) {
        print_header();
        for (size_t i = 0; i < found; i++) {
            print_part_row(&results[i]);
        }
    }

    return 0;
}

static int
cmd_stats(PartsDB *db)
{
    PartsDBStats stats;
    PartsDBError err = partsdb_get_stats(db, &stats);

    if (err != PARTSDB_OK) {
        print_db_error(err, "getting stats");
        return 1;
    }

    printf("Database Statistics:\n");
    printf("  Parts:          %zu\n", stats.part_count);
    printf("  Capacity:       %zu\n", stats.capacity);
    printf("  Memory used:    %zu bytes\n", stats.memory_used);
    printf("  Total inserts:  %llu\n", (unsigned long long)stats.insert_count);
    printf("  Total updates:  %llu\n", (unsigned long long)stats.update_count);
    printf("  Total deletes:  %llu\n", (unsigned long long)stats.delete_count);
    printf("  Total searches: %llu\n", (unsigned long long)stats.search_count);

    return 0;
}

/*============================================================================
 * Help and Usage
 *============================================================================*/

static void
print_usage(void)
{
    printf("Usage: %s [options] <command> [args...]\n", prog_name);
    printf("\n");
    printf("Commands:\n");
    printf("  list                         List all parts\n");
    printf("  add <id> <name> <price>      Add a new part\n");
    printf("       [quantity] [desc]\n");
    printf("  find <id>                    Find part by ID\n");
    printf("  search <pattern>             Search parts by name\n");
    printf("  update <id> field=value ...  Update a part\n");
    printf("  delete <id>                  Delete a part\n");
    printf("  low-stock <threshold>        Find parts with low stock\n");
    printf("  stats                        Show database statistics\n");
    printf("\n");
    printf("Options:\n");
    printf("  -f, --file <path>   Database file (default: %s)\n", DEFAULT_DB_FILE);
    printf("  -n, --new           Create new database (don't load existing)\n");
    printf("  -h, --help          Show this help\n");
    printf("  -v, --version       Show version\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s add 1001 \"Hex Bolt M10x50\" 0.25 500\n", prog_name);
    printf("  %s search bolt\n", prog_name);
    printf("  %s update 1001 quantity=450 price=0.30\n", prog_name);
    printf("  %s low-stock 100\n", prog_name);
}

static void
print_version(void)
{
    printf("%s version %s\n", PROGRAM_NAME, partsdb_version());
}

/*============================================================================
 * Main Entry Point
 *============================================================================*/

int
main(int argc, char *argv[])
{
    /* Save program name (Part 13: K&R style) */
    if (argc > 0 && argv[0]) {
        const char *p = strrchr(argv[0], '/');
        if (!p) p = strrchr(argv[0], '\\');
        prog_name = p ? p + 1 : argv[0];
    }

    /* Parse options */
    const char *db_file = DEFAULT_DB_FILE;
    bool create_new = false;
    int arg_index = 1;

    while (arg_index < argc && argv[arg_index][0] == '-') {
        const char *opt = argv[arg_index];

        if (strcmp(opt, "-h") == 0 || strcmp(opt, "--help") == 0) {
            print_usage();
            return 0;
        }
        if (strcmp(opt, "-v") == 0 || strcmp(opt, "--version") == 0) {
            print_version();
            return 0;
        }
        if (strcmp(opt, "-n") == 0 || strcmp(opt, "--new") == 0) {
            create_new = true;
            arg_index++;
            continue;
        }
        if (strcmp(opt, "-f") == 0 || strcmp(opt, "--file") == 0) {
            if (arg_index + 1 >= argc) {
                fatal("option %s requires an argument", opt);
            }
            db_file = argv[++arg_index];
            arg_index++;
            continue;
        }
        if (opt[0] == '-' && opt[1] == 'f' && opt[2] != '\0') {
            db_file = opt + 2;
            arg_index++;
            continue;
        }

        fatal("unknown option: %s (use --help for usage)", opt);
    }

    /* Need at least a command */
    if (arg_index >= argc) {
        print_usage();
        return 1;
    }

    const char *command = argv[arg_index++];

    /* Open or create database */
    PartsDB *db = NULL;
    PartsDBError err;

    if (!create_new) {
        /* Try to open existing */
        err = partsdb_open(&db, db_file);
        if (err == PARTSDB_ERR_IO) {
            /* File doesn't exist - create new */
            err = partsdb_create(&db, NULL);
        }
    } else {
        err = partsdb_create(&db, NULL);
    }

    if (err != PARTSDB_OK) {
        print_db_error(err, "opening database");
        return 1;
    }

    /* Dispatch command */
    int result = 0;
    int cmd_argc = argc - arg_index;
    char **cmd_argv = argv + arg_index;

    if (strcmp(command, "list") == 0) {
        result = cmd_list(db);
    }
    else if (strcmp(command, "add") == 0) {
        result = cmd_add(db, cmd_argc, cmd_argv);
    }
    else if (strcmp(command, "find") == 0) {
        if (cmd_argc < 1) {
            fatal("find requires an ID argument");
        }
        result = cmd_find(db, cmd_argv[0]);
    }
    else if (strcmp(command, "search") == 0) {
        if (cmd_argc < 1) {
            fatal("search requires a pattern argument");
        }
        result = cmd_search(db, cmd_argv[0]);
    }
    else if (strcmp(command, "update") == 0) {
        result = cmd_update(db, cmd_argc, cmd_argv);
    }
    else if (strcmp(command, "delete") == 0) {
        if (cmd_argc < 1) {
            fatal("delete requires an ID argument");
        }
        result = cmd_delete(db, cmd_argv[0]);
    }
    else if (strcmp(command, "low-stock") == 0) {
        if (cmd_argc < 1) {
            fatal("low-stock requires a threshold argument");
        }
        result = cmd_low_stock(db, cmd_argv[0]);
    }
    else if (strcmp(command, "stats") == 0) {
        result = cmd_stats(db);
    }
    else {
        fatal("unknown command: %s (use --help for usage)", command);
    }

    /* Save database if modified (add, update, delete) */
    if (result == 0 &&
        (strcmp(command, "add") == 0 ||
         strcmp(command, "update") == 0 ||
         strcmp(command, "delete") == 0)) {
        err = partsdb_save(db, db_file);
        if (err != PARTSDB_OK) {
            print_db_error(err, "saving database");
            result = 1;
        }
    }

    /* Cleanup */
    partsdb_close(db);

    return result;
}
