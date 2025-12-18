// =============================================================================
// DATA ACCESS LAYER - Database Operations with Dapper
// =============================================================================
// The Data Access Layer (DAL) is responsible for all database communication.
// This class uses Dapper, a "micro-ORM" (Object-Relational Mapper) that maps
// database rows to C# objects with minimal overhead.
//
// 📖 WHY DAPPER OVER ENTITY FRAMEWORK?
// - Dapper is lightweight (~20KB) vs EF Core (~10MB)
// - Dapper gives you full SQL control
// - Dapper is faster for simple operations
// - EF Core is better for complex object graphs and change tracking
//
// 📚 Learn more:
// - Dapper: https://github.com/DapperLib/Dapper
// - SQLite: https://www.sqlite.org/docs.html
// - Data Access Patterns: https://learn.microsoft.com/en-us/dotnet/architecture/microservices/microservice-ddd-cqrs-patterns/
// =============================================================================

using System.Data;
using Microsoft.Data.Sqlite;
using Dapper;
using PartsDb.Shared.Models;

namespace PartsDb.Shared.Data;

// =============================================================================
// PartsDbContext CLASS
// =============================================================================
/// <summary>
/// Provides database access for the parts inventory system.
/// Implements IDisposable to properly clean up database connections.
/// </summary>
/// <remarks>
/// 📖 IDisposable PATTERN:
/// When a class holds "unmanaged resources" (database connections, file handles,
/// network sockets), it should implement IDisposable to release them properly.
///
/// Usage:
///   using var context = new PartsDbContext("parts.db");
///   // When 'using' block ends, Dispose() is called automatically
///
/// Or manually:
///   var context = new PartsDbContext("parts.db");
///   try { ... }
///   finally { context.Dispose(); }
///
/// 📚 Deep dive: https://learn.microsoft.com/en-us/dotnet/standard/garbage-collection/implementing-dispose
/// </remarks>
public class PartsDbContext : IDisposable
{
    // -------------------------------------------------------------------------
    // PRIVATE FIELDS
    // -------------------------------------------------------------------------

    /// <summary>
    /// The SQLite database connection.
    /// </summary>
    /// <remarks>
    /// 📖 SqliteConnection:
    /// - Represents a connection to a SQLite database
    /// - Must be opened before use, closed when done
    /// - Part of Microsoft.Data.Sqlite package
    /// - Thread-safe when Cache=Shared is enabled
    /// </remarks>
    private readonly SqliteConnection _connection;

    /// <summary>
    /// Tracks whether Dispose has been called.
    /// </summary>
    /// <remarks>
    /// 📖 DISPOSE PATTERN:
    /// - Prevents double-disposal (calling Dispose twice)
    /// - Simple boolean flag pattern
    /// - Set to true in Dispose() method
    /// </remarks>
    private bool _disposed;

    // -------------------------------------------------------------------------
    // CONSTRUCTOR
    // -------------------------------------------------------------------------
    /// <summary>
    /// Creates a new database context for the specified database file.
    /// </summary>
    /// <param name="databasePath">Path to the SQLite database file.</param>
    /// <remarks>
    /// 📖 SqliteConnectionStringBuilder:
    /// - Type-safe way to build connection strings
    /// - Better than string concatenation: "Data Source=x;Mode=y"
    /// - Properties match SQLite connection string options
    ///
    /// 📖 Connection String Options:
    /// - DataSource: Path to the .db file
    /// - Mode: ReadWriteCreate creates file if it doesn't exist
    /// - Cache: Shared enables multi-threaded access
    /// </remarks>
    public PartsDbContext(string databasePath)
    {
        // 📖 OBJECT INITIALIZER SYNTAX:
        // Create object and set properties in one statement
        var connectionString = new SqliteConnectionStringBuilder
        {
            DataSource = databasePath,
            Mode = SqliteOpenMode.ReadWriteCreate,  // Create DB if missing
            Cache = SqliteCacheMode.Shared          // Allow concurrent access
        }.ToString();  // Convert to "Data Source=...;Mode=...;Cache=..." string

        _connection = new SqliteConnection(connectionString);
    }

    // -------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------
    /// <summary>
    /// Opens the database connection and creates schema if needed.
    /// </summary>
    /// <remarks>
    /// 📖 ASYNC INITIALIZATION PATTERN:
    /// Constructors can't be async, so we use a separate InitializeAsync method.
    /// This is a common pattern when async work is needed during setup.
    ///
    /// Usage:
    ///   var context = new PartsDbContext("parts.db");
    ///   await context.InitializeAsync();  // Must call before using!
    ///
    /// Alternative: Factory pattern
    ///   public static async Task&lt;PartsDbContext&gt; CreateAsync(string path)
    ///   {
    ///       var context = new PartsDbContext(path);
    ///       await context.InitializeAsync();
    ///       return context;
    ///   }
    /// </remarks>
    public async Task InitializeAsync()
    {
        await _connection.OpenAsync();
        await ExecuteSchemaAsync();
    }

    /// <summary>
    /// Executes the database schema creation script.
    /// </summary>
    /// <remarks>
    /// 📖 SCHEMA MANAGEMENT:
    /// - Schema defines database structure (tables, indexes, etc.)
    /// - We store schema in a .sql file for easy maintenance
    /// - This method finds and executes that file
    ///
    /// 📖 Path.Combine:
    /// - Safely joins path segments with correct separator
    /// - Works on Windows (\\) and Linux/Mac (/)
    /// - Better than string concatenation
    ///
    /// 📖 AppContext.BaseDirectory:
    /// - Directory where the application's assembly is located
    /// - Useful for finding files deployed with the app
    /// </remarks>
    private async Task ExecuteSchemaAsync()
    {
        // Try to find schema.sql in the app's base directory first
        var schemaPath = Path.Combine(AppContext.BaseDirectory, "database", "schema.sql");

        // Fallback to current working directory (useful during development)
        if (!File.Exists(schemaPath))
        {
            schemaPath = Path.Combine(Directory.GetCurrentDirectory(), "database", "schema.sql");
        }

        // Execute schema if found
        if (File.Exists(schemaPath))
        {
            var schema = await File.ReadAllTextAsync(schemaPath);
            // 📖 ExecuteAsync: Dapper extension method for non-query SQL
            await _connection.ExecuteAsync(schema);
        }
    }

    // -------------------------------------------------------------------------
    // CONNECTION PROPERTY
    // -------------------------------------------------------------------------
    /// <summary>
    /// Exposes the underlying database connection.
    /// </summary>
    /// <remarks>
    /// 📖 EXPRESSION-BODIED PROPERTY:
    /// - The '=>' syntax is shorthand for a getter-only property
    /// - Returns IDbConnection (interface) instead of SqliteConnection (implementation)
    /// - This allows code to work with any database, not just SQLite
    ///
    /// Equivalent to:
    ///   public IDbConnection Connection
    ///   {
    ///       get { return _connection; }
    ///   }
    /// </remarks>
    public IDbConnection Connection => _connection;

    // =========================================================================
    // PARTS REGION - CRUD Operations for Parts
    // =========================================================================
    // 📖 #region DIRECTIVE:
    // - Allows collapsing code sections in IDEs
    // - Purely organizational, no runtime effect
    // - Pairs with #endregion
    // - Use sparingly - prefer small, focused classes

    #region Parts

    /// <summary>
    /// Retrieves a paginated list of active parts with their categories.
    /// </summary>
    /// <remarks>
    /// 📖 DAPPER MULTI-MAPPING:
    /// When a query returns data from multiple tables (JOIN), Dapper can
    /// automatically map results to multiple objects.
    ///
    /// QueryAsync&lt;Part, Category, Part&gt; means:
    /// - First type (Part): First set of columns
    /// - Second type (Category): Second set of columns
    /// - Third type (Part): The return type
    ///
    /// The lambda (part, category) => { ... } tells Dapper how to combine them.
    ///
    /// 📖 splitOn PARAMETER:
    /// - Tells Dapper where to split columns between types
    /// - "id" means: columns before second "id" go to Part, after go to Category
    /// - ORDER MATTERS in your SELECT statement!
    ///
    /// 📖 RAW STRING LITERALS (C# 11+):
    /// - Three quotes """ allow multi-line strings without escaping
    /// - Great for SQL queries
    /// - Whitespace before the closing """ sets the indentation baseline
    ///
    /// 📖 PARAMETERIZED QUERIES:
    /// - @Limit and @Offset are parameters, not string concatenation
    /// - Prevents SQL injection attacks
    /// - Dapper maps anonymous object properties to parameters
    /// </remarks>
    public async Task<IEnumerable<Part>> GetPartsAsync(int limit = 100, int offset = 0)
    {
        // 📖 const string: Compile-time constant, embedded directly in code
        const string sql = """
            SELECT p.*, c.id, c.name, c.description
            FROM parts p
            LEFT JOIN categories c ON p.category_id = c.id
            WHERE p.is_active = 1
            ORDER BY p.updated_at DESC
            LIMIT @Limit OFFSET @Offset
            """;

        // 📖 Multi-mapping: Map JOIN results to related objects
        var parts = await _connection.QueryAsync<Part, Category, Part>(
            sql,
            (part, category) =>
            {
                // This lambda runs for each row, combining Part + Category
                part.Category = category;
                return part;
            },
            new { Limit = limit, Offset = offset },  // Anonymous object for parameters
            splitOn: "id");  // Split columns at the second "id" column

        return parts;
    }

    /// <summary>
    /// Retrieves a single part by its ID.
    /// </summary>
    /// <remarks>
    /// 📖 NULLABLE RETURN TYPE (Part?):
    /// - The '?' indicates this method might return null
    /// - Null is returned when no part with that ID exists
    /// - Callers must handle the null case
    ///
    /// 📖 FirstOrDefault():
    /// - Returns the first element, or default(T) if empty
    /// - For reference types, default is null
    /// - Alternative: First() throws if empty, Single() throws if not exactly one
    /// </remarks>
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

    /// <summary>
    /// Searches for parts using full-text search.
    /// </summary>
    /// <remarks>
    /// 📖 SQLITE FTS5 (Full-Text Search):
    /// - FTS5 is SQLite's full-text search extension
    /// - Indexes text for fast searching (like Google)
    /// - Supports prefix matching, phrase search, boolean operators
    /// - We use MATCH keyword for FTS queries
    ///
    /// 📖 QUERY TRANSFORMATION:
    /// Input: "resistor 100k"
    /// Output: "\"resistor\"* OR \"100k\"*"
    /// - Each word wrapped in quotes (exact match)
    /// - * suffix for prefix matching
    /// - OR for any word match
    ///
    /// 📖 LINQ Select:
    /// - .Select(t => $"..") transforms each element
    /// - Similar to map() in other languages
    /// - Part of LINQ (Language Integrated Query)
    ///
    /// 📖 string.Join:
    /// - Combines array elements with separator
    /// - string.Join(" OR ", ["a", "b"]) => "a OR b"
    /// </remarks>
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

        // Transform user query into FTS5 format
        // "resistor 100k" => "\"resistor\"* OR \"100k\"*"
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

    /// <summary>
    /// Creates a new part in the database.
    /// </summary>
    /// <returns>The ID of the newly created part.</returns>
    /// <remarks>
    /// 📖 INSERT + SELECT last_insert_rowid():
    /// - SQLite's way to get the auto-generated ID
    /// - In one statement for atomicity
    /// - PostgreSQL uses RETURNING id
    /// - SQL Server uses SCOPE_IDENTITY()
    ///
    /// 📖 ExecuteScalarAsync&lt;int&gt;:
    /// - Executes query and returns single value
    /// - Generic type &lt;int&gt; specifies the return type
    /// - Use for: COUNT, SUM, MAX, or getting inserted ID
    ///
    /// 📖 DAPPER PARAMETER MAPPING:
    /// - Passing 'part' object directly
    /// - Dapper maps Part.PartNumber to @PartNumber automatically
    /// - Property names must match parameter names (case-insensitive)
    /// </remarks>
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

    /// <summary>
    /// Updates an existing part.
    /// </summary>
    /// <remarks>
    /// 📖 ExecuteAsync vs ExecuteScalarAsync:
    /// - ExecuteAsync: Returns number of rows affected (int)
    /// - ExecuteScalarAsync: Returns single value from query
    /// - Use ExecuteAsync for UPDATE/DELETE/INSERT without return value
    /// </remarks>
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

    /// <summary>
    /// Permanently deletes a part from the database.
    /// </summary>
    /// <remarks>
    /// 📖 HARD DELETE vs SOFT DELETE:
    /// - Hard delete: Actually removes the row (what this does)
    /// - Soft delete: Sets is_active = 0 (preserves data)
    /// - Choose based on business requirements
    /// - Soft delete is better for audit trails
    /// </remarks>
    public async Task DeletePartAsync(int id)
    {
        await _connection.ExecuteAsync("DELETE FROM parts WHERE id = @Id", new { Id = id });
    }

    /// <summary>
    /// Updates part quantity and logs the transaction.
    /// </summary>
    /// <remarks>
    /// 📖 DATABASE TRANSACTIONS:
    /// A transaction ensures multiple operations succeed or fail together.
    /// This is the ACID property of databases:
    /// - Atomic: All operations complete or none do
    /// - Consistent: Database stays in valid state
    /// - Isolated: Transactions don't interfere
    /// - Durable: Committed changes persist
    ///
    /// 📖 using var transaction:
    /// - Creates a database transaction
    /// - 'using' ensures it's disposed (rolled back if not committed)
    ///
    /// 📖 TRY-CATCH-ROLLBACK PATTERN:
    /// - Begin transaction
    /// - Try: Execute operations
    /// - Success: Commit
    /// - Failure: Rollback (undo all changes)
    ///
    /// Why transaction here? We need BOTH:
    /// 1. Update the quantity
    /// 2. Log the transaction
    /// If logging fails, we don't want the quantity change to persist.
    /// </remarks>
    public async Task UpdateQuantityAsync(int partId, int quantityChange, TransactionType type, string? reference = null)
    {
        // 📖 BeginTransaction: Starts a new database transaction
        using var transaction = _connection.BeginTransaction();
        try
        {
            // Operation 1: Update the part's quantity
            await _connection.ExecuteAsync(
                "UPDATE parts SET quantity = quantity + @Change WHERE id = @Id",
                new { Change = quantityChange, Id = partId },
                transaction);  // Pass transaction to include this operation

            // Operation 2: Log the transaction for audit trail
            await _connection.ExecuteAsync("""
                INSERT INTO inventory_transactions (part_id, quantity_change, transaction_type, reference)
                VALUES (@PartId, @Change, @Type, @Reference)
                """,
                new { PartId = partId, Change = quantityChange, Type = type.ToString().ToUpper(), Reference = reference },
                transaction);

            // 📖 Commit: Make all changes permanent
            transaction.Commit();
        }
        catch
        {
            // 📖 Rollback: Undo all changes if anything failed
            transaction.Rollback();
            throw;  // Re-throw the exception after cleanup
        }
    }

    #endregion

    // =========================================================================
    // CATEGORIES REGION
    // =========================================================================
    #region Categories

    /// <summary>
    /// Retrieves all categories.
    /// </summary>
    /// <remarks>
    /// 📖 QueryAsync&lt;Category&gt;:
    /// - Simple single-type mapping (no JOINs)
    /// - Dapper maps column names to property names
    /// - Uses convention: category_id => CategoryId (snake_case to PascalCase)
    /// </remarks>
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

    // =========================================================================
    // IMAGES REGION - Binary Data Storage
    // =========================================================================
    #region Images

    /// <summary>
    /// Saves image data for a part.
    /// </summary>
    /// <remarks>
    /// 📖 BLOB STORAGE:
    /// - BLOB = Binary Large OBject
    /// - SQLite stores byte[] as BLOB type
    /// - Dapper handles byte[] automatically
    /// - Good for small-medium files (&lt;1MB)
    /// - For large files, consider file system + path reference
    /// </remarks>
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

    /// <summary>
    /// Retrieves image data for a part in the requested size.
    /// </summary>
    /// <remarks>
    /// 📖 SWITCH EXPRESSION (C# 8+):
    /// - Concise pattern matching syntax
    /// - Each arm: pattern => result
    /// - '_' is the default/fallback case
    ///
    /// Equivalent to:
    ///   string column;
    ///   switch (size)
    ///   {
    ///       case ImageSize.Full: column = "image_full"; break;
    ///       case ImageSize.Thumbnail: column = "image_thumbnail"; break;
    ///       default: column = "image_thumbnail"; break;
    ///   }
    ///
    /// 📖 STRING INTERPOLATION IN SQL:
    /// - $"SELECT {column}" inserts the column name
    /// - SAFE here because 'column' comes from switch, not user input
    /// - NEVER interpolate user input into SQL (SQL injection!)
    /// </remarks>
    public async Task<PartImage?> GetImageAsync(int partId, ImageSize size)
    {
        // Select which column based on requested size
        var column = size switch
        {
            ImageSize.Full => "image_full",
            ImageSize.Thumbnail => "image_thumbnail",
            ImageSize.Embedded => "image_embedded",
            _ => "image_thumbnail"  // Default fallback
        };

        // 📖 QueryFirstOrDefaultAsync: Returns first result or null
        var sql = $"SELECT id, part_id, {column} as ImageFull, mime_type FROM part_images WHERE part_id = @PartId AND is_primary = 1";
        return await _connection.QueryFirstOrDefaultAsync<PartImage>(sql, new { PartId = partId });
    }

    /// <summary>
    /// Retrieves metadata for all images of a part (without binary data).
    /// </summary>
    /// <remarks>
    /// 📖 SELECTIVE COLUMNS:
    /// - We don't SELECT image_full, image_thumbnail, etc.
    /// - This avoids loading large binary data unnecessarily
    /// - Performance optimization for listing images
    /// </remarks>
    public async Task<IEnumerable<PartImage>> GetPartImagesAsync(int partId)
    {
        return await _connection.QueryAsync<PartImage>(
            "SELECT id, part_id, original_filename, mime_type, width, height, file_size, is_primary, created_at FROM part_images WHERE part_id = @PartId",
            new { PartId = partId });
    }

    #endregion

    // =========================================================================
    // LOW STOCK REGION - Business Queries
    // =========================================================================
    #region Low Stock

    /// <summary>
    /// Retrieves parts that are at or below their minimum quantity threshold.
    /// </summary>
    /// <remarks>
    /// 📖 BUSINESS LOGIC IN SQL:
    /// - The WHERE clause (quantity &lt;= min_quantity) is business logic
    /// - Putting it in SQL is more efficient than filtering in C#
    /// - Database can use indexes for fast filtering
    /// - Only matching rows are sent over the wire
    /// </remarks>
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

    // =========================================================================
    // IDisposable IMPLEMENTATION
    // =========================================================================
    /// <summary>
    /// Releases database connection resources.
    /// </summary>
    /// <remarks>
    /// 📖 DISPOSE PATTERN:
    /// 1. Check if already disposed (prevent double-dispose)
    /// 2. Dispose managed resources (the connection)
    /// 3. Mark as disposed
    /// 4. Suppress finalizer (optimization)
    ///
    /// 📖 GC.SuppressFinalize:
    /// - Tells garbage collector: "Don't run my finalizer"
    /// - Optimization when Dispose() is called manually
    /// - Finalizers are slow; avoiding them is good
    ///
    /// 📖 WHY CHECK _disposed?
    /// - Calling Dispose twice is allowed but should do nothing
    /// - Prevents errors from disposing already-disposed connection
    ///
    /// 📚 Full pattern: https://learn.microsoft.com/en-us/dotnet/standard/garbage-collection/implementing-dispose
    /// </remarks>
    public void Dispose()
    {
        if (!_disposed)
        {
            _connection.Dispose();  // Close and release the database connection
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}

// =============================================================================
// 📖 EXTENSION IDEAS FOR LEARNING
// =============================================================================
/*
1. ADD CONNECTION POOLING:
   For high-traffic scenarios, reuse connections:

   private static readonly ConcurrentDictionary<string, SqliteConnection> _pool = new();

   public static PartsDbContext GetFromPool(string path)
   {
       return _pool.GetOrAdd(path, p => new PartsDbContext(p));
   }

2. ADD RETRY LOGIC:
   Handle transient failures (e.g., database locked):

   public async Task<T> ExecuteWithRetryAsync<T>(Func<Task<T>> operation, int maxRetries = 3)
   {
       for (int i = 0; i < maxRetries; i++)
       {
           try { return await operation(); }
           catch (SqliteException ex) when (ex.SqliteErrorCode == 5) // SQLITE_BUSY
           {
               await Task.Delay(100 * (i + 1));
           }
       }
       throw new Exception("Max retries exceeded");
   }

3. ADD QUERY LOGGING:
   Log all SQL queries for debugging:

   public class LoggingConnection : IDbConnection
   {
       private readonly IDbConnection _inner;
       private readonly ILogger _logger;

       public IDbCommand CreateCommand()
       {
           var cmd = _inner.CreateCommand();
           return new LoggingCommand(cmd, _logger);
       }
   }

4. ADD UNIT OF WORK PATTERN:
   Coordinate multiple operations:

   public class UnitOfWork : IDisposable
   {
       public PartsDbContext Parts { get; }
       public CategoriesDbContext Categories { get; }
       private IDbTransaction? _transaction;

       public void BeginTransaction() => _transaction = Parts.Connection.BeginTransaction();
       public void Commit() => _transaction?.Commit();
       public void Rollback() => _transaction?.Rollback();
   }
*/
