using System.Data;
using Microsoft.Data.Sqlite;
using Dapper;
using PartsDb.Shared.Models;

namespace PartsDb.Shared.Data;

public class PartsDbContext : IDisposable
{
    private readonly SqliteConnection _connection;
    private bool _disposed;

    public PartsDbContext(string databasePath)
    {
        var connectionString = new SqliteConnectionStringBuilder
        {
            DataSource = databasePath,
            Mode = SqliteOpenMode.ReadWriteCreate,
            Cache = SqliteCacheMode.Shared
        }.ToString();

        _connection = new SqliteConnection(connectionString);
    }

    public async Task InitializeAsync()
    {
        await _connection.OpenAsync();
        await ExecuteSchemaAsync();
    }

    private async Task ExecuteSchemaAsync()
    {
        var schemaPath = Path.Combine(AppContext.BaseDirectory, "database", "schema.sql");

        if (!File.Exists(schemaPath))
        {
            schemaPath = Path.Combine(Directory.GetCurrentDirectory(), "database", "schema.sql");
        }

        if (File.Exists(schemaPath))
        {
            var schema = await File.ReadAllTextAsync(schemaPath);
            await _connection.ExecuteAsync(schema);
        }
    }

    public IDbConnection Connection => _connection;

    #region Parts

    public async Task<IEnumerable<Part>> GetPartsAsync(int limit = 100, int offset = 0)
    {
        const string sql = """
            SELECT p.*, c.id, c.name, c.description
            FROM parts p
            LEFT JOIN categories c ON p.category_id = c.id
            WHERE p.is_active = 1
            ORDER BY p.updated_at DESC
            LIMIT @Limit OFFSET @Offset
            """;

        var parts = await _connection.QueryAsync<Part, Category, Part>(
            sql,
            (part, category) =>
            {
                part.Category = category;
                return part;
            },
            new { Limit = limit, Offset = offset },
            splitOn: "id");

        return parts;
    }

    public async Task<Part?> GetPartByIdAsync(int id)
    {
        const string sql = """
            SELECT p.*, c.id, c.name, c.description
            FROM parts p
            LEFT JOIN categories c ON p.category_id = c.id
            WHERE p.id = @Id
            """;

        var parts = await _connection.QueryAsync<Part, Category, Part>(
            sql,
            (part, category) =>
            {
                part.Category = category;
                return part;
            },
            new { Id = id },
            splitOn: "id");

        return parts.FirstOrDefault();
    }

    public async Task<Part?> GetPartByNumberAsync(string partNumber)
    {
        const string sql = """
            SELECT p.*, c.id, c.name, c.description
            FROM parts p
            LEFT JOIN categories c ON p.category_id = c.id
            WHERE p.part_number = @PartNumber
            """;

        var parts = await _connection.QueryAsync<Part, Category, Part>(
            sql,
            (part, category) =>
            {
                part.Category = category;
                return part;
            },
            new { PartNumber = partNumber },
            splitOn: "id");

        return parts.FirstOrDefault();
    }

    public async Task<IEnumerable<Part>> SearchPartsAsync(string query)
    {
        const string sql = """
            SELECT p.*, c.id, c.name, c.description
            FROM parts p
            LEFT JOIN categories c ON p.category_id = c.id
            WHERE p.id IN (
                SELECT rowid FROM parts_fts WHERE parts_fts MATCH @Query
            )
            ORDER BY p.name
            LIMIT 100
            """;

        var ftsQuery = string.Join(" OR ", query.Split(' ', StringSplitOptions.RemoveEmptyEntries)
            .Select(t => $"\"{t}\"*"));

        var parts = await _connection.QueryAsync<Part, Category, Part>(
            sql,
            (part, category) =>
            {
                part.Category = category;
                return part;
            },
            new { Query = ftsQuery },
            splitOn: "id");

        return parts;
    }

    public async Task<int> CreatePartAsync(Part part)
    {
        const string sql = """
            INSERT INTO parts (part_number, name, description, category_id, manufacturer,
                             quantity, min_quantity, unit_price, currency, location,
                             datasheet_url, notes, is_active)
            VALUES (@PartNumber, @Name, @Description, @CategoryId, @Manufacturer,
                   @Quantity, @MinQuantity, @UnitPrice, @Currency, @Location,
                   @DatasheetUrl, @Notes, @IsActive);
            SELECT last_insert_rowid();
            """;

        return await _connection.ExecuteScalarAsync<int>(sql, part);
    }

    public async Task UpdatePartAsync(Part part)
    {
        const string sql = """
            UPDATE parts SET
                part_number = @PartNumber,
                name = @Name,
                description = @Description,
                category_id = @CategoryId,
                manufacturer = @Manufacturer,
                quantity = @Quantity,
                min_quantity = @MinQuantity,
                unit_price = @UnitPrice,
                currency = @Currency,
                location = @Location,
                datasheet_url = @DatasheetUrl,
                notes = @Notes,
                is_active = @IsActive
            WHERE id = @Id
            """;

        await _connection.ExecuteAsync(sql, part);
    }

    public async Task DeletePartAsync(int id)
    {
        await _connection.ExecuteAsync("DELETE FROM parts WHERE id = @Id", new { Id = id });
    }

    public async Task UpdateQuantityAsync(int partId, int quantityChange, TransactionType type, string? reference = null)
    {
        using var transaction = _connection.BeginTransaction();
        try
        {
            await _connection.ExecuteAsync(
                "UPDATE parts SET quantity = quantity + @Change WHERE id = @Id",
                new { Change = quantityChange, Id = partId },
                transaction);

            await _connection.ExecuteAsync("""
                INSERT INTO inventory_transactions (part_id, quantity_change, transaction_type, reference)
                VALUES (@PartId, @Change, @Type, @Reference)
                """,
                new { PartId = partId, Change = quantityChange, Type = type.ToString().ToUpper(), Reference = reference },
                transaction);

            transaction.Commit();
        }
        catch
        {
            transaction.Rollback();
            throw;
        }
    }

    #endregion

    #region Categories

    public async Task<IEnumerable<Category>> GetCategoriesAsync()
    {
        return await _connection.QueryAsync<Category>("SELECT * FROM categories ORDER BY name");
    }

    public async Task<int> CreateCategoryAsync(Category category)
    {
        const string sql = """
            INSERT INTO categories (name, description, parent_id)
            VALUES (@Name, @Description, @ParentId);
            SELECT last_insert_rowid();
            """;

        return await _connection.ExecuteScalarAsync<int>(sql, category);
    }

    #endregion

    #region Images

    public async Task<int> SaveImageAsync(PartImage image)
    {
        const string sql = """
            INSERT INTO part_images (part_id, image_full, image_thumbnail, image_embedded,
                                    original_filename, mime_type, width, height, file_size, is_primary)
            VALUES (@PartId, @ImageFull, @ImageThumbnail, @ImageEmbedded,
                   @OriginalFilename, @MimeType, @Width, @Height, @FileSize, @IsPrimary);
            SELECT last_insert_rowid();
            """;

        return await _connection.ExecuteScalarAsync<int>(sql, image);
    }

    public async Task<PartImage?> GetImageAsync(int partId, ImageSize size)
    {
        var column = size switch
        {
            ImageSize.Full => "image_full",
            ImageSize.Thumbnail => "image_thumbnail",
            ImageSize.Embedded => "image_embedded",
            _ => "image_thumbnail"
        };

        var sql = $"SELECT id, part_id, {column} as ImageFull, mime_type FROM part_images WHERE part_id = @PartId AND is_primary = 1";
        return await _connection.QueryFirstOrDefaultAsync<PartImage>(sql, new { PartId = partId });
    }

    public async Task<IEnumerable<PartImage>> GetPartImagesAsync(int partId)
    {
        return await _connection.QueryAsync<PartImage>(
            "SELECT id, part_id, original_filename, mime_type, width, height, file_size, is_primary, created_at FROM part_images WHERE part_id = @PartId",
            new { PartId = partId });
    }

    #endregion

    #region Low Stock

    public async Task<IEnumerable<Part>> GetLowStockPartsAsync()
    {
        const string sql = """
            SELECT p.*, c.id, c.name
            FROM parts p
            LEFT JOIN categories c ON p.category_id = c.id
            WHERE p.is_active = 1 AND p.quantity <= p.min_quantity
            ORDER BY p.quantity ASC
            """;

        var parts = await _connection.QueryAsync<Part, Category, Part>(
            sql,
            (part, category) =>
            {
                part.Category = category;
                return part;
            },
            splitOn: "id");

        return parts;
    }

    #endregion

    public void Dispose()
    {
        if (!_disposed)
        {
            _connection.Dispose();
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}
