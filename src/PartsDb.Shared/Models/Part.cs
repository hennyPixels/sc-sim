// =============================================================================
// MODELS - Data Transfer Objects (DTOs) and Entity Classes
// =============================================================================
// Models define the SHAPE of your data. They're like blueprints that describe
// what properties an object has. In this project, models map to database tables.
//
// 📚 Learn more: https://learn.microsoft.com/en-us/dotnet/csharp/fundamentals/object-oriented/
// =============================================================================

namespace PartsDb.Shared.Models;

// =============================================================================
// PART CLASS - The main entity in our inventory system
// =============================================================================
/// <summary>
/// Represents a part in the inventory system.
/// This is an "entity" class - it maps directly to the 'parts' table in the database.
/// </summary>
/// <remarks>
/// 📖 LEARNING POINTS:
/// - Classes group related data (properties) and behavior (methods) together
/// - Properties use { get; set; } syntax (auto-implemented properties)
/// - The '?' after a type makes it nullable (can be null)
/// - Default values are set with '= value'
/// </remarks>
public class Part
{
    // -------------------------------------------------------------------------
    // PRIMARY KEY
    // -------------------------------------------------------------------------
    // Every database record needs a unique identifier.
    // By convention, a property named 'Id' or '[ClassName]Id' is the primary key.
    public int Id { get; set; }

    // -------------------------------------------------------------------------
    // REQUIRED PROPERTIES (non-nullable with defaults)
    // -------------------------------------------------------------------------
    // These properties MUST have a value. We use 'string.Empty' as default
    // to avoid null reference exceptions. The database schema enforces NOT NULL.

    /// <summary>
    /// Unique identifier for the part (e.g., "RES-100K-0805").
    /// </summary>
    /// <remarks>
    /// 📖 string.Empty vs "": Both work, but string.Empty is more explicit
    /// and can be slightly more efficient (reuses the same empty string instance).
    /// </remarks>
    public string PartNumber { get; set; } = string.Empty;

    /// <summary>
    /// Human-readable name for the part.
    /// </summary>
    public string Name { get; set; } = string.Empty;

    /// <summary>
    /// Currency code for pricing (ISO 4217 format).
    /// </summary>
    /// <remarks>
    /// 📖 Default values are set at object creation time.
    /// You can override them: new Part { Currency = "EUR" }
    /// </remarks>
    public string Currency { get; set; } = "USD";

    /// <summary>
    /// Whether this part is active in the system.
    /// </summary>
    /// <remarks>
    /// 📖 We use "soft delete" - instead of removing records, we set IsActive = false.
    /// This preserves history and relationships.
    /// </remarks>
    public bool IsActive { get; set; } = true;

    // -------------------------------------------------------------------------
    // OPTIONAL PROPERTIES (nullable)
    // -------------------------------------------------------------------------
    // The '?' makes these nullable - they can be null without causing errors.
    // In C# 10+, with <Nullable>enable</Nullable>, the compiler warns about
    // potential null reference issues.

    /// <summary>
    /// Detailed description of the part.
    /// </summary>
    /// <remarks>
    /// 📖 NULLABLE TYPES:
    /// - string? means this can be null
    /// - Use null-conditional operator: description?.Length
    /// - Use null-coalescing: description ?? "No description"
    /// </remarks>
    public string? Description { get; set; }

    /// <summary>
    /// Foreign key to the Category table.
    /// </summary>
    /// <remarks>
    /// 📖 NULLABLE VALUE TYPES:
    /// - int? is shorthand for Nullable<int>
    /// - Value types (int, bool, DateTime) normally can't be null
    /// - Adding '?' wraps them in Nullable<T>, allowing null
    /// - Check with: if (CategoryId.HasValue) or if (CategoryId is not null)
    /// - Access value with: CategoryId.Value (throws if null!) or CategoryId ?? 0
    /// </remarks>
    public int? CategoryId { get; set; }

    public string? Manufacturer { get; set; }
    public string? Location { get; set; }
    public string? DatasheetUrl { get; set; }
    public string? Notes { get; set; }

    // -------------------------------------------------------------------------
    // NUMERIC PROPERTIES
    // -------------------------------------------------------------------------

    /// <summary>
    /// Current quantity in stock.
    /// </summary>
    /// <remarks>
    /// 📖 Value types (int, bool, decimal) have default values:
    /// - int defaults to 0
    /// - bool defaults to false
    /// - decimal defaults to 0m
    /// </remarks>
    public int Quantity { get; set; }

    /// <summary>
    /// Minimum quantity threshold for low stock alerts.
    /// </summary>
    public int MinQuantity { get; set; }

    /// <summary>
    /// Unit price of the part.
    /// </summary>
    /// <remarks>
    /// 📖 WHY DECIMAL FOR MONEY?
    /// - decimal is 128-bit with ~28 significant digits
    /// - double is 64-bit with ~15 significant digits
    /// - double can have rounding errors: 0.1 + 0.2 != 0.3
    /// - decimal is exact for financial calculations
    /// - Use 'm' suffix for decimal literals: 19.99m
    /// </remarks>
    public decimal? UnitPrice { get; set; }

    // -------------------------------------------------------------------------
    // TIMESTAMP PROPERTIES
    // -------------------------------------------------------------------------

    /// <summary>
    /// When this record was created.
    /// </summary>
    /// <remarks>
    /// 📖 DateTime stores both date and time.
    /// Consider using DateTimeOffset for timezone-aware timestamps.
    /// </remarks>
    public DateTime CreatedAt { get; set; }

    /// <summary>
    /// When this record was last updated.
    /// </summary>
    public DateTime UpdatedAt { get; set; }

    // -------------------------------------------------------------------------
    // NAVIGATION PROPERTIES (Relationships)
    // -------------------------------------------------------------------------
    // Navigation properties represent relationships between entities.
    // They allow you to "navigate" from one entity to related entities.

    /// <summary>
    /// The category this part belongs to.
    /// </summary>
    /// <remarks>
    /// 📖 NAVIGATION PROPERTIES:
    /// - This is a "reference navigation" (one-to-one or many-to-one)
    /// - CategoryId is the "foreign key"
    /// - Category is the "navigation property"
    /// - ORMs like Entity Framework can automatically load related data
    /// </remarks>
    public Category? Category { get; set; }

    /// <summary>
    /// Images associated with this part.
    /// </summary>
    /// <remarks>
    /// 📖 COLLECTION NAVIGATION:
    /// - This is a "collection navigation" (one-to-many)
    /// - One Part can have many PartImages
    /// - Initialize with empty list to avoid null reference exceptions
    /// - ICollection<T> is an interface - we use List<T> as the implementation
    /// </remarks>
    public ICollection<PartImage> Images { get; set; } = new List<PartImage>();

    // -------------------------------------------------------------------------
    // COMPUTED PROPERTY (Expression-Bodied)
    // -------------------------------------------------------------------------

    /// <summary>
    /// Returns true if current stock is at or below minimum threshold.
    /// </summary>
    /// <remarks>
    /// 📖 EXPRESSION-BODIED MEMBERS:
    /// - The '=>' syntax is shorthand for a getter-only property
    /// - Equivalent to: public bool IsLowStock { get { return Quantity <= MinQuantity; } }
    /// - The value is computed each time the property is accessed (not stored)
    /// - No 'set' accessor, so this is read-only
    /// </remarks>
    public bool IsLowStock => Quantity <= MinQuantity;
}

// =============================================================================
// CATEGORY CLASS - Hierarchical organization of parts
// =============================================================================
/// <summary>
/// Represents a category for organizing parts (e.g., "Resistors", "Capacitors").
/// Categories can be nested (parent-child relationships).
/// </summary>
/// <remarks>
/// 📖 SELF-REFERENCING RELATIONSHIPS:
/// A Category can have a Parent category and multiple Child categories.
/// This creates a tree structure (hierarchy).
/// </remarks>
public class Category
{
    public int Id { get; set; }
    public string Name { get; set; } = string.Empty;
    public string? Description { get; set; }

    /// <summary>
    /// Foreign key to parent category (null for top-level categories).
    /// </summary>
    public int? ParentId { get; set; }

    // Navigation properties for hierarchy
    public Category? Parent { get; set; }
    public ICollection<Category> Children { get; set; } = new List<Category>();

    /// <summary>
    /// All parts in this category.
    /// </summary>
    /// <remarks>
    /// 📖 INVERSE NAVIGATION:
    /// This is the "inverse" of Part.Category.
    /// Part has CategoryId (FK) and Category (navigation).
    /// Category has Parts (inverse navigation).
    /// </remarks>
    public ICollection<Part> Parts { get; set; } = new List<Part>();
}

// =============================================================================
// PARTIMAGE CLASS - Binary image data storage
// =============================================================================
/// <summary>
/// Stores image data for a part in multiple sizes/formats.
/// </summary>
/// <remarks>
/// 📖 BLOB STORAGE:
/// Binary data (byte[]) is stored as BLOB in SQLite.
/// We store multiple versions for different use cases:
/// - Full: High quality for detailed view
/// - Thumbnail: Small preview
/// - Embedded: Tiny, compressed for embedded systems
/// </remarks>
public class PartImage
{
    public int Id { get; set; }

    /// <summary>
    /// Foreign key linking to the Part.
    /// </summary>
    public int PartId { get; set; }

    /// <summary>
    /// Full-resolution image data (WebP format).
    /// </summary>
    /// <remarks>
    /// 📖 BYTE ARRAYS:
    /// - byte[] stores raw binary data
    /// - Used for files, images, encrypted data, etc.
    /// - Nullable because we might not have an image yet
    /// </remarks>
    public byte[]? ImageFull { get; set; }

    public byte[]? ImageThumbnail { get; set; }
    public byte[]? ImageEmbedded { get; set; }
    public string? OriginalFilename { get; set; }
    public string MimeType { get; set; } = "image/webp";
    public int? Width { get; set; }
    public int? Height { get; set; }
    public int? FileSize { get; set; }
    public bool IsPrimary { get; set; }
    public DateTime CreatedAt { get; set; }

    // Navigation back to Part
    public Part? Part { get; set; }
}

// =============================================================================
// SUPPLIER CLASS - Vendor information
// =============================================================================
/// <summary>
/// Represents a supplier/vendor who provides parts.
/// </summary>
public class Supplier
{
    public int Id { get; set; }
    public string Name { get; set; } = string.Empty;
    public string? ContactName { get; set; }
    public string? Email { get; set; }
    public string? Phone { get; set; }
    public string? Website { get; set; }
    public string? Address { get; set; }
    public string? Notes { get; set; }
    public DateTime CreatedAt { get; set; }
}

// =============================================================================
// PARTSUPPLIER CLASS - Many-to-Many Junction Table
// =============================================================================
/// <summary>
/// Links parts to suppliers with pricing information.
/// This is a "junction table" for a many-to-many relationship.
/// </summary>
/// <remarks>
/// 📖 MANY-TO-MANY RELATIONSHIPS:
/// - One Part can have multiple Suppliers
/// - One Supplier can supply multiple Parts
/// - This requires a junction/bridge table
/// - The junction table can have its own properties (price, lead time, etc.)
/// </remarks>
public class PartSupplier
{
    public int Id { get; set; }

    // Composite foreign key to Part
    public int PartId { get; set; }

    // Composite foreign key to Supplier
    public int SupplierId { get; set; }

    /// <summary>
    /// The supplier's own part number (may differ from our PartNumber).
    /// </summary>
    public string? SupplierPartNumber { get; set; }

    /// <summary>
    /// Price from this specific supplier.
    /// </summary>
    public decimal? UnitPrice { get; set; }

    public int MinOrderQty { get; set; } = 1;
    public int? LeadTimeDays { get; set; }
    public bool IsPreferred { get; set; }

    // Navigation properties
    public Part? Part { get; set; }
    public Supplier? Supplier { get; set; }
}

// =============================================================================
// INVENTORYTRANSACTION CLASS - Audit Trail
// =============================================================================
/// <summary>
/// Records every change to inventory quantities.
/// </summary>
/// <remarks>
/// 📖 AUDIT TRAIL PATTERN:
/// Instead of just updating Quantity, we log every change.
/// This provides:
/// - History of all changes
/// - Ability to trace issues
/// - Data for analytics
/// </remarks>
public class InventoryTransaction
{
    public int Id { get; set; }
    public int PartId { get; set; }

    /// <summary>
    /// The change amount (positive for additions, negative for removals).
    /// </summary>
    public int QuantityChange { get; set; }

    /// <summary>
    /// Type of transaction (In, Out, Adjust, Count).
    /// </summary>
    public TransactionType TransactionType { get; set; }

    /// <summary>
    /// Reference number (e.g., PO number, work order).
    /// </summary>
    public string? Reference { get; set; }

    public string? Notes { get; set; }
    public DateTime CreatedAt { get; set; }
    public string? CreatedBy { get; set; }

    // Navigation
    public Part? Part { get; set; }
}

// =============================================================================
// ENUMS - Named Constants
// =============================================================================
/// <summary>
/// Types of inventory transactions.
/// </summary>
/// <remarks>
/// 📖 ENUMS (Enumerations):
/// - Define a set of named constants
/// - Type-safe alternative to "magic strings" or numbers
/// - Underlying type is int by default (In=0, Out=1, etc.)
/// - Can specify values: In = 1, Out = 2
///
/// Usage:
///   var type = TransactionType.In;
///   if (transaction.TransactionType == TransactionType.Out) { ... }
///
/// Convert to string: TransactionType.In.ToString() => "In"
/// Parse from string: Enum.Parse<TransactionType>("In")
/// </remarks>
public enum TransactionType
{
    /// <summary>Stock received/added.</summary>
    In,

    /// <summary>Stock issued/removed.</summary>
    Out,

    /// <summary>Manual adjustment (correction).</summary>
    Adjust,

    /// <summary>Physical inventory count.</summary>
    Count
}

/// <summary>
/// Image size options for retrieval.
/// </summary>
public enum ImageSize
{
    /// <summary>Full resolution image.</summary>
    Full,

    /// <summary>Small thumbnail for lists.</summary>
    Thumbnail,

    /// <summary>Tiny compressed image for embedded systems.</summary>
    Embedded
}
