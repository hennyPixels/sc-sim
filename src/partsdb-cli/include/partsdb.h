/**
 * PartsDB CLI - C23 Library Header
 * Lightweight parts database interface for Linux and embedded systems
 */

#ifndef PARTSDB_H
#define PARTSDB_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version */
#define PARTSDB_VERSION_MAJOR 1
#define PARTSDB_VERSION_MINOR 0
#define PARTSDB_VERSION_PATCH 0

/* Error codes */
typedef enum {
    PARTSDB_OK = 0,
    PARTSDB_ERROR_OPEN = -1,
    PARTSDB_ERROR_QUERY = -2,
    PARTSDB_ERROR_MEMORY = -3,
    PARTSDB_ERROR_NOT_FOUND = -4,
    PARTSDB_ERROR_INVALID = -5,
    PARTSDB_ERROR_IO = -6
} partsdb_error_t;

/* Forward declarations */
typedef struct partsdb partsdb_t;

/* Part structure */
typedef struct {
    int64_t id;
    char part_number[64];
    char name[128];
    char description[512];
    char category[64];
    char manufacturer[128];
    int32_t quantity;
    int32_t min_quantity;
    double unit_price;
    char location[64];
    char created_at[32];
    char updated_at[32];
} part_t;

/* Category structure */
typedef struct {
    int64_t id;
    char name[64];
    char description[256];
    int64_t parent_id;
} category_t;

/* Embedded image structure (compressed format) */
typedef struct {
    uint16_t width;
    uint16_t height;
    size_t data_size;
    uint8_t *data;  /* 4-bit grayscale RLE compressed */
} embedded_image_t;

/* Database operations */
partsdb_error_t partsdb_open(const char *path, partsdb_t **db);
void partsdb_close(partsdb_t *db);
const char *partsdb_error_string(partsdb_error_t error);

/* Part queries */
partsdb_error_t partsdb_get_all(partsdb_t *db, part_t **parts, size_t *count, size_t limit, size_t offset);
partsdb_error_t partsdb_get_by_id(partsdb_t *db, int64_t id, part_t *part);
partsdb_error_t partsdb_get_by_number(partsdb_t *db, const char *part_number, part_t *part);
partsdb_error_t partsdb_search(partsdb_t *db, const char *query, part_t **parts, size_t *count);
partsdb_error_t partsdb_get_low_stock(partsdb_t *db, part_t **parts, size_t *count);

/* Part mutations */
partsdb_error_t partsdb_create(partsdb_t *db, const part_t *part, int64_t *id);
partsdb_error_t partsdb_update(partsdb_t *db, const part_t *part);
partsdb_error_t partsdb_delete(partsdb_t *db, int64_t id);
partsdb_error_t partsdb_update_quantity(partsdb_t *db, int64_t id, int32_t change, const char *type);

/* Category operations */
partsdb_error_t partsdb_get_categories(partsdb_t *db, category_t **categories, size_t *count);

/* Image operations (embedded format) */
partsdb_error_t partsdb_get_image_embedded(partsdb_t *db, int64_t part_id, embedded_image_t *image);
void partsdb_free_image(embedded_image_t *image);

/* Memory management */
void partsdb_free_parts(part_t *parts);
void partsdb_free_categories(category_t *categories);

/* Utility functions */
void partsdb_render_image_ascii(const embedded_image_t *image, char *buffer, size_t buffer_size, int width);

#ifdef __cplusplus
}
#endif

#endif /* PARTSDB_H */
