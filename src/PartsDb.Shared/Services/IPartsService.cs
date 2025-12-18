// =============================================================================
// INTERFACES - Contracts that define WHAT a class can do
// =============================================================================
// An interface is like a contract or a promise. It defines method signatures
// but NOT implementations. Classes that "implement" an interface MUST provide
// implementations for all defined methods.
//
// 📚 Learn more: https://learn.microsoft.com/en-us/dotnet/csharp/fundamentals/types/interfaces
// =============================================================================

using PartsDb.Shared.Models;

namespace PartsDb.Shared.Services;

// =============================================================================
// IPartsService INTERFACE
// =============================================================================
/// <summary>
/// Defines the contract for parts service operations.
/// Any class implementing this interface MUST provide these methods.
/// </summary>
/// <remarks>
/// 📖 WHY USE INTERFACES?
///
/// 1. ABSTRACTION: Hide implementation details
///    - Code depends on interface, not concrete class
///    - Can change implementation without affecting callers
///
/// 2. DEPENDENCY INJECTION: Easy to swap implementations
///    - Production: services.AddSingleton&lt;IPartsService, PartsService&gt;();
///    - Testing:    services.AddSingleton&lt;IPartsService, MockPartsService&gt;();
///
/// 3. TESTABILITY: Create mock/fake implementations for unit tests
///    - No database needed for testing business logic
///
/// 4. MULTIPLE IMPLEMENTATIONS: Different implementations for different scenarios
///    - PartsService: Uses SQLite
///    - ApiPartsService: Calls REST API
///    - CachedPartsService: Adds caching layer
///
/// 📚 Book: "Dependency Injection Principles, Practices, and Patterns"
///          by Steven van Deursen &amp; Mark Seemann
/// </remarks>
public interface IPartsService
{
    // -------------------------------------------------------------------------
    // READ OPERATIONS (Queries)
    // -------------------------------------------------------------------------

    /// <summary>
    /// Retrieves a paginated list of parts.
    /// </summary>
    /// <param name="limit">Maximum number of parts to return (default: 100)</param>
    /// <param name="offset">Number of parts to skip for pagination (default: 0)</param>
    /// <returns>A collection of parts.</returns>
    /// <remarks>
    /// 📖 RETURN TYPE BREAKDOWN:
    /// - Task&lt;T&gt;: This method is async and returns T when complete
    /// - IEnumerable&lt;Part&gt;: A sequence of Part objects (can be iterated with foreach)
    ///
    /// 📖 DEFAULT PARAMETERS:
    /// - 'limit = 100' means if you don't pass a value, 100 is used
    /// - Call as: GetPartsAsync() or GetPartsAsync(50) or GetPartsAsync(50, 10)
    /// </remarks>
    Task<IEnumerable<Part>> GetPartsAsync(int limit = 100, int offset = 0);

    /// <summary>
    /// Retrieves a single part by its ID.
    /// </summary>
    /// <param name="id">The unique identifier of the part.</param>
    /// <returns>The part if found; otherwise, null.</returns>
    /// <remarks>
    /// 📖 NULLABLE RETURN TYPE:
    /// - Task&lt;Part?&gt; means the result can be null (part not found)
    /// - Caller MUST handle the null case:
    ///
    ///   var part = await service.GetPartByIdAsync(123);
    ///   if (part is null)
    ///   {
    ///       // Handle not found
    ///   }
    /// </remarks>
    Task<Part?> GetPartByIdAsync(int id);

    /// <summary>
    /// Retrieves a part by its part number.
    /// </summary>
    Task<Part?> GetPartByNumberAsync(string partNumber);

    /// <summary>
    /// Searches for parts matching the query string.
    /// </summary>
    /// <param name="query">Search term to match against part number, name, etc.</param>
    /// <remarks>
    /// 📖 SEARCH BEHAVIOR:
    /// - Empty/null query might return all parts or empty result (implementation decides)
    /// - Implementation uses full-text search (FTS5 in SQLite)
    /// </remarks>
    Task<IEnumerable<Part>> SearchPartsAsync(string query);

    /// <summary>
    /// Gets all parts where quantity is at or below minimum threshold.
    /// </summary>
    Task<IEnumerable<Part>> GetLowStockPartsAsync();

    /// <summary>
    /// Gets all categories.
    /// </summary>
    Task<IEnumerable<Category>> GetCategoriesAsync();

    // -------------------------------------------------------------------------
    // WRITE OPERATIONS (Commands)
    // -------------------------------------------------------------------------

    /// <summary>
    /// Creates a new part in the database.
    /// </summary>
    /// <param name="part">The part to create.</param>
    /// <returns>The ID of the newly created part.</returns>
    /// <remarks>
    /// 📖 COMMAND vs QUERY:
    /// - Queries: Read data, don't change state (Get*, Search*, etc.)
    /// - Commands: Change state (Create*, Update*, Delete*)
    ///
    /// This follows the CQRS pattern (Command Query Responsibility Segregation).
    /// </remarks>
    Task<int> CreatePartAsync(Part part);

    /// <summary>
    /// Updates an existing part.
    /// </summary>
    /// <remarks>
    /// 📖 Task WITHOUT GENERIC:
    /// - Task (not Task&lt;T&gt;) means async method with no return value
    /// - Equivalent to 'async void' but safer (can be awaited, exceptions propagate)
    /// - NEVER use 'async void' except for event handlers!
    /// </remarks>
    Task UpdatePartAsync(Part part);

    /// <summary>
    /// Deletes a part by ID.
    /// </summary>
    /// <remarks>
    /// Note: Implementation may use "soft delete" (set IsActive = false)
    /// instead of actually removing the record.
    /// </remarks>
    Task DeletePartAsync(int id);

    /// <summary>
    /// Adjusts the quantity of a part and logs the transaction.
    /// </summary>
    /// <param name="partId">The part to adjust.</param>
    /// <param name="change">Amount to change (positive = add, negative = remove).</param>
    /// <param name="type">Type of transaction for audit trail.</param>
    /// <param name="reference">Optional reference (PO number, work order, etc.).</param>
    /// <remarks>
    /// 📖 NULLABLE PARAMETER WITH DEFAULT:
    /// - 'string? reference = null' means:
    ///   - Parameter is nullable (can accept null)
    ///   - Default value is null if not provided
    /// - Caller can: UpdateQuantityAsync(1, 5, TransactionType.In)
    ///   or: UpdateQuantityAsync(1, 5, TransactionType.In, "PO-12345")
    /// </remarks>
    Task UpdateQuantityAsync(int partId, int change, TransactionType type, string? reference = null);

    /// <summary>
    /// Creates a new category.
    /// </summary>
    Task<int> CreateCategoryAsync(Category category);

    // -------------------------------------------------------------------------
    // IMAGE OPERATIONS
    // -------------------------------------------------------------------------

    /// <summary>
    /// Imports an image for a part, processing it into multiple formats.
    /// </summary>
    /// <param name="partId">The part to attach the image to.</param>
    /// <param name="imageStream">Stream containing the image data.</param>
    /// <param name="filename">Original filename for reference.</param>
    /// <param name="isPrimary">Whether this should be the main image.</param>
    /// <remarks>
    /// 📖 STREAM PARAMETER:
    /// - Stream is an abstract class for reading/writing bytes
    /// - Can be FileStream, MemoryStream, NetworkStream, etc.
    /// - Using Stream instead of byte[] allows processing large files
    ///   without loading everything into memory at once
    ///
    /// 📚 More on Streams: https://learn.microsoft.com/en-us/dotnet/api/system.io.stream
    /// </remarks>
    Task ImportImageAsync(int partId, Stream imageStream, string filename, bool isPrimary = false);

    /// <summary>
    /// Retrieves an image for a part in the specified size.
    /// </summary>
    /// <returns>Image data as byte array, or null if no image exists.</returns>
    Task<byte[]?> GetImageAsync(int partId, ImageSize size);
}

// =============================================================================
// 📖 EXTENSION IDEA: Create a mock implementation for testing
// =============================================================================
/*
public class MockPartsService : IPartsService
{
    private readonly List<Part> _parts = new();
    private int _nextId = 1;

    public Task<IEnumerable<Part>> GetPartsAsync(int limit = 100, int offset = 0)
    {
        var result = _parts.Skip(offset).Take(limit);
        return Task.FromResult(result);
    }

    public Task<Part?> GetPartByIdAsync(int id)
    {
        var part = _parts.FirstOrDefault(p => p.Id == id);
        return Task.FromResult(part);
    }

    public Task<int> CreatePartAsync(Part part)
    {
        part.Id = _nextId++;
        _parts.Add(part);
        return Task.FromResult(part.Id);
    }

    // ... implement other methods
}
*/
