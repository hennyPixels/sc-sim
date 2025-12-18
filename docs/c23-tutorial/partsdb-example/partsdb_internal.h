/**
 * @file partsdb_internal.h
 * @brief PartsDB Internal Implementation Details
 *
 * This header is NOT part of the public API. It contains:
 * - Internal structure definitions (hidden from users)
 * - Debug macros and assertions (Part 10: Writing Solid Code)
 * - Memory debugging support
 *
 * @warning Do not include this header in application code.
 */

#ifndef PARTSDB_INTERNAL_H
#define PARTSDB_INTERNAL_H

#include "partsdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>

/*============================================================================
 * Configuration
 *============================================================================*/

/** Default initial capacity if not specified */
#define PARTSDB_DEFAULT_CAPACITY 64

/** Growth factor when resizing */
#define PARTSDB_GROWTH_FACTOR 2

/** Maximum supported capacity */
#define PARTSDB_MAX_CAPACITY (SIZE_MAX / sizeof(Part) / 2)

/** Magic number for integrity checking */
#define PARTSDB_MAGIC 0x50415254U  /* "PART" in ASCII */

/** File format version */
#define PARTSDB_FILE_VERSION 1

/*============================================================================
 * Debug and Assertion Macros (Part 10: Defensive Programming)
 *============================================================================*/

#ifdef NDEBUG
    /* Release build: minimal checks */
    #define PARTSDB_ASSERT(cond) ((void)0)
    #define PARTSDB_DEBUG_LOG(fmt, ...) ((void)0)
#else
    /* Debug build: full assertions and logging */
    #define PARTSDB_ASSERT(cond) \
        do { \
            if (!(cond)) { \
                partsdb_assert_fail(#cond, __FILE__, __LINE__, __func__); \
            } \
        } while (0)

    #define PARTSDB_DEBUG_LOG(fmt, ...) \
        partsdb_debug_log(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#endif

/** Runtime verification that stays in release builds */
#define PARTSDB_VERIFY(cond) \
    ((cond) ? true : (partsdb_verify_fail(#cond, __FILE__, __LINE__), false))

/*============================================================================
 * Internal Structure Definitions
 *============================================================================*/

/**
 * @brief Internal part storage with metadata
 *
 * Includes checksum for integrity verification.
 */
typedef struct InternalPart {
    Part     data;          /**< Public part data */
    uint32_t checksum;      /**< CRC32 checksum of data */
    bool     occupied;      /**< Slot is in use */
} InternalPart;

/**
 * @brief Main database structure (opaque to users)
 *
 * Contains all state needed for database operations.
 */
struct PartsDB {
    uint32_t       magic;            /**< Magic number for validation */
    InternalPart  *parts;            /**< Array of part slots */
    size_t         count;            /**< Number of parts stored */
    size_t         capacity;         /**< Current array capacity */
    bool           checksums_enabled;/**< Enable integrity checks */
    bool           logging_enabled;  /**< Enable operation logging */

    /* Statistics */
    uint64_t       insert_count;
    uint64_t       update_count;
    uint64_t       delete_count;
    uint64_t       search_count;

    /* For integrity checking */
    uint32_t       header_checksum;  /**< Checksum of header fields */
};

/**
 * @brief Iterator structure
 */
struct PartsDBIterator {
    const PartsDB *db;               /**< Database being iterated */
    size_t         current_index;    /**< Current position in array */
    uint32_t       snapshot_count;   /**< Count at creation (for change detection) */
};

/*============================================================================
 * Internal Function Prototypes
 *============================================================================*/

/* Assertion handling */
void partsdb_assert_fail(const char *cond, const char *file,
                         int line, const char *func);
void partsdb_verify_fail(const char *cond, const char *file, int line);
void partsdb_debug_log(const char *file, int line, const char *func,
                       const char *fmt, ...);

/* Memory management (Part 11: Safe allocation) */
void *partsdb_alloc(size_t size);
void *partsdb_calloc(size_t count, size_t size);
void *partsdb_realloc(void *ptr, size_t size);
void  partsdb_free(void *ptr);

/* Integrity checking */
uint32_t partsdb_crc32(const void *data, size_t size);
uint32_t partsdb_compute_part_checksum(const Part *part);
bool     partsdb_verify_part_checksum(const InternalPart *ip);
uint32_t partsdb_compute_header_checksum(const PartsDB *db);
bool     partsdb_verify_header(const PartsDB *db);

/* Internal validation */
bool partsdb_is_valid_handle(const PartsDB *db);
bool partsdb_is_valid_part(const Part *part);

/* Array operations */
PartsDBError partsdb_ensure_capacity(PartsDB *db, size_t required);
size_t       partsdb_find_slot(const PartsDB *db, uint32_t id);
size_t       partsdb_find_empty_slot(const PartsDB *db);

/* String utilities (Part 4: Safe string handling) */
size_t partsdb_strlcpy(char *dst, const char *src, size_t size);
int    partsdb_strcasecmp(const char *s1, const char *s2);
bool   partsdb_strcasestr(const char *haystack, const char *needle);

/*============================================================================
 * Inline Utilities
 *============================================================================*/

/**
 * @brief Check if a floating-point value is valid (not NaN or infinity)
 */
static inline bool
partsdb_is_valid_price(double price)
{
    return price >= 0.0 && isfinite(price);
}

/**
 * @brief Safe minimum
 */
static inline size_t
partsdb_min(size_t a, size_t b)
{
    return (a < b) ? a : b;
}

/**
 * @brief Safe maximum
 */
static inline size_t
partsdb_max(size_t a, size_t b)
{
    return (a > b) ? a : b;
}

/**
 * @brief Check for multiplication overflow
 */
static inline bool
partsdb_mul_overflow(size_t a, size_t b, size_t *result)
{
    if (a > 0 && b > SIZE_MAX / a) {
        return true;  /* Would overflow */
    }
    *result = a * b;
    return false;
}

/*============================================================================
 * Debug Memory Tracking (Part 10: Memory debugging)
 *============================================================================*/

#ifdef PARTSDB_DEBUG_MEMORY

typedef struct MemoryBlock {
    void              *ptr;
    size_t             size;
    const char        *file;
    int                line;
    struct MemoryBlock *next;
} MemoryBlock;

void  partsdb_memory_track(void *ptr, size_t size, const char *file, int line);
void  partsdb_memory_untrack(void *ptr);
void  partsdb_memory_report(void);
size_t partsdb_memory_allocated(void);

#define PARTSDB_ALLOC(size) \
    partsdb_alloc_tracked(size, __FILE__, __LINE__)
#define PARTSDB_FREE(ptr) \
    partsdb_free_tracked(ptr, __FILE__, __LINE__)

#else

#define PARTSDB_ALLOC(size) partsdb_alloc(size)
#define PARTSDB_FREE(ptr)   partsdb_free(ptr)

#endif /* PARTSDB_DEBUG_MEMORY */

#endif /* PARTSDB_INTERNAL_H */
