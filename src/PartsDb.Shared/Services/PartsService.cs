// =============================================================================
// SERVICE IMPLEMENTATION - Business Logic Layer
// =============================================================================
// Services contain your application's business logic. They coordinate between
// different parts of your application (data access, external services, etc.)
//
// This class IMPLEMENTS the IPartsService interface, providing concrete
// implementations for all the methods defined in the interface.
//
// 📚 Learn more: https://learn.microsoft.com/en-us/dotnet/architecture/modern-web-apps-azure/common-web-application-architectures
// =============================================================================

using PartsDb.Shared.Data;
using PartsDb.Shared.ImageProcessing;
using PartsDb.Shared.Models;

namespace PartsDb.Shared.Services;

// =============================================================================
// PartsService CLASS
// =============================================================================
/// <summary>
/// Implements the IPartsService interface, providing business logic
/// for part management operations.
/// </summary>
/// <remarks>
/// 📖 CLASS DECLARATION BREAKDOWN:
/// - 'public': Accessible from anywhere
/// - 'class': This is a reference type (vs struct which is a value type)
/// - 'PartsService': The class name
/// - ': IPartsService': This class implements the IPartsService interface
///
/// 📖 IMPLEMENTATION vs INHERITANCE:
/// - ':' can mean inheritance (from a class) or implementation (from an interface)
/// - A class can only inherit from ONE base class
/// - A class can implement MULTIPLE interfaces
/// - Example: public class MyClass : BaseClass, IInterface1, IInterface2
/// </remarks>
public class PartsService : IPartsService
{
    // -------------------------------------------------------------------------
    // PRIVATE FIELDS (Dependencies)
    // -------------------------------------------------------------------------
    // Fields store the state of the object. These are marked 'private' so they
    // can only be accessed within this class.

    /// <summary>
    /// Database context for data access.
    /// </summary>
    /// <remarks>
    /// 📖 NAMING CONVENTION:
    /// - Private fields often start with underscore: _fieldName
    /// - This distinguishes them from parameters and local variables
    /// - It's a convention, not a rule - but very common in C#
    ///
    /// 📖 READONLY MODIFIER:
    /// - 'readonly' means this field can only be set in the constructor
    /// - After construction, it cannot be changed
    /// - This ensures the dependency doesn't change during object lifetime
    /// - Makes the code more predictable and thread-safe
    /// </remarks>
    private readonly PartsDbContext _context;

    /// <summary>
    /// Image processor for handling image uploads.
    /// </summary>
    /// <remarks>
    /// 📖 DEPENDENCY INJECTION:
    /// - We depend on IImageProcessor (interface), not a concrete class
    /// - The actual implementation is "injected" through the constructor
    /// - This follows the Dependency Inversion Principle (DIP)
    /// </remarks>
    private readonly IImageProcessor _imageProcessor;

    // -------------------------------------------------------------------------
    // CONSTRUCTOR
    // -------------------------------------------------------------------------
    /// <summary>
    /// Creates a new instance of PartsService with required dependencies.
    /// </summary>
    /// <param name="context">Database context for data operations.</param>
    /// <param name="imageProcessor">Processor for image operations.</param>
    /// <remarks>
    /// 📖 CONSTRUCTOR:
    /// - Special method called when creating a new instance: new PartsService(...)
    /// - Same name as the class, no return type
    /// - Used for initialization and dependency injection
    ///
    /// 📖 DEPENDENCY INJECTION PATTERN:
    /// - Dependencies are passed IN rather than created inside
    /// - Makes the class easier to test (can pass mock dependencies)
    /// - Makes dependencies explicit and visible
    ///
    /// 📖 WHY NOT CREATE DEPENDENCIES INSIDE?
    /// BAD:
    ///   public PartsService()
    ///   {
    ///       _context = new PartsDbContext("hardcoded.db"); // Tightly coupled!
    ///   }
    ///
    /// GOOD:
    ///   public PartsService(PartsDbContext context) // Loosely coupled
    ///   {
    ///       _context = context;
    ///   }
    /// </remarks>
    public PartsService(PartsDbContext context, IImageProcessor imageProcessor)
    {
        // Store dependencies in fields for use throughout the class
        _context = context;
        _imageProcessor = imageProcessor;
    }

    // -------------------------------------------------------------------------
    // EXPRESSION-BODIED METHODS (Simple Delegation)
    // -------------------------------------------------------------------------
    // When a method simply calls another method and returns its result,
    // we can use the '=>' syntax for a more concise declaration.

    /// <summary>
    /// Gets a paginated list of parts.
    /// </summary>
    /// <remarks>
    /// 📖 EXPRESSION-BODIED METHOD:
    /// - The '=>' syntax is shorthand for a simple return statement
    /// - This method simply delegates to the context
    ///
    /// Equivalent to:
    ///   public Task&lt;IEnumerable&lt;Part&gt;&gt; GetPartsAsync(int limit = 100, int offset = 0)
    ///   {
    ///       return _context.GetPartsAsync(limit, offset);
    ///   }
    ///
    /// Note: No 'async' keyword needed when just returning another Task.
    /// Using 'async' unnecessarily adds overhead (creates a state machine).
    /// </remarks>
    public Task<IEnumerable<Part>> GetPartsAsync(int limit = 100, int offset = 0)
        => _context.GetPartsAsync(limit, offset);

    public Task<Part?> GetPartByIdAsync(int id)
        => _context.GetPartByIdAsync(id);

    public Task<Part?> GetPartByNumberAsync(string partNumber)
        => _context.GetPartByNumberAsync(partNumber);

    // -------------------------------------------------------------------------
    // METHODS WITH BUSINESS LOGIC
    // -------------------------------------------------------------------------
    // Some methods need to do more than just delegate - they contain
    // business logic (decisions, validations, transformations).

    /// <summary>
    /// Searches for parts, with empty query handling.
    /// </summary>
    /// <remarks>
    /// 📖 GUARD CLAUSE:
    /// - The 'if' at the beginning handles an edge case first
    /// - This pattern is called "early return" or "guard clause"
    /// - It keeps the main logic simple by handling special cases first
    ///
    /// 📖 string.IsNullOrWhiteSpace:
    /// - Returns true if string is null, empty "", or only whitespace "   "
    /// - Safer than checking just for null or empty
    ///
    /// Alternative without guard clause (harder to read):
    ///   return string.IsNullOrWhiteSpace(query)
    ///       ? _context.GetPartsAsync()
    ///       : _context.SearchPartsAsync(query);
    /// </remarks>
    public Task<IEnumerable<Part>> SearchPartsAsync(string query)
    {
        // Guard clause: handle empty/null query
        if (string.IsNullOrWhiteSpace(query))
            return _context.GetPartsAsync();

        // Main logic: perform the search
        return _context.SearchPartsAsync(query);
    }

    public Task<int> CreatePartAsync(Part part)
        => _context.CreatePartAsync(part);

    public Task UpdatePartAsync(Part part)
        => _context.UpdatePartAsync(part);

    public Task DeletePartAsync(int id)
        => _context.DeletePartAsync(id);

    public Task UpdateQuantityAsync(int partId, int change, TransactionType type, string? reference = null)
        => _context.UpdateQuantityAsync(partId, change, type, reference);

    public Task<IEnumerable<Part>> GetLowStockPartsAsync()
        => _context.GetLowStockPartsAsync();

    public Task<IEnumerable<Category>> GetCategoriesAsync()
        => _context.GetCategoriesAsync();

    public Task<int> CreateCategoryAsync(Category category)
        => _context.CreateCategoryAsync(category);

    // -------------------------------------------------------------------------
    // ASYNC METHODS (With actual async work)
    // -------------------------------------------------------------------------
    // These methods use 'async' and 'await' because they need to do
    // multiple async operations or process results between calls.

    /// <summary>
    /// Imports and processes an image for a part.
    /// </summary>
    /// <remarks>
    /// 📖 ASYNC/AWAIT EXPLAINED:
    ///
    /// 1. 'async' keyword on method: Enables use of 'await' inside
    ///
    /// 2. 'await' keyword: Pauses execution until the Task completes
    ///    BUT doesn't block the thread - thread can do other work
    ///
    /// 3. What happens step by step:
    ///    a. Call ProcessImageAsync - starts async operation
    ///    b. 'await' - method pauses, thread is freed for other work
    ///    c. When processing completes, method resumes
    ///    d. Create PartImage object (synchronous)
    ///    e. Call SaveImageAsync - another async operation
    ///    f. 'await' - method pauses again
    ///    g. When save completes, method finishes
    ///
    /// 📖 WHY ASYNC?
    /// - Keeps UI responsive during long operations
    /// - Allows server to handle more requests
    /// - No thread is blocked waiting
    ///
    /// 📚 Deep dive: https://learn.microsoft.com/en-us/dotnet/csharp/asynchronous-programming/
    /// 📚 Book: "Concurrency in C# Cookbook" by Stephen Cleary
    /// </remarks>
    public async Task ImportImageAsync(int partId, Stream imageStream, string filename, bool isPrimary = false)
    {
        // Step 1: Process the image (resize, compress, create thumbnails)
        // This is an I/O-bound operation - 'await' frees the thread
        var processedImage = await _imageProcessor.ProcessImageAsync(imageStream);

        // Step 2: Create entity object using object initializer syntax
        // 📖 OBJECT INITIALIZER:
        // - Creates object and sets properties in one statement
        // - Cleaner than setting each property separately
        // - Properties not listed get default values
        var partImage = new PartImage
        {
            PartId = partId,
            ImageFull = processedImage.FullImage,
            ImageThumbnail = processedImage.Thumbnail,
            ImageEmbedded = processedImage.EmbeddedImage,
            OriginalFilename = filename,
            MimeType = "image/webp",
            Width = processedImage.Width,
            Height = processedImage.Height,
            // 📖 NULL-COALESCING OPERATOR:
            // - '??' returns left side if not null, otherwise right side
            // - processedImage.FullImage?.Length: null if FullImage is null
            // - ?? 0: if null, use 0 instead
            FileSize = processedImage.FullImage?.Length ?? 0,
            IsPrimary = isPrimary
        };

        // Step 3: Save to database
        await _context.SaveImageAsync(partImage);
    }

    /// <summary>
    /// Retrieves an image for a part in the requested size.
    /// </summary>
    /// <remarks>
    /// 📖 NULL-CONDITIONAL OPERATOR '?.':
    /// - 'image?.ImageFull' means: if image is null, return null; otherwise return ImageFull
    /// - Prevents NullReferenceException
    /// - Short-circuit evaluation: if left side is null, right side is not evaluated
    ///
    /// Without null-conditional (verbose):
    ///   if (image == null)
    ///       return null;
    ///   return image.ImageFull;
    ///
    /// With null-conditional (concise):
    ///   return image?.ImageFull;
    /// </remarks>
    public async Task<byte[]?> GetImageAsync(int partId, ImageSize size)
    {
        var image = await _context.GetImageAsync(partId, size);
        return image?.ImageFull;
    }
}

// =============================================================================
// 📖 EXTENSION IDEAS FOR LEARNING
// =============================================================================
/*
1. ADD VALIDATION:
   Create a PartValidator class and use it in CreatePartAsync:

   public async Task<int> CreatePartAsync(Part part)
   {
       var validation = _validator.Validate(part);
       if (!validation.IsValid)
           throw new ValidationException(validation.Errors);

       return await _context.CreatePartAsync(part);
   }

2. ADD CACHING:
   Wrap frequently accessed data in memory cache:

   public async Task<IEnumerable<Category>> GetCategoriesAsync()
   {
       return await _cache.GetOrCreateAsync("categories", async entry =>
       {
           entry.SlidingExpiration = TimeSpan.FromMinutes(10);
           return await _context.GetCategoriesAsync();
       });
   }

3. ADD LOGGING:
   Inject ILogger and log operations:

   public async Task<int> CreatePartAsync(Part part)
   {
       _logger.LogInformation("Creating part {PartNumber}", part.PartNumber);
       try
       {
           var id = await _context.CreatePartAsync(part);
           _logger.LogInformation("Created part {Id}", id);
           return id;
       }
       catch (Exception ex)
       {
           _logger.LogError(ex, "Failed to create part {PartNumber}", part.PartNumber);
           throw;
       }
   }

4. ADD EVENTS:
   Raise events when parts change:

   public event EventHandler<Part>? PartCreated;

   public async Task<int> CreatePartAsync(Part part)
   {
       var id = await _context.CreatePartAsync(part);
       part.Id = id;
       PartCreated?.Invoke(this, part);
       return id;
   }
*/
