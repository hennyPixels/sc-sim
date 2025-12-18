/**
 * PartsDB CLI - Database Implementation
 * SQLite3 wrapper for parts database operations
 */

#include "partsdb.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct partsdb {
    sqlite3 *db;
    char *path;
};

/* Helper to copy string safely */
static void safe_strcpy(char *dest, const char *src, size_t dest_size) {
    if (src == nullptr) {
        dest[0] = '\0';
        return;
    }
    size_t len = strlen(src);
    if (len >= dest_size) {
        len = dest_size - 1;
    }
    memcpy(dest, src, len);
    dest[len] = '\0';
}

/* Convert SQLite text to string, handling NULL */
static const char *sqlite_text(sqlite3_stmt *stmt, int col) {
    const unsigned char *text = sqlite3_column_text(stmt, col);
    return text ? (const char *)text : "";
}

const char *partsdb_error_string(partsdb_error_t error) {
    switch (error) {
        case PARTSDB_OK: return "Success";
        case PARTSDB_ERROR_OPEN: return "Failed to open database";
        case PARTSDB_ERROR_QUERY: return "Query execution failed";
        case PARTSDB_ERROR_MEMORY: return "Memory allocation failed";
        case PARTSDB_ERROR_NOT_FOUND: return "Record not found";
        case PARTSDB_ERROR_INVALID: return "Invalid parameter";
        case PARTSDB_ERROR_IO: return "I/O error";
        default: return "Unknown error";
    }
}

partsdb_error_t partsdb_open(const char *path, partsdb_t **db) {
    if (path == nullptr || db == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    partsdb_t *ctx = calloc(1, sizeof(partsdb_t));
    if (ctx == nullptr) {
        return PARTSDB_ERROR_MEMORY;
    }

    int rc = sqlite3_open_v2(path, &ctx->db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

    if (rc != SQLITE_OK) {
        free(ctx);
        return PARTSDB_ERROR_OPEN;
    }

    ctx->path = strdup(path);
    *db = ctx;

    /* Enable foreign keys */
    sqlite3_exec(ctx->db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    return PARTSDB_OK;
}

void partsdb_close(partsdb_t *db) {
    if (db == nullptr) return;

    if (db->db) {
        sqlite3_close(db->db);
    }
    free(db->path);
    free(db);
}

partsdb_error_t partsdb_get_all(partsdb_t *db, part_t **parts, size_t *count,
                                 size_t limit, size_t offset) {
    if (db == nullptr || parts == nullptr || count == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    const char *sql =
        "SELECT p.id, p.part_number, p.name, p.description, "
        "       COALESCE(c.name, '') as category, p.manufacturer, "
        "       p.quantity, p.min_quantity, p.unit_price, p.location, "
        "       p.created_at, p.updated_at "
        "FROM parts p "
        "LEFT JOIN categories c ON p.category_id = c.id "
        "WHERE p.is_active = 1 "
        "ORDER BY p.updated_at DESC "
        "LIMIT ? OFFSET ?;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)limit);
    sqlite3_bind_int64(stmt, 2, (sqlite3_int64)offset);

    /* First pass: count results */
    size_t n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        n++;
    }

    if (n == 0) {
        sqlite3_finalize(stmt);
        *parts = nullptr;
        *count = 0;
        return PARTSDB_OK;
    }

    /* Allocate array */
    part_t *result = calloc(n, sizeof(part_t));
    if (result == nullptr) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_MEMORY;
    }

    /* Reset and read data */
    sqlite3_reset(stmt);
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < n) {
        result[i].id = sqlite3_column_int64(stmt, 0);
        safe_strcpy(result[i].part_number, sqlite_text(stmt, 1), sizeof(result[i].part_number));
        safe_strcpy(result[i].name, sqlite_text(stmt, 2), sizeof(result[i].name));
        safe_strcpy(result[i].description, sqlite_text(stmt, 3), sizeof(result[i].description));
        safe_strcpy(result[i].category, sqlite_text(stmt, 4), sizeof(result[i].category));
        safe_strcpy(result[i].manufacturer, sqlite_text(stmt, 5), sizeof(result[i].manufacturer));
        result[i].quantity = sqlite3_column_int(stmt, 6);
        result[i].min_quantity = sqlite3_column_int(stmt, 7);
        result[i].unit_price = sqlite3_column_double(stmt, 8);
        safe_strcpy(result[i].location, sqlite_text(stmt, 9), sizeof(result[i].location));
        safe_strcpy(result[i].created_at, sqlite_text(stmt, 10), sizeof(result[i].created_at));
        safe_strcpy(result[i].updated_at, sqlite_text(stmt, 11), sizeof(result[i].updated_at));
        i++;
    }

    sqlite3_finalize(stmt);
    *parts = result;
    *count = i;

    return PARTSDB_OK;
}

partsdb_error_t partsdb_get_by_id(partsdb_t *db, int64_t id, part_t *part) {
    if (db == nullptr || part == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    const char *sql =
        "SELECT p.id, p.part_number, p.name, p.description, "
        "       COALESCE(c.name, '') as category, p.manufacturer, "
        "       p.quantity, p.min_quantity, p.unit_price, p.location, "
        "       p.created_at, p.updated_at "
        "FROM parts p "
        "LEFT JOIN categories c ON p.category_id = c.id "
        "WHERE p.id = ?;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_int64(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_NOT_FOUND;
    }

    part->id = sqlite3_column_int64(stmt, 0);
    safe_strcpy(part->part_number, sqlite_text(stmt, 1), sizeof(part->part_number));
    safe_strcpy(part->name, sqlite_text(stmt, 2), sizeof(part->name));
    safe_strcpy(part->description, sqlite_text(stmt, 3), sizeof(part->description));
    safe_strcpy(part->category, sqlite_text(stmt, 4), sizeof(part->category));
    safe_strcpy(part->manufacturer, sqlite_text(stmt, 5), sizeof(part->manufacturer));
    part->quantity = sqlite3_column_int(stmt, 6);
    part->min_quantity = sqlite3_column_int(stmt, 7);
    part->unit_price = sqlite3_column_double(stmt, 8);
    safe_strcpy(part->location, sqlite_text(stmt, 9), sizeof(part->location));
    safe_strcpy(part->created_at, sqlite_text(stmt, 10), sizeof(part->created_at));
    safe_strcpy(part->updated_at, sqlite_text(stmt, 11), sizeof(part->updated_at));

    sqlite3_finalize(stmt);
    return PARTSDB_OK;
}

partsdb_error_t partsdb_get_by_number(partsdb_t *db, const char *part_number, part_t *part) {
    if (db == nullptr || part_number == nullptr || part == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    const char *sql =
        "SELECT p.id, p.part_number, p.name, p.description, "
        "       COALESCE(c.name, '') as category, p.manufacturer, "
        "       p.quantity, p.min_quantity, p.unit_price, p.location, "
        "       p.created_at, p.updated_at "
        "FROM parts p "
        "LEFT JOIN categories c ON p.category_id = c.id "
        "WHERE p.part_number = ?;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, part_number, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_NOT_FOUND;
    }

    part->id = sqlite3_column_int64(stmt, 0);
    safe_strcpy(part->part_number, sqlite_text(stmt, 1), sizeof(part->part_number));
    safe_strcpy(part->name, sqlite_text(stmt, 2), sizeof(part->name));
    safe_strcpy(part->description, sqlite_text(stmt, 3), sizeof(part->description));
    safe_strcpy(part->category, sqlite_text(stmt, 4), sizeof(part->category));
    safe_strcpy(part->manufacturer, sqlite_text(stmt, 5), sizeof(part->manufacturer));
    part->quantity = sqlite3_column_int(stmt, 6);
    part->min_quantity = sqlite3_column_int(stmt, 7);
    part->unit_price = sqlite3_column_double(stmt, 8);
    safe_strcpy(part->location, sqlite_text(stmt, 9), sizeof(part->location));
    safe_strcpy(part->created_at, sqlite_text(stmt, 10), sizeof(part->created_at));
    safe_strcpy(part->updated_at, sqlite_text(stmt, 11), sizeof(part->updated_at));

    sqlite3_finalize(stmt);
    return PARTSDB_OK;
}

partsdb_error_t partsdb_search(partsdb_t *db, const char *query, part_t **parts, size_t *count) {
    if (db == nullptr || query == nullptr || parts == nullptr || count == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    /* Build FTS query */
    char fts_query[256];
    snprintf(fts_query, sizeof(fts_query), "\"%s\"*", query);

    const char *sql =
        "SELECT p.id, p.part_number, p.name, p.description, "
        "       COALESCE(c.name, '') as category, p.manufacturer, "
        "       p.quantity, p.min_quantity, p.unit_price, p.location, "
        "       p.created_at, p.updated_at "
        "FROM parts p "
        "LEFT JOIN categories c ON p.category_id = c.id "
        "WHERE p.id IN (SELECT rowid FROM parts_fts WHERE parts_fts MATCH ?) "
        "ORDER BY p.name LIMIT 100;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, fts_query, -1, SQLITE_STATIC);

    /* Count results */
    size_t n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        n++;
    }

    if (n == 0) {
        sqlite3_finalize(stmt);
        *parts = nullptr;
        *count = 0;
        return PARTSDB_OK;
    }

    part_t *result = calloc(n, sizeof(part_t));
    if (result == nullptr) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_MEMORY;
    }

    sqlite3_reset(stmt);
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < n) {
        result[i].id = sqlite3_column_int64(stmt, 0);
        safe_strcpy(result[i].part_number, sqlite_text(stmt, 1), sizeof(result[i].part_number));
        safe_strcpy(result[i].name, sqlite_text(stmt, 2), sizeof(result[i].name));
        safe_strcpy(result[i].description, sqlite_text(stmt, 3), sizeof(result[i].description));
        safe_strcpy(result[i].category, sqlite_text(stmt, 4), sizeof(result[i].category));
        safe_strcpy(result[i].manufacturer, sqlite_text(stmt, 5), sizeof(result[i].manufacturer));
        result[i].quantity = sqlite3_column_int(stmt, 6);
        result[i].min_quantity = sqlite3_column_int(stmt, 7);
        result[i].unit_price = sqlite3_column_double(stmt, 8);
        safe_strcpy(result[i].location, sqlite_text(stmt, 9), sizeof(result[i].location));
        safe_strcpy(result[i].created_at, sqlite_text(stmt, 10), sizeof(result[i].created_at));
        safe_strcpy(result[i].updated_at, sqlite_text(stmt, 11), sizeof(result[i].updated_at));
        i++;
    }

    sqlite3_finalize(stmt);
    *parts = result;
    *count = i;

    return PARTSDB_OK;
}

partsdb_error_t partsdb_get_low_stock(partsdb_t *db, part_t **parts, size_t *count) {
    if (db == nullptr || parts == nullptr || count == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    const char *sql =
        "SELECT p.id, p.part_number, p.name, p.description, "
        "       COALESCE(c.name, '') as category, p.manufacturer, "
        "       p.quantity, p.min_quantity, p.unit_price, p.location, "
        "       p.created_at, p.updated_at "
        "FROM parts p "
        "LEFT JOIN categories c ON p.category_id = c.id "
        "WHERE p.is_active = 1 AND p.quantity <= p.min_quantity "
        "ORDER BY p.quantity ASC;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    size_t n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        n++;
    }

    if (n == 0) {
        sqlite3_finalize(stmt);
        *parts = nullptr;
        *count = 0;
        return PARTSDB_OK;
    }

    part_t *result = calloc(n, sizeof(part_t));
    if (result == nullptr) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_MEMORY;
    }

    sqlite3_reset(stmt);
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < n) {
        result[i].id = sqlite3_column_int64(stmt, 0);
        safe_strcpy(result[i].part_number, sqlite_text(stmt, 1), sizeof(result[i].part_number));
        safe_strcpy(result[i].name, sqlite_text(stmt, 2), sizeof(result[i].name));
        safe_strcpy(result[i].description, sqlite_text(stmt, 3), sizeof(result[i].description));
        safe_strcpy(result[i].category, sqlite_text(stmt, 4), sizeof(result[i].category));
        safe_strcpy(result[i].manufacturer, sqlite_text(stmt, 5), sizeof(result[i].manufacturer));
        result[i].quantity = sqlite3_column_int(stmt, 6);
        result[i].min_quantity = sqlite3_column_int(stmt, 7);
        result[i].unit_price = sqlite3_column_double(stmt, 8);
        safe_strcpy(result[i].location, sqlite_text(stmt, 9), sizeof(result[i].location));
        safe_strcpy(result[i].created_at, sqlite_text(stmt, 10), sizeof(result[i].created_at));
        safe_strcpy(result[i].updated_at, sqlite_text(stmt, 11), sizeof(result[i].updated_at));
        i++;
    }

    sqlite3_finalize(stmt);
    *parts = result;
    *count = i;

    return PARTSDB_OK;
}

partsdb_error_t partsdb_create(partsdb_t *db, const part_t *part, int64_t *id) {
    if (db == nullptr || part == nullptr || id == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    const char *sql =
        "INSERT INTO parts (part_number, name, description, manufacturer, "
        "                   quantity, min_quantity, unit_price, location) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, part->part_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, part->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, part->description, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, part->manufacturer, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, part->quantity);
    sqlite3_bind_int(stmt, 6, part->min_quantity);
    sqlite3_bind_double(stmt, 7, part->unit_price);
    sqlite3_bind_text(stmt, 8, part->location, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_QUERY;
    }

    *id = sqlite3_last_insert_rowid(db->db);
    sqlite3_finalize(stmt);

    return PARTSDB_OK;
}

partsdb_error_t partsdb_update(partsdb_t *db, const part_t *part) {
    if (db == nullptr || part == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    const char *sql =
        "UPDATE parts SET part_number = ?, name = ?, description = ?, "
        "       manufacturer = ?, quantity = ?, min_quantity = ?, "
        "       unit_price = ?, location = ? "
        "WHERE id = ?;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_text(stmt, 1, part->part_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, part->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, part->description, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, part->manufacturer, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, part->quantity);
    sqlite3_bind_int(stmt, 6, part->min_quantity);
    sqlite3_bind_double(stmt, 7, part->unit_price);
    sqlite3_bind_text(stmt, 8, part->location, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 9, part->id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_finalize(stmt);
    return PARTSDB_OK;
}

partsdb_error_t partsdb_delete(partsdb_t *db, int64_t id) {
    if (db == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    const char *sql = "DELETE FROM parts WHERE id = ?;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_int64(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_finalize(stmt);
    return PARTSDB_OK;
}

partsdb_error_t partsdb_update_quantity(partsdb_t *db, int64_t id, int32_t change, const char *type) {
    if (db == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    char *err_msg = nullptr;
    sqlite3_exec(db->db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    /* Update quantity */
    const char *update_sql = "UPDATE parts SET quantity = quantity + ? WHERE id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db->db, update_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_int(stmt, 1, change);
    sqlite3_bind_int64(stmt, 2, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_exec(db->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return PARTSDB_ERROR_QUERY;
    }
    sqlite3_finalize(stmt);

    /* Record transaction */
    const char *trans_sql =
        "INSERT INTO inventory_transactions (part_id, quantity_change, transaction_type) "
        "VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db->db, trans_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_int64(stmt, 1, id);
    sqlite3_bind_int(stmt, 2, change);
    sqlite3_bind_text(stmt, 3, type ? type : "ADJUST", -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_exec(db->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return PARTSDB_ERROR_QUERY;
    }
    sqlite3_finalize(stmt);

    sqlite3_exec(db->db, "COMMIT;", nullptr, nullptr, nullptr);
    return PARTSDB_OK;
}

partsdb_error_t partsdb_get_categories(partsdb_t *db, category_t **categories, size_t *count) {
    if (db == nullptr || categories == nullptr || count == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    const char *sql = "SELECT id, name, description, parent_id FROM categories ORDER BY name;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    size_t n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        n++;
    }

    if (n == 0) {
        sqlite3_finalize(stmt);
        *categories = nullptr;
        *count = 0;
        return PARTSDB_OK;
    }

    category_t *result = calloc(n, sizeof(category_t));
    if (result == nullptr) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_MEMORY;
    }

    sqlite3_reset(stmt);
    size_t i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < n) {
        result[i].id = sqlite3_column_int64(stmt, 0);
        safe_strcpy(result[i].name, sqlite_text(stmt, 1), sizeof(result[i].name));
        safe_strcpy(result[i].description, sqlite_text(stmt, 2), sizeof(result[i].description));
        result[i].parent_id = sqlite3_column_int64(stmt, 3);
        i++;
    }

    sqlite3_finalize(stmt);
    *categories = result;
    *count = i;

    return PARTSDB_OK;
}

partsdb_error_t partsdb_get_image_embedded(partsdb_t *db, int64_t part_id, embedded_image_t *image) {
    if (db == nullptr || image == nullptr) {
        return PARTSDB_ERROR_INVALID;
    }

    const char *sql =
        "SELECT image_embedded FROM part_images WHERE part_id = ? AND is_primary = 1;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return PARTSDB_ERROR_QUERY;
    }

    sqlite3_bind_int64(stmt, 1, part_id);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_NOT_FOUND;
    }

    const void *blob = sqlite3_column_blob(stmt, 0);
    int blob_size = sqlite3_column_bytes(stmt, 0);

    if (blob == nullptr || blob_size < 4) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_NOT_FOUND;
    }

    /* Parse header */
    const uint8_t *data = (const uint8_t *)blob;
    image->width = data[0] | (data[1] << 8);
    image->height = data[2] | (data[3] << 8);
    image->data_size = blob_size - 4;

    image->data = malloc(image->data_size);
    if (image->data == nullptr) {
        sqlite3_finalize(stmt);
        return PARTSDB_ERROR_MEMORY;
    }

    memcpy(image->data, data + 4, image->data_size);

    sqlite3_finalize(stmt);
    return PARTSDB_OK;
}

void partsdb_free_image(embedded_image_t *image) {
    if (image != nullptr && image->data != nullptr) {
        free(image->data);
        image->data = nullptr;
    }
}

void partsdb_free_parts(part_t *parts) {
    free(parts);
}

void partsdb_free_categories(category_t *categories) {
    free(categories);
}

/* ASCII art rendering for embedded images */
static const char ASCII_CHARS[] = " .:-=+*#%@";

void partsdb_render_image_ascii(const embedded_image_t *image, char *buffer,
                                 size_t buffer_size, int target_width) {
    if (image == nullptr || buffer == nullptr || image->data == nullptr) {
        if (buffer && buffer_size > 0) buffer[0] = '\0';
        return;
    }

    /* Decompress RLE and render */
    size_t buf_pos = 0;
    int x = 0, y = 0;

    /* Calculate scale */
    float scale_x = (float)image->width / target_width;
    float scale_y = scale_x * 2.0f;  /* Terminal chars are ~2:1 aspect ratio */
    int target_height = (int)(image->height / scale_y);

    for (int ty = 0; ty < target_height && buf_pos < buffer_size - 2; ty++) {
        for (int tx = 0; tx < target_width && buf_pos < buffer_size - 2; tx++) {
            int src_x = (int)(tx * scale_x);
            int src_y = (int)(ty * scale_y);

            if (src_x >= image->width) src_x = image->width - 1;
            if (src_y >= image->height) src_y = image->height - 1;

            /* Get pixel from compressed data (simplified - assumes no RLE for display) */
            int pixel_idx = src_y * image->width + src_x;
            int byte_idx = pixel_idx / 2;
            uint8_t gray4;

            if (byte_idx < (int)image->data_size) {
                uint8_t byte = image->data[byte_idx];
                gray4 = (pixel_idx % 2 == 0) ? (byte >> 4) : (byte & 0x0F);
            } else {
                gray4 = 0;
            }

            /* Map 4-bit grayscale to ASCII */
            int char_idx = (gray4 * (sizeof(ASCII_CHARS) - 2)) / 15;
            buffer[buf_pos++] = ASCII_CHARS[char_idx];
        }
        buffer[buf_pos++] = '\n';
    }

    buffer[buf_pos] = '\0';
}
