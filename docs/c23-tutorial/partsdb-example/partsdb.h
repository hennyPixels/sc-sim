/**
 * @file partsdb.h
 * @brief PartsDB - A Secure Parts Database Library
 *
 * This header demonstrates concepts from the C23 Tutorial:
 * - Opaque types for information hiding (Part 6: C Interfaces)
 * - const correctness (Part 4: Effective C)
 * - Clear error handling (Part 9: Practice of Programming)
 * - Documentation style (Part 13: K&R)
 *
 * @note Thread-safety: Individual operations are thread-safe when using
 *       separate database handles. Concurrent access to the same handle
 *       requires external synchronization.
 */

#ifndef PARTSDB_H
#define PARTSDB_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Version Information
 *============================================================================*/

#define PARTSDB_VERSION_MAJOR 1
#define PARTSDB_VERSION_MINOR 0
#define PARTSDB_VERSION_PATCH 0

/**
 * @brief Get version string
 * @return Static string containing version (e.g., "1.0.0")
 */
const char *partsdb_version(void);

/*============================================================================
 * Opaque Types (Part 6: Information Hiding)
 *============================================================================*/

/**
 * @brief Opaque database handle
 *
 * Users cannot access internal structure - they only work with pointers.
 * This allows implementation changes without breaking API compatibility.
 */
typedef struct PartsDB PartsDB;

/**
 * @brief Opaque iterator for traversing parts
 */
typedef struct PartsDBIterator PartsDBIterator;

/*============================================================================
 * Public Data Structures
 *============================================================================*/

/**
 * @brief Maximum length for part name (including null terminator)
 */
#define PARTSDB_MAX_NAME_LEN 64

/**
 * @brief Maximum length for part description (including null terminator)
 */
#define PARTSDB_MAX_DESC_LEN 256

/**
 * @brief Part record - the public view of a part
 *
 * This structure is used for input/output with the API.
 * Internal storage may differ.
 */
typedef struct Part {
    uint32_t id;                          /**< Unique part identifier (0 = invalid) */
    char     name[PARTSDB_MAX_NAME_LEN];  /**< Part name (null-terminated) */
    char     description[PARTSDB_MAX_DESC_LEN]; /**< Description (null-terminated) */
    uint32_t quantity;                    /**< Quantity in stock */
    double   unit_price;                  /**< Price per unit (must be >= 0) */
    bool     is_active;                   /**< Whether part is active/available */
} Part;

/*============================================================================
 * Error Handling (Part 9: Clear Error Contracts)
 *============================================================================*/

/**
 * @brief Error codes returned by PartsDB functions
 *
 * All functions that can fail return a PartsDBError.
 * Use partsdb_error_string() to get a human-readable message.
 */
typedef enum PartsDBError {
    PARTSDB_OK = 0,              /**< Operation succeeded */
    PARTSDB_ERR_NULL_ARG,        /**< NULL argument passed */
    PARTSDB_ERR_INVALID_ID,      /**< Invalid part ID (0 or not found) */
    PARTSDB_ERR_INVALID_NAME,    /**< Invalid or empty part name */
    PARTSDB_ERR_INVALID_PRICE,   /**< Negative or NaN price */
    PARTSDB_ERR_DUPLICATE,       /**< Part with this ID already exists */
    PARTSDB_ERR_NOT_FOUND,       /**< Part not found */
    PARTSDB_ERR_FULL,            /**< Database is full */
    PARTSDB_ERR_OUT_OF_MEMORY,   /**< Memory allocation failed */
    PARTSDB_ERR_IO,              /**< File I/O error */
    PARTSDB_ERR_CORRUPT,         /**< Data corruption detected */
    PARTSDB_ERR_NOT_OPEN,        /**< Database not open */
    PARTSDB_ERR_ALREADY_OPEN,    /**< Database already open */
    PARTSDB_ERR_INTERNAL         /**< Internal error (bug) */
} PartsDBError;

/**
 * @brief Get human-readable error message
 * @param err Error code
 * @return Static string describing the error
 */
const char *partsdb_error_string(PartsDBError err);

/*============================================================================
 * Database Lifecycle
 *============================================================================*/

/**
 * @brief Configuration options for database creation
 */
typedef struct PartsDBConfig {
    size_t initial_capacity;   /**< Initial capacity (0 = default) */
    bool   enable_checksums;   /**< Enable data integrity checksums */
    bool   enable_logging;     /**< Enable operation logging */
} PartsDBConfig;

/**
 * @brief Default configuration
 */
#define PARTSDB_CONFIG_DEFAULT { \
    .initial_capacity = 0,       \
    .enable_checksums = true,    \
    .enable_logging = false      \
}

/**
 * @brief Create a new in-memory database
 *
 * @param[out] db      Pointer to receive database handle
 * @param[in]  config  Configuration options (NULL for defaults)
 * @return PARTSDB_OK on success, error code otherwise
 *
 * @note Caller must call partsdb_close() when done.
 *
 * Example:
 * @code
 *     PartsDB *db = NULL;
 *     PartsDBError err = partsdb_create(&db, NULL);
 *     if (err != PARTSDB_OK) {
 *         fprintf(stderr, "Failed: %s\n", partsdb_error_string(err));
 *         return 1;
 *     }
 *     // ... use database ...
 *     partsdb_close(db);
 * @endcode
 */
PartsDBError partsdb_create(PartsDB **db, const PartsDBConfig *config);

/**
 * @brief Open database from file
 *
 * @param[out] db    Pointer to receive database handle
 * @param[in]  path  Path to database file
 * @return PARTSDB_OK on success, error code otherwise
 */
PartsDBError partsdb_open(PartsDB **db, const char *path);

/**
 * @brief Save database to file
 *
 * @param[in] db    Database handle
 * @param[in] path  Path to save to
 * @return PARTSDB_OK on success, error code otherwise
 */
PartsDBError partsdb_save(const PartsDB *db, const char *path);

/**
 * @brief Close database and free resources
 *
 * @param db Database handle (NULL is safe)
 *
 * After this call, the handle is invalid and must not be used.
 */
void partsdb_close(PartsDB *db);

/*============================================================================
 * CRUD Operations
 *============================================================================*/

/**
 * @brief Insert a new part
 *
 * @param[in] db   Database handle
 * @param[in] part Part to insert (id must be non-zero and unique)
 * @return PARTSDB_OK on success, error code otherwise
 *
 * Validation:
 * - part->id must be non-zero
 * - part->id must not already exist
 * - part->name must be non-empty
 * - part->unit_price must be >= 0 and finite
 */
PartsDBError partsdb_insert(PartsDB *db, const Part *part);

/**
 * @brief Find a part by ID
 *
 * @param[in]  db     Database handle
 * @param[in]  id     Part ID to find
 * @param[out] result Part data (copied to caller's buffer)
 * @return PARTSDB_OK if found, PARTSDB_ERR_NOT_FOUND otherwise
 */
PartsDBError partsdb_find_by_id(const PartsDB *db, uint32_t id, Part *result);

/**
 * @brief Update an existing part
 *
 * @param[in] db   Database handle
 * @param[in] part Updated part data (matched by id)
 * @return PARTSDB_OK on success, PARTSDB_ERR_NOT_FOUND if id doesn't exist
 */
PartsDBError partsdb_update(PartsDB *db, const Part *part);

/**
 * @brief Delete a part by ID
 *
 * @param[in] db Database handle
 * @param[in] id Part ID to delete
 * @return PARTSDB_OK on success, PARTSDB_ERR_NOT_FOUND if id doesn't exist
 */
PartsDBError partsdb_delete(PartsDB *db, uint32_t id);

/*============================================================================
 * Query Operations
 *============================================================================*/

/**
 * @brief Get total number of parts
 *
 * @param[in] db Database handle
 * @return Number of parts, or 0 if db is NULL
 */
size_t partsdb_count(const PartsDB *db);

/**
 * @brief Search for parts by name pattern
 *
 * @param[in]  db           Database handle
 * @param[in]  pattern      Search pattern (substring match, case-insensitive)
 * @param[out] results      Array to receive matching parts
 * @param[in]  max_results  Maximum number of results to return
 * @param[out] found_count  Actual number of matches found
 * @return PARTSDB_OK on success (even if no matches)
 */
PartsDBError partsdb_search_by_name(
    const PartsDB *db,
    const char *pattern,
    Part *results,
    size_t max_results,
    size_t *found_count
);

/**
 * @brief Find parts with quantity below threshold
 *
 * @param[in]  db           Database handle
 * @param[in]  threshold    Quantity threshold
 * @param[out] results      Array to receive matching parts
 * @param[in]  max_results  Maximum number of results
 * @param[out] found_count  Actual number of matches
 * @return PARTSDB_OK on success
 */
PartsDBError partsdb_find_low_stock(
    const PartsDB *db,
    uint32_t threshold,
    Part *results,
    size_t max_results,
    size_t *found_count
);

/*============================================================================
 * Iterator Interface (Part 6: First-Class ADT)
 *============================================================================*/

/**
 * @brief Create iterator to traverse all parts
 *
 * @param[in]  db   Database handle
 * @param[out] iter Pointer to receive iterator
 * @return PARTSDB_OK on success
 *
 * @note Modifying the database while iterating is undefined behavior.
 */
PartsDBError partsdb_iter_create(const PartsDB *db, PartsDBIterator **iter);

/**
 * @brief Get next part from iterator
 *
 * @param[in]  iter Iterator handle
 * @param[out] part Part data
 * @return true if a part was returned, false if iteration complete
 */
bool partsdb_iter_next(PartsDBIterator *iter, Part *part);

/**
 * @brief Reset iterator to beginning
 *
 * @param[in] iter Iterator handle
 */
void partsdb_iter_reset(PartsDBIterator *iter);

/**
 * @brief Destroy iterator
 *
 * @param iter Iterator handle (NULL is safe)
 */
void partsdb_iter_destroy(PartsDBIterator *iter);

/*============================================================================
 * Utility Functions
 *============================================================================*/

/**
 * @brief Validate a Part structure
 *
 * Checks all fields for validity according to business rules.
 *
 * @param[in]  part    Part to validate
 * @param[out] errmsg  Optional buffer for error message (can be NULL)
 * @param[in]  errlen  Size of error message buffer
 * @return true if valid, false otherwise
 */
bool partsdb_validate_part(const Part *part, char *errmsg, size_t errlen);

/**
 * @brief Initialize a Part structure with defaults
 *
 * @param[out] part Part to initialize
 */
void partsdb_part_init(Part *part);

/**
 * @brief Get database statistics
 */
typedef struct PartsDBStats {
    size_t   part_count;        /**< Number of parts */
    size_t   capacity;          /**< Current capacity */
    size_t   memory_used;       /**< Approximate memory usage in bytes */
    uint64_t insert_count;      /**< Total inserts since creation */
    uint64_t update_count;      /**< Total updates since creation */
    uint64_t delete_count;      /**< Total deletes since creation */
    uint64_t search_count;      /**< Total searches since creation */
} PartsDBStats;

/**
 * @brief Get database statistics
 *
 * @param[in]  db    Database handle
 * @param[out] stats Statistics structure
 * @return PARTSDB_OK on success
 */
PartsDBError partsdb_get_stats(const PartsDB *db, PartsDBStats *stats);

#ifdef __cplusplus
}
#endif

#endif /* PARTSDB_H */
