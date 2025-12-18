namespace PartsDb.Shared.Models;

public class Part
{
    public int Id { get; set; }
    public string PartNumber { get; set; } = string.Empty;
    public string Name { get; set; } = string.Empty;
    public string? Description { get; set; }
    public int? CategoryId { get; set; }
    public string? Manufacturer { get; set; }
    public int Quantity { get; set; }
    public int MinQuantity { get; set; }
    public decimal? UnitPrice { get; set; }
    public string Currency { get; set; } = "USD";
    public string? Location { get; set; }
    public string? DatasheetUrl { get; set; }
    public string? Notes { get; set; }
    public bool IsActive { get; set; } = true;
    public DateTime CreatedAt { get; set; }
    public DateTime UpdatedAt { get; set; }

    // Navigation properties
    public Category? Category { get; set; }
    public ICollection<PartImage> Images { get; set; } = new List<PartImage>();

    public bool IsLowStock => Quantity <= MinQuantity;
}

public class Category
{
    public int Id { get; set; }
    public string Name { get; set; } = string.Empty;
    public string? Description { get; set; }
    public int? ParentId { get; set; }

    // Navigation
    public Category? Parent { get; set; }
    public ICollection<Category> Children { get; set; } = new List<Category>();
    public ICollection<Part> Parts { get; set; } = new List<Part>();
}

public class PartImage
{
    public int Id { get; set; }
    public int PartId { get; set; }
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

    // Navigation
    public Part? Part { get; set; }
}

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

public class PartSupplier
{
    public int Id { get; set; }
    public int PartId { get; set; }
    public int SupplierId { get; set; }
    public string? SupplierPartNumber { get; set; }
    public decimal? UnitPrice { get; set; }
    public int MinOrderQty { get; set; } = 1;
    public int? LeadTimeDays { get; set; }
    public bool IsPreferred { get; set; }

    // Navigation
    public Part? Part { get; set; }
    public Supplier? Supplier { get; set; }
}

public class InventoryTransaction
{
    public int Id { get; set; }
    public int PartId { get; set; }
    public int QuantityChange { get; set; }
    public TransactionType TransactionType { get; set; }
    public string? Reference { get; set; }
    public string? Notes { get; set; }
    public DateTime CreatedAt { get; set; }
    public string? CreatedBy { get; set; }

    // Navigation
    public Part? Part { get; set; }
}

public enum TransactionType
{
    In,
    Out,
    Adjust,
    Count
}

public enum ImageSize
{
    Full,
    Thumbnail,
    Embedded
}
