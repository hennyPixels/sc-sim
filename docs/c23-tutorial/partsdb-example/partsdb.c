/**
 * @file partsdb.c
 * @brief PartsDB Implementation
 *
 * This implementation demonstrates concepts from the C23 Tutorial:
 *
 * - Part 4:  Input validation, bounds checking, safe string handling
 * - Part 5:  Avoiding common traps (null checks, overflow)
 * - Part 6:  Opaque types, resource management, error handling
 * - Part 9:  Consistent style, debugging support
 * - Part 10: Defensive programming, assertions, invariant checking
 * - Part 11: Safe memory management, no leaks
 * - Part 12: CERT C compliance
 * - Part 13: K&R-style simplicity
 */

#include "partsdb_internal.h"
#include <ctype.h>
#include <errno.h>

/*============================================================================
 * Version Information
 *============================================================================*/

static const char VERSION_STRING[] = "1.0.0";

const char *
partsdb_version(void)
{
    return VERSION_STRING;
}

/*============================================================================
 * Error Handling (Part 9: Clear error messages)
 *============================================================================*/

static const char *error_messages[] = {
    [PARTSDB_OK]             = "Success",
    [PARTSDB_ERR_NULL_ARG]   = "Null argument provided",
    [PARTSDB_ERR_INVALID_ID] = "Invalid part ID",
    [PARTSDB_ERR_INVALID_NAME] = "Invalid or empty part name",
    [PARTSDB_ERR_INVALID_PRICE] = "Invalid price (negative or NaN)",
    [PARTSDB_ERR_DUPLICATE]  = "Part with this ID already exists",
    [PARTSDB_ERR_NOT_FOUND]  = "Part not found",
    [PARTSDB_ERR_FULL]       = "Database is full",
    [PARTSDB_ERR_OUT_OF_MEMORY] = "Memory allocation failed",
    [PARTSDB_ERR_IO]         = "File I/O error",
    [PARTSDB_ERR_CORRUPT]    = "Data corruption detected",
    [PARTSDB_ERR_NOT_OPEN]   = "Database not open",
    [PARTSDB_ERR_ALREADY_OPEN] = "Database already open",
    [PARTSDB_ERR_INTERNAL]   = "Internal error"
};

const char *
partsdb_error_string(PartsDBError err)
{
    if (err < 0 || err > PARTSDB_ERR_INTERNAL) {
        return "Unknown error";
    }
    return error_messages[err];
}

/*============================================================================
 * Debug Support (Part 10: Assertions and logging)
 *============================================================================*/

void
partsdb_assert_fail(const char *cond, const char *file,
                    int line, const char *func)
{
    fprintf(stderr,
            "\n*** PARTSDB ASSERTION FAILED ***\n"
            "Condition: %s\n"
            "Location:  %s:%d in %s()\n\n",
            cond, file, line, func);
    abort();
}

void
partsdb_verify_fail(const char *cond, const char *file, int line)
{
    fprintf(stderr, "[PARTSDB] Verification failed: %s (%s:%d)\n",
            cond, file, line);
}

void
partsdb_debug_log(const char *file, int line, const char *func,
                  const char *fmt, ...)
{
    fprintf(stderr, "[PARTSDB DEBUG] %s:%d %s(): ", file, line, func);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/*============================================================================
 * Memory Management (Part 11: Safe allocation)
 *============================================================================*/

void *
partsdb_alloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    void *ptr = malloc(size);
    if (ptr) {
        /* Fill with debug pattern in debug builds */
        #ifndef NDEBUG
        memset(ptr, 0xCD, size);  /* Uninitialized pattern */
        #endif
    }
    return ptr;
}

void *
partsdb_calloc(size_t count, size_t size)
{
    /* calloc handles overflow checking internally */
    return calloc(count, size);
}

void *
partsdb_realloc(void *ptr, size_t size)
{
    if (size == 0) {
        partsdb_free(ptr);
        return NULL;
    }
    return realloc(ptr, size);
}

void
partsdb_free(void *ptr)
{
    if (ptr) {
        /* In debug builds, we could poison memory here */
        free(ptr);
    }
}

/*============================================================================
 * String Utilities (Part 4: Safe string handling)
 *============================================================================*/

/* Safe string copy - like strlcpy */
size_t
partsdb_strlcpy(char *dst, const char *src, size_t size)
{
    PARTSDB_ASSERT(dst != NULL);
    PARTSDB_ASSERT(src != NULL);

    size_t src_len = strlen(src);

    if (size > 0) {
        size_t copy_len = partsdb_min(src_len, size - 1);
        memcpy(dst, src, copy_len);
        dst[copy_len] = '\0';
    }

    return src_len;
}

/* Case-insensitive string comparison */
int
partsdb_strcasecmp(const char *s1, const char *s2)
{
    PARTSDB_ASSERT(s1 != NULL);
    PARTSDB_ASSERT(s2 != NULL);

    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) {
            return c1 - c2;
        }
        s1++;
        s2++;
    }

    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/* Case-insensitive substring search */
bool
partsdb_strcasestr(const char *haystack, const char *needle)
{
    PARTSDB_ASSERT(haystack != NULL);
    PARTSDB_ASSERT(needle != NULL);

    if (*needle == '\0') {
        return true;
    }

    size_t needle_len = strlen(needle);
    size_t haystack_len = strlen(haystack);

    if (needle_len > haystack_len) {
        return false;
    }

    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        bool match = true;
        for (size_t j = 0; j < needle_len; j++) {
            if (tolower((unsigned char)haystack[i + j]) !=
                tolower((unsigned char)needle[j])) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }

    return false;
}

/*============================================================================
 * Integrity Checking (Part 10: Data validation)
 *============================================================================*/

/* Simple CRC32 implementation */
static const uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t
partsdb_crc32(const void *data, size_t size)
{
    const uint8_t *bytes = data;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < size; i++) {
        crc = crc32_table[(crc ^ bytes[i]) & 0xFF] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFF;
}

uint32_t
partsdb_compute_part_checksum(const Part *part)
{
    return partsdb_crc32(part, sizeof(*part));
}

bool
partsdb_verify_part_checksum(const InternalPart *ip)
{
    uint32_t computed = partsdb_compute_part_checksum(&ip->data);
    return computed == ip->checksum;
}

/*============================================================================
 * Handle Validation (Part 10: Invariant checking)
 *============================================================================*/

bool
partsdb_is_valid_handle(const PartsDB *db)
{
    if (!db) {
        return false;
    }

    /* Check magic number */
    if (db->magic != PARTSDB_MAGIC) {
        PARTSDB_DEBUG_LOG("Invalid magic: 0x%08X (expected 0x%08X)",
                          db->magic, PARTSDB_MAGIC);
        return false;
    }

    /* Check invariants */
    if (db->count > db->capacity) {
        PARTSDB_DEBUG_LOG("Count > capacity: %zu > %zu",
                          db->count, db->capacity);
        return false;
    }

    if (db->capacity > 0 && !db->parts) {
        PARTSDB_DEBUG_LOG("Capacity > 0 but parts is NULL");
        return false;
    }

    return true;
}

bool
partsdb_is_valid_part(const Part *part)
{
    if (!part) {
        return false;
    }

    if (part->id == 0) {
        return false;
    }

    if (part->name[0] == '\0') {
        return false;
    }

    if (!partsdb_is_valid_price(part->unit_price)) {
        return false;
    }

    return true;
}

/*============================================================================
 * Array Management
 *============================================================================*/

PartsDBError
partsdb_ensure_capacity(PartsDB *db, size_t required)
{
    PARTSDB_ASSERT(partsdb_is_valid_handle(db));

    if (required <= db->capacity) {
        return PARTSDB_OK;
    }

    /* Check maximum */
    if (required > PARTSDB_MAX_CAPACITY) {
        return PARTSDB_ERR_FULL;
    }

    /* Calculate new capacity (double, but at least required) */
    size_t new_capacity = db->capacity ? db->capacity : PARTSDB_DEFAULT_CAPACITY;
    while (new_capacity < required && new_capacity <= PARTSDB_MAX_CAPACITY / 2) {
        new_capacity *= PARTSDB_GROWTH_FACTOR;
    }
    if (new_capacity < required) {
        new_capacity = required;
    }

    /* Overflow check */
    size_t new_size;
    if (partsdb_mul_overflow(new_capacity, sizeof(InternalPart), &new_size)) {
        return PARTSDB_ERR_FULL;
    }

    /* Reallocate */
    InternalPart *new_parts = partsdb_realloc(db->parts, new_size);
    if (!new_parts) {
        return PARTSDB_ERR_OUT_OF_MEMORY;
    }

    /* Initialize new slots */
    for (size_t i = db->capacity; i < new_capacity; i++) {
        new_parts[i].occupied = false;
    }

    db->parts = new_parts;
    db->capacity = new_capacity;

    PARTSDB_DEBUG_LOG("Resized to capacity %zu", new_capacity);

    return PARTSDB_OK;
}

/* Find slot containing part with given ID, or SIZE_MAX if not found */
size_t
partsdb_find_slot(const PartsDB *db, uint32_t id)
{
    PARTSDB_ASSERT(partsdb_is_valid_handle(db));

    for (size_t i = 0; i < db->capacity; i++) {
        if (db->parts[i].occupied && db->parts[i].data.id == id) {
            return i;
        }
    }

    return SIZE_MAX;
}

/* Find first empty slot, or SIZE_MAX if none */
size_t
partsdb_find_empty_slot(const PartsDB *db)
{
    PARTSDB_ASSERT(partsdb_is_valid_handle(db));

    for (size_t i = 0; i < db->capacity; i++) {
        if (!db->parts[i].occupied) {
            return i;
        }
    }

    return SIZE_MAX;
}

/*============================================================================
 * Public API: Lifecycle
 *============================================================================*/

PartsDBError
partsdb_create(PartsDB **db, const PartsDBConfig *config)
{
    /* Validate arguments (Part 12: CERT - NULL checks) */
    if (!db) {
        return PARTSDB_ERR_NULL_ARG;
    }
    *db = NULL;

    /* Use defaults if no config provided */
    PartsDBConfig cfg = PARTSDB_CONFIG_DEFAULT;
    if (config) {
        cfg = *config;
    }

    /* Allocate handle */
    PartsDB *new_db = partsdb_calloc(1, sizeof(PartsDB));
    if (!new_db) {
        return PARTSDB_ERR_OUT_OF_MEMORY;
    }

    /* Initialize */
    new_db->magic = PARTSDB_MAGIC;
    new_db->checksums_enabled = cfg.enable_checksums;
    new_db->logging_enabled = cfg.enable_logging;

    /* Allocate initial storage */
    size_t initial = cfg.initial_capacity > 0 ?
                     cfg.initial_capacity : PARTSDB_DEFAULT_CAPACITY;

    PartsDBError err = partsdb_ensure_capacity(new_db, initial);
    if (err != PARTSDB_OK) {
        partsdb_free(new_db);
        return err;
    }

    PARTSDB_ASSERT(partsdb_is_valid_handle(new_db));

    *db = new_db;
    return PARTSDB_OK;
}

void
partsdb_close(PartsDB *db)
{
    if (!db) {
        return;  /* Safe to call with NULL */
    }

    if (!PARTSDB_VERIFY(db->magic == PARTSDB_MAGIC)) {
        return;  /* Invalid handle */
    }

    /* Invalidate magic to catch use-after-free */
    db->magic = 0;

    /* Free storage */
    partsdb_free(db->parts);
    db->parts = NULL;
    db->count = 0;
    db->capacity = 0;

    /* Free handle */
    partsdb_free(db);
}

/*============================================================================
 * Public API: CRUD Operations
 *============================================================================*/

PartsDBError
partsdb_insert(PartsDB *db, const Part *part)
{
    /* Validate arguments */
    if (!db) {
        return PARTSDB_ERR_NULL_ARG;
    }
    if (!partsdb_is_valid_handle(db)) {
        return PARTSDB_ERR_INTERNAL;
    }
    if (!part) {
        return PARTSDB_ERR_NULL_ARG;
    }

    /* Validate part data */
    if (part->id == 0) {
        return PARTSDB_ERR_INVALID_ID;
    }
    if (part->name[0] == '\0') {
        return PARTSDB_ERR_INVALID_NAME;
    }
    if (!partsdb_is_valid_price(part->unit_price)) {
        return PARTSDB_ERR_INVALID_PRICE;
    }

    /* Check for duplicate */
    if (partsdb_find_slot(db, part->id) != SIZE_MAX) {
        return PARTSDB_ERR_DUPLICATE;
    }

    /* Ensure capacity */
    PartsDBError err = partsdb_ensure_capacity(db, db->count + 1);
    if (err != PARTSDB_OK) {
        return err;
    }

    /* Find empty slot */
    size_t slot = partsdb_find_empty_slot(db);
    PARTSDB_ASSERT(slot != SIZE_MAX);

    /* Copy data */
    InternalPart *ip = &db->parts[slot];
    ip->data = *part;
    ip->occupied = true;

    /* Ensure null termination (defensive) */
    ip->data.name[PARTSDB_MAX_NAME_LEN - 1] = '\0';
    ip->data.description[PARTSDB_MAX_DESC_LEN - 1] = '\0';

    /* Compute checksum */
    if (db->checksums_enabled) {
        ip->checksum = partsdb_compute_part_checksum(&ip->data);
    }

    db->count++;
    db->insert_count++;

    PARTSDB_DEBUG_LOG("Inserted part id=%u name='%s' at slot %zu",
                      part->id, part->name, slot);

    return PARTSDB_OK;
}

PartsDBError
partsdb_find_by_id(const PartsDB *db, uint32_t id, Part *result)
{
    /* Validate arguments */
    if (!db || !result) {
        return PARTSDB_ERR_NULL_ARG;
    }
    if (!partsdb_is_valid_handle(db)) {
        return PARTSDB_ERR_INTERNAL;
    }
    if (id == 0) {
        return PARTSDB_ERR_INVALID_ID;
    }

    /* Find slot */
    size_t slot = partsdb_find_slot(db, id);
    if (slot == SIZE_MAX) {
        return PARTSDB_ERR_NOT_FOUND;
    }

    InternalPart *ip = &db->parts[slot];

    /* Verify checksum if enabled */
    if (db->checksums_enabled && !partsdb_verify_part_checksum(ip)) {
        return PARTSDB_ERR_CORRUPT;
    }

    /* Copy to result */
    *result = ip->data;

    /* Cast away const for statistics update */
    ((PartsDB *)db)->search_count++;

    return PARTSDB_OK;
}

PartsDBError
partsdb_update(PartsDB *db, const Part *part)
{
    /* Validate arguments */
    if (!db || !part) {
        return PARTSDB_ERR_NULL_ARG;
    }
    if (!partsdb_is_valid_handle(db)) {
        return PARTSDB_ERR_INTERNAL;
    }
    if (part->id == 0) {
        return PARTSDB_ERR_INVALID_ID;
    }
    if (part->name[0] == '\0') {
        return PARTSDB_ERR_INVALID_NAME;
    }
    if (!partsdb_is_valid_price(part->unit_price)) {
        return PARTSDB_ERR_INVALID_PRICE;
    }

    /* Find existing */
    size_t slot = partsdb_find_slot(db, part->id);
    if (slot == SIZE_MAX) {
        return PARTSDB_ERR_NOT_FOUND;
    }

    /* Update data */
    InternalPart *ip = &db->parts[slot];
    ip->data = *part;

    /* Ensure null termination */
    ip->data.name[PARTSDB_MAX_NAME_LEN - 1] = '\0';
    ip->data.description[PARTSDB_MAX_DESC_LEN - 1] = '\0';

    /* Update checksum */
    if (db->checksums_enabled) {
        ip->checksum = partsdb_compute_part_checksum(&ip->data);
    }

    db->update_count++;

    PARTSDB_DEBUG_LOG("Updated part id=%u", part->id);

    return PARTSDB_OK;
}

PartsDBError
partsdb_delete(PartsDB *db, uint32_t id)
{
    /* Validate arguments */
    if (!db) {
        return PARTSDB_ERR_NULL_ARG;
    }
    if (!partsdb_is_valid_handle(db)) {
        return PARTSDB_ERR_INTERNAL;
    }
    if (id == 0) {
        return PARTSDB_ERR_INVALID_ID;
    }

    /* Find slot */
    size_t slot = partsdb_find_slot(db, id);
    if (slot == SIZE_MAX) {
        return PARTSDB_ERR_NOT_FOUND;
    }

    /* Clear slot */
    InternalPart *ip = &db->parts[slot];
    memset(ip, 0, sizeof(*ip));
    ip->occupied = false;

    db->count--;
    db->delete_count++;

    PARTSDB_DEBUG_LOG("Deleted part id=%u from slot %zu", id, slot);

    return PARTSDB_OK;
}

/*============================================================================
 * Public API: Query Operations
 *============================================================================*/

size_t
partsdb_count(const PartsDB *db)
{
    if (!db || !partsdb_is_valid_handle(db)) {
        return 0;
    }
    return db->count;
}

PartsDBError
partsdb_search_by_name(const PartsDB *db, const char *pattern,
                       Part *results, size_t max_results,
                       size_t *found_count)
{
    /* Validate arguments */
    if (!db || !pattern || !results || !found_count) {
        return PARTSDB_ERR_NULL_ARG;
    }
    if (!partsdb_is_valid_handle(db)) {
        return PARTSDB_ERR_INTERNAL;
    }

    *found_count = 0;

    /* Empty pattern matches nothing (or everything?) - match nothing */
    if (pattern[0] == '\0') {
        return PARTSDB_OK;
    }

    /* Search through all parts */
    for (size_t i = 0; i < db->capacity && *found_count < max_results; i++) {
        if (!db->parts[i].occupied) {
            continue;
        }

        const Part *p = &db->parts[i].data;

        /* Case-insensitive substring search */
        if (partsdb_strcasestr(p->name, pattern)) {
            results[*found_count] = *p;
            (*found_count)++;
        }
    }

    /* Cast away const for statistics */
    ((PartsDB *)db)->search_count++;

    return PARTSDB_OK;
}

PartsDBError
partsdb_find_low_stock(const PartsDB *db, uint32_t threshold,
                       Part *results, size_t max_results,
                       size_t *found_count)
{
    /* Validate arguments */
    if (!db || !results || !found_count) {
        return PARTSDB_ERR_NULL_ARG;
    }
    if (!partsdb_is_valid_handle(db)) {
        return PARTSDB_ERR_INTERNAL;
    }

    *found_count = 0;

    for (size_t i = 0; i < db->capacity && *found_count < max_results; i++) {
        if (!db->parts[i].occupied) {
            continue;
        }

        const Part *p = &db->parts[i].data;

        if (p->is_active && p->quantity < threshold) {
            results[*found_count] = *p;
            (*found_count)++;
        }
    }

    ((PartsDB *)db)->search_count++;

    return PARTSDB_OK;
}

/*============================================================================
 * Public API: Iterator
 *============================================================================*/

PartsDBError
partsdb_iter_create(const PartsDB *db, PartsDBIterator **iter)
{
    if (!db || !iter) {
        return PARTSDB_ERR_NULL_ARG;
    }
    if (!partsdb_is_valid_handle(db)) {
        return PARTSDB_ERR_INTERNAL;
    }

    *iter = NULL;

    PartsDBIterator *new_iter = partsdb_alloc(sizeof(PartsDBIterator));
    if (!new_iter) {
        return PARTSDB_ERR_OUT_OF_MEMORY;
    }

    new_iter->db = db;
    new_iter->current_index = 0;
    new_iter->snapshot_count = (uint32_t)db->count;

    *iter = new_iter;
    return PARTSDB_OK;
}

bool
partsdb_iter_next(PartsDBIterator *iter, Part *part)
{
    if (!iter || !part || !iter->db) {
        return false;
    }

    const PartsDB *db = iter->db;

    /* Find next occupied slot */
    while (iter->current_index < db->capacity) {
        size_t i = iter->current_index++;

        if (db->parts[i].occupied) {
            *part = db->parts[i].data;
            return true;
        }
    }

    return false;  /* No more parts */
}

void
partsdb_iter_reset(PartsDBIterator *iter)
{
    if (iter) {
        iter->current_index = 0;
    }
}

void
partsdb_iter_destroy(PartsDBIterator *iter)
{
    partsdb_free(iter);
}

/*============================================================================
 * Public API: Utilities
 *============================================================================*/

bool
partsdb_validate_part(const Part *part, char *errmsg, size_t errlen)
{
    if (!part) {
        if (errmsg && errlen > 0) {
            partsdb_strlcpy(errmsg, "Part is NULL", errlen);
        }
        return false;
    }

    if (part->id == 0) {
        if (errmsg && errlen > 0) {
            partsdb_strlcpy(errmsg, "Part ID cannot be zero", errlen);
        }
        return false;
    }

    if (part->name[0] == '\0') {
        if (errmsg && errlen > 0) {
            partsdb_strlcpy(errmsg, "Part name cannot be empty", errlen);
        }
        return false;
    }

    /* Check for embedded nulls or control characters */
    for (size_t i = 0; i < PARTSDB_MAX_NAME_LEN && part->name[i]; i++) {
        unsigned char c = (unsigned char)part->name[i];
        if (c < 0x20 && c != '\0') {
            if (errmsg && errlen > 0) {
                partsdb_strlcpy(errmsg, "Part name contains control characters", errlen);
            }
            return false;
        }
    }

    if (!partsdb_is_valid_price(part->unit_price)) {
        if (errmsg && errlen > 0) {
            partsdb_strlcpy(errmsg, "Price must be non-negative and finite", errlen);
        }
        return false;
    }

    return true;
}

void
partsdb_part_init(Part *part)
{
    if (part) {
        memset(part, 0, sizeof(*part));
        part->is_active = true;
    }
}

PartsDBError
partsdb_get_stats(const PartsDB *db, PartsDBStats *stats)
{
    if (!db || !stats) {
        return PARTSDB_ERR_NULL_ARG;
    }
    if (!partsdb_is_valid_handle(db)) {
        return PARTSDB_ERR_INTERNAL;
    }

    stats->part_count = db->count;
    stats->capacity = db->capacity;
    stats->memory_used = sizeof(PartsDB) +
                         (db->capacity * sizeof(InternalPart));
    stats->insert_count = db->insert_count;
    stats->update_count = db->update_count;
    stats->delete_count = db->delete_count;
    stats->search_count = db->search_count;

    return PARTSDB_OK;
}

/*============================================================================
 * File I/O (Part 12: CERT FIO rules)
 *============================================================================*/

/* File header structure */
typedef struct FileHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t part_count;
    uint32_t checksum;
} FileHeader;

PartsDBError
partsdb_save(const PartsDB *db, const char *path)
{
    if (!db || !path) {
        return PARTSDB_ERR_NULL_ARG;
    }
    if (!partsdb_is_valid_handle(db)) {
        return PARTSDB_ERR_INTERNAL;
    }

    FILE *fp = fopen(path, "wb");
    if (!fp) {
        return PARTSDB_ERR_IO;
    }

    /* Write header */
    FileHeader header = {
        .magic = PARTSDB_MAGIC,
        .version = PARTSDB_FILE_VERSION,
        .part_count = (uint32_t)db->count,
        .checksum = 0  /* Will compute after writing parts */
    };

    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        fclose(fp);
        return PARTSDB_ERR_IO;
    }

    /* Write parts */
    uint32_t running_checksum = 0;
    for (size_t i = 0; i < db->capacity; i++) {
        if (!db->parts[i].occupied) {
            continue;
        }

        if (fwrite(&db->parts[i].data, sizeof(Part), 1, fp) != 1) {
            fclose(fp);
            return PARTSDB_ERR_IO;
        }

        running_checksum ^= partsdb_crc32(&db->parts[i].data, sizeof(Part));
    }

    /* Update header with checksum */
    header.checksum = running_checksum;
    fseek(fp, 0, SEEK_SET);
    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        fclose(fp);
        return PARTSDB_ERR_IO;
    }

    fclose(fp);
    return PARTSDB_OK;
}

PartsDBError
partsdb_open(PartsDB **db, const char *path)
{
    if (!db || !path) {
        return PARTSDB_ERR_NULL_ARG;
    }
    *db = NULL;

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return PARTSDB_ERR_IO;
    }

    /* Read header */
    FileHeader header;
    if (fread(&header, sizeof(header), 1, fp) != 1) {
        fclose(fp);
        return PARTSDB_ERR_IO;
    }

    /* Validate header */
    if (header.magic != PARTSDB_MAGIC) {
        fclose(fp);
        return PARTSDB_ERR_CORRUPT;
    }

    if (header.version != PARTSDB_FILE_VERSION) {
        fclose(fp);
        return PARTSDB_ERR_CORRUPT;  /* Could handle version migration */
    }

    /* Create database */
    PartsDBConfig config = {
        .initial_capacity = header.part_count,
        .enable_checksums = true,
        .enable_logging = false
    };

    PartsDBError err = partsdb_create(db, &config);
    if (err != PARTSDB_OK) {
        fclose(fp);
        return err;
    }

    /* Read parts */
    uint32_t running_checksum = 0;
    for (uint32_t i = 0; i < header.part_count; i++) {
        Part part;
        if (fread(&part, sizeof(part), 1, fp) != 1) {
            partsdb_close(*db);
            *db = NULL;
            fclose(fp);
            return PARTSDB_ERR_IO;
        }

        running_checksum ^= partsdb_crc32(&part, sizeof(part));

        err = partsdb_insert(*db, &part);
        if (err != PARTSDB_OK) {
            partsdb_close(*db);
            *db = NULL;
            fclose(fp);
            return err;
        }
    }

    /* Verify checksum */
    if (running_checksum != header.checksum) {
        partsdb_close(*db);
        *db = NULL;
        fclose(fp);
        return PARTSDB_ERR_CORRUPT;
    }

    fclose(fp);
    return PARTSDB_OK;
}
