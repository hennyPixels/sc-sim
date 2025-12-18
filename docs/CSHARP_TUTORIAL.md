# C# and .NET Learning Tutorial

Welcome! This tutorial will guide you through the PartsDB codebase, explaining C# and .NET concepts as we explore each component.

## Table of Contents

1. [Getting Started with C#](#1-getting-started-with-c)
2. [Project Structure](#2-project-structure)
3. [Understanding the Code](#3-understanding-the-code)
4. [Blazor and UI](#4-blazor-and-ui)
5. [Exercises](#5-exercises)
6. [Learning Resources](#6-learning-resources)

---

## 1. Getting Started with C#

### What is C#?

C# (pronounced "C-sharp") is a modern, object-oriented programming language developed by Microsoft. It runs on the **.NET runtime**, which provides:

- **Garbage collection** - Automatic memory management
- **Type safety** - Catches errors at compile time
- **Cross-platform support** - Runs on Windows, Linux, macOS

### Your First C# Concepts

```csharp
// This is a single-line comment

/*
   This is a
   multi-line comment
*/

// Variables and Types
int count = 10;              // Integer (whole number)
string name = "Resistor";    // Text
double price = 4.99;         // Decimal number
bool inStock = true;         // True/False
decimal money = 19.99m;      // Precise decimal (for money)

// Nullable types (can be null OR have a value)
int? maybeCount = null;      // The '?' makes it nullable
string? maybeName = null;    // Reference types are nullable by default in modern C#

// String interpolation (embedding variables in strings)
string message = $"Part {name} costs ${price}";
// Result: "Part Resistor costs $4.99"
```

### Classes and Objects

A **class** is a blueprint. An **object** is an instance of that blueprint.

```csharp
// This is a class definition
public class Part
{
    // Properties (data the class holds)
    public int Id { get; set; }
    public string Name { get; set; } = "";  // Default value
    public int Quantity { get; set; }

    // Method (behavior/action)
    public bool IsLowStock()
    {
        return Quantity < 10;
    }
}

// Creating an object (instance of the class)
Part resistor = new Part();
resistor.Name = "100Ω Resistor";
resistor.Quantity = 5;

// Or using object initializer syntax (preferred)
Part capacitor = new Part
{
    Name = "10µF Capacitor",
    Quantity = 25
};
```

### Key C# Keywords in This Project

| Keyword | Meaning | Example |
|---------|---------|---------|
| `public` | Accessible from anywhere | `public class Part` |
| `private` | Only accessible within the class | `private int _count;` |
| `async` | Method runs asynchronously | `async Task LoadData()` |
| `await` | Wait for async operation | `await db.GetPartsAsync()` |
| `using` | Import namespace OR dispose resource | `using System.IO;` |
| `namespace` | Organize code into groups | `namespace PartsDb.Models` |
| `interface` | Contract that classes must follow | `interface IPartsService` |
| `virtual` | Can be overridden in derived classes | `virtual void Save()` |
| `override` | Replaces base class implementation | `override void Save()` |

---

## 2. Project Structure

```
sc-sim/
├── src/
│   ├── PartsDb.Shared/          # 📚 Shared library (business logic)
│   │   ├── Models/              # Data structures
│   │   ├── Services/            # Business operations
│   │   ├── Data/                # Database access
│   │   └── ImageProcessing/     # Image handling
│   │
│   └── PartsDb.Maui/            # 🖥️ UI Application (Blazor Hybrid)
│       ├── Components/          # Reusable UI pieces
│       ├── Pages/               # Full page views
│       └── wwwroot/             # Static files (CSS, JS)
│
├── tests/
│   └── PartsDb.Tests/           # ✅ Unit tests
│
└── database/
    └── schema.sql               # 🗄️ Database structure
```

### Understanding .csproj Files

The `.csproj` file tells .NET how to build your project:

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <!-- Target framework: .NET 10 -->
    <TargetFramework>net10.0</TargetFramework>

    <!-- Enable modern C# features -->
    <ImplicitUsings>enable</ImplicitUsings>  <!-- Auto-import common namespaces -->
    <Nullable>enable</Nullable>               <!-- Null safety checks -->
    <LangVersion>latest</LangVersion>         <!-- Use newest C# syntax -->
  </PropertyGroup>

  <!-- External packages (like npm packages in JavaScript) -->
  <ItemGroup>
    <PackageReference Include="Dapper" Version="2.1.35" />
  </ItemGroup>
</Project>
```

---

## 3. Understanding the Code

### 3.1 Models (Data Structures)

**File: `src/PartsDb.Shared/Models/Part.cs`**

Models define the shape of your data. Think of them as templates.

```csharp
namespace PartsDb.Shared.Models;  // Namespace groups related code

public class Part
{
    // Auto-implemented properties
    // The compiler generates a hidden backing field
    public int Id { get; set; }

    // Property with default value
    public string PartNumber { get; set; } = string.Empty;

    // Nullable property (can be null)
    public string? Description { get; set; }

    // Nullable value type (int can normally never be null)
    public int? CategoryId { get; set; }

    // Expression-bodied property (computed on access)
    // The '=>' is a lambda expression
    public bool IsLowStock => Quantity <= MinQuantity;

    // Navigation property (relationship to another entity)
    public Category? Category { get; set; }

    // Collection navigation (one-to-many relationship)
    public ICollection<PartImage> Images { get; set; } = new List<PartImage>();
}
```

**Key Concepts:**

1. **Properties vs Fields**
   ```csharp
   // Field (avoid for public data)
   public int count;

   // Property (preferred - allows validation, change notification)
   public int Count { get; set; }

   // Property with validation
   private int _quantity;
   public int Quantity
   {
       get => _quantity;
       set => _quantity = value >= 0 ? value : 0;  // Never negative
   }
   ```

2. **Nullable Reference Types**
   ```csharp
   // With <Nullable>enable</Nullable> in .csproj:
   string name;        // Cannot be null, compiler warns if you try
   string? nickname;   // Can be null, you must check before using

   // Safe access with null-conditional operator
   int? length = nickname?.Length;  // Returns null if nickname is null

   // Null-coalescing operator
   string display = nickname ?? "No nickname";  // Use default if null
   ```

### 3.2 Interfaces (Contracts)

**File: `src/PartsDb.Shared/Services/IPartsService.cs`**

An interface defines WHAT a class must do, not HOW.

```csharp
namespace PartsDb.Shared.Services;

// Interface names conventionally start with 'I'
public interface IPartsService
{
    // Method signatures only - no implementation
    Task<IEnumerable<Part>> GetPartsAsync(int limit = 100, int offset = 0);
    Task<Part?> GetPartByIdAsync(int id);
    Task<int> CreatePartAsync(Part part);

    // Notice:
    // - Task<T> means this is async and returns T
    // - IEnumerable<Part> is a sequence of Parts
    // - Part? means it might return null (not found)
}
```

**Why use interfaces?**

1. **Dependency Injection** - Swap implementations easily
2. **Testing** - Create mock implementations for tests
3. **Abstraction** - Hide complex implementation details

```csharp
// In production: use real database
services.AddSingleton<IPartsService, PartsService>();

// In testing: use fake/mock service
services.AddSingleton<IPartsService, MockPartsService>();
```

### 3.3 Services (Business Logic)

**File: `src/PartsDb.Shared/Services/PartsService.cs`**

Services contain the application's business logic.

```csharp
namespace PartsDb.Shared.Services;

// This class IMPLEMENTS the interface
public class PartsService : IPartsService
{
    // Dependencies injected through constructor
    private readonly PartsDbContext _context;
    private readonly IImageProcessor _imageProcessor;

    // Constructor - called when creating an instance
    // 'readonly' means these can only be set in constructor
    public PartsService(PartsDbContext context, IImageProcessor imageProcessor)
    {
        _context = context;
        _imageProcessor = imageProcessor;
    }

    // Expression-bodied method (short syntax for simple methods)
    public Task<IEnumerable<Part>> GetPartsAsync(int limit = 100, int offset = 0)
        => _context.GetPartsAsync(limit, offset);

    // Full method syntax
    public Task<IEnumerable<Part>> SearchPartsAsync(string query)
    {
        // Guard clause - handle edge case first
        if (string.IsNullOrWhiteSpace(query))
            return _context.GetPartsAsync();

        return _context.SearchPartsAsync(query);
    }

    // Async method with actual async work
    public async Task ImportImageAsync(int partId, Stream imageStream,
                                       string filename, bool isPrimary = false)
    {
        // 'await' pauses here until ProcessImageAsync completes
        // BUT doesn't block the thread - it can do other work
        var processedImage = await _imageProcessor.ProcessImageAsync(imageStream);

        // Create new object with object initializer
        var partImage = new PartImage
        {
            PartId = partId,
            ImageFull = processedImage.FullImage,
            ImageThumbnail = processedImage.Thumbnail,
            // ... more properties
        };

        await _context.SaveImageAsync(partImage);
    }
}
```

**Understanding Async/Await:**

```csharp
// WRONG - blocks the thread
public Part GetPart(int id)
{
    return _context.GetPartAsync(id).Result;  // Blocks! Can cause deadlocks
}

// RIGHT - async all the way
public async Task<Part> GetPartAsync(int id)
{
    return await _context.GetPartAsync(id);  // Non-blocking
}

// Multiple async operations
public async Task<DashboardData> GetDashboardAsync()
{
    // Run both queries simultaneously (parallel)
    var partsTask = _context.GetPartsAsync();
    var categoriesTask = _context.GetCategoriesAsync();

    // Wait for both to complete
    await Task.WhenAll(partsTask, categoriesTask);

    return new DashboardData
    {
        Parts = await partsTask,
        Categories = await categoriesTask
    };
}
```

### 3.4 Data Access (Database Layer)

**File: `src/PartsDb.Shared/Data/PartsDbContext.cs`**

This layer handles all database operations.

```csharp
using System.Data;
using Microsoft.Data.Sqlite;
using Dapper;  // Micro-ORM for simple database access

namespace PartsDb.Shared.Data;

// IDisposable means this class holds resources that need cleanup
public class PartsDbContext : IDisposable
{
    private readonly SqliteConnection _connection;
    private bool _disposed;  // Track if we've already cleaned up

    public PartsDbContext(string databasePath)
    {
        // Connection string builder - safer than string concatenation
        var connectionString = new SqliteConnectionStringBuilder
        {
            DataSource = databasePath,
            Mode = SqliteOpenMode.ReadWriteCreate,
            Cache = SqliteCacheMode.Shared
        }.ToString();

        _connection = new SqliteConnection(connectionString);
    }

    // Async initialization
    public async Task InitializeAsync()
    {
        await _connection.OpenAsync();
        await ExecuteSchemaAsync();
    }

    // Example: Parameterized query (prevents SQL injection!)
    public async Task<Part?> GetPartByIdAsync(int id)
    {
        // Raw SQL string (using raw string literal """)
        const string sql = """
            SELECT p.*, c.id, c.name, c.description
            FROM parts p
            LEFT JOIN categories c ON p.category_id = c.id
            WHERE p.id = @Id
            """;

        // Dapper's QueryAsync with multi-mapping
        // Maps result to Part AND Category objects
        var parts = await _connection.QueryAsync<Part, Category, Part>(
            sql,
            (part, category) =>  // Lambda to combine results
            {
                part.Category = category;
                return part;
            },
            new { Id = id },     // Anonymous object for parameters
            splitOn: "id"        // Where to split for second object
        );

        return parts.FirstOrDefault();  // Return first or null
    }

    // Transaction example (all-or-nothing operations)
    public async Task UpdateQuantityAsync(int partId, int change,
                                          TransactionType type, string? reference)
    {
        using var transaction = _connection.BeginTransaction();
        try
        {
            // Update quantity
            await _connection.ExecuteAsync(
                "UPDATE parts SET quantity = quantity + @Change WHERE id = @Id",
                new { Change = change, Id = partId },
                transaction  // Pass transaction
            );

            // Log the transaction
            await _connection.ExecuteAsync("""
                INSERT INTO inventory_transactions
                (part_id, quantity_change, transaction_type, reference)
                VALUES (@PartId, @Change, @Type, @Reference)
                """,
                new { PartId = partId, Change = change,
                      Type = type.ToString().ToUpper(), Reference = reference },
                transaction
            );

            transaction.Commit();  // Success - save all changes
        }
        catch
        {
            transaction.Rollback();  // Error - undo all changes
            throw;  // Re-throw the exception
        }
    }

    // IDisposable implementation - cleanup resources
    public void Dispose()
    {
        if (!_disposed)
        {
            _connection.Dispose();
            _disposed = true;
        }
        GC.SuppressFinalize(this);  // Tell GC we've cleaned up
    }
}
```

**Key Database Concepts:**

```csharp
// DANGER: SQL Injection vulnerability!
string query = $"SELECT * FROM parts WHERE name = '{userInput}'";
// If userInput is: "'; DROP TABLE parts; --"
// This becomes: SELECT * FROM parts WHERE name = ''; DROP TABLE parts; --'

// SAFE: Parameterized queries
string query = "SELECT * FROM parts WHERE name = @Name";
await connection.QueryAsync(query, new { Name = userInput });
// Parameters are escaped automatically
```

### 3.5 Dependency Injection

**File: `src/PartsDb.Maui/MauiProgram.cs`**

Dependency Injection (DI) is how .NET manages object creation and lifetimes.

```csharp
public static class MauiProgram
{
    public static MauiApp CreateMauiApp()
    {
        var builder = MauiApp.CreateBuilder();

        // Configure the app
        builder.UseMauiApp<App>();

        // DEPENDENCY INJECTION SETUP
        // Register services with the DI container

        // Singleton: ONE instance for entire app lifetime
        // Good for: Database connections, configuration
        builder.Services.AddSingleton(new PartsDbContext(dbPath));
        builder.Services.AddSingleton<IImageProcessor, ImageProcessor>();
        builder.Services.AddSingleton<IPartsService, PartsService>();

        // Scoped: ONE instance per "scope" (per request in web apps)
        // builder.Services.AddScoped<IMyService, MyService>();

        // Transient: NEW instance every time it's requested
        // builder.Services.AddTransient<IMyService, MyService>();

        return builder.Build();
    }
}
```

**How DI Works:**

```csharp
// WITHOUT Dependency Injection (tight coupling)
public class OrderService
{
    private PartsService _partsService;

    public OrderService()
    {
        // Hard-coded dependency - hard to test!
        _partsService = new PartsService(
            new PartsDbContext("parts.db"),
            new ImageProcessor()
        );
    }
}

// WITH Dependency Injection (loose coupling)
public class OrderService
{
    private readonly IPartsService _partsService;

    // Dependencies injected through constructor
    public OrderService(IPartsService partsService)
    {
        _partsService = partsService;
    }
}

// Now you can easily swap implementations:
// - Real service for production
// - Mock service for testing
```

---

## 4. Blazor and UI

### 4.1 Blazor Components

**File: `src/PartsDb.Maui/Pages/Index.razor`**

Blazor uses `.razor` files that combine C# and HTML.

```razor
@* This is a Razor comment *@

@* Page directive - sets the URL route *@
@page "/"

@* Inject services (DI in action!) *@
@inject IPartsService PartsService

@* HTML with Razor syntax *@
<div class="search-container">
    @* Two-way binding: changes sync between UI and variable *@
    <input type="text"
           @bind="searchQuery"
           @bind:event="oninput"
           @onkeyup="OnSearchKeyUp" />
</div>

@* Conditional rendering *@
@if (isLoading)
{
    <div class="loading">Loading...</div>
}
else if (parts != null && parts.Any())
{
    @* Loop through collection *@
    @foreach (var part in parts)
    {
        @* Event handler with parameter *@
        <div class="part-card" @onclick="() => SelectPart(part)">
            <span>@part.Name</span>
            <span>@part.Quantity in stock</span>
        </div>
    }
}
else
{
    <p>No parts found.</p>
}

@* C# code block *@
@code {
    // Component state (variables)
    private IEnumerable<Part>? parts;
    private string searchQuery = "";
    private bool isLoading = true;
    private Part? selectedPart;

    // Lifecycle method - runs when component initializes
    protected override async Task OnInitializedAsync()
    {
        await LoadData();
    }

    private async Task LoadData()
    {
        isLoading = true;
        try
        {
            parts = await PartsService.GetPartsAsync();
        }
        finally
        {
            isLoading = false;
        }
        // StateHasChanged() is called automatically after async methods
    }

    // Event handler
    private void SelectPart(Part part)
    {
        selectedPart = part;
        // Blazor automatically re-renders when state changes
    }

    // Async event handler
    private async Task OnSearchKeyUp(KeyboardEventArgs e)
    {
        if (string.IsNullOrWhiteSpace(searchQuery))
        {
            parts = await PartsService.GetPartsAsync();
        }
        else
        {
            parts = await PartsService.SearchPartsAsync(searchQuery);
        }
    }
}
```

### 4.2 Component Communication

```razor
@* Parent component passing data DOWN to child *@
<PartCard Part="@selectedPart" OnDelete="HandleDelete" />

@* Child component (PartCard.razor) *@
@code {
    // Parameter - receives data from parent
    [Parameter]
    public Part? Part { get; set; }

    // EventCallback - sends events UP to parent
    [Parameter]
    public EventCallback<int> OnDelete { get; set; }

    private async Task DeletePart()
    {
        if (Part != null)
        {
            // Invoke the callback, parent handles the actual delete
            await OnDelete.InvokeAsync(Part.Id);
        }
    }
}
```

---

## 5. Exercises

### Exercise 1: Add a New Property (Beginner)

Add a `Notes` property to track additional information about parts.

1. Open `src/PartsDb.Shared/Models/Part.cs`
2. Add: `public string? Notes { get; set; }`
3. Update the database schema in `database/schema.sql`
4. Update the service methods to include the new field

### Exercise 2: Create a New Service Method (Intermediate)

Add a method to get parts by manufacturer.

```csharp
// In IPartsService.cs, add:
Task<IEnumerable<Part>> GetPartsByManufacturerAsync(string manufacturer);

// In PartsService.cs, implement it:
public Task<IEnumerable<Part>> GetPartsByManufacturerAsync(string manufacturer)
    => _context.GetPartsByManufacturerAsync(manufacturer);

// In PartsDbContext.cs, add the query:
public async Task<IEnumerable<Part>> GetPartsByManufacturerAsync(string manufacturer)
{
    const string sql = """
        SELECT * FROM parts
        WHERE manufacturer = @Manufacturer AND is_active = 1
        ORDER BY name
        """;

    return await _connection.QueryAsync<Part>(sql, new { Manufacturer = manufacturer });
}
```

### Exercise 3: Add Input Validation (Intermediate)

Create a validation service for parts.

```csharp
public class PartValidator
{
    public ValidationResult Validate(Part part)
    {
        var errors = new List<string>();

        if (string.IsNullOrWhiteSpace(part.PartNumber))
            errors.Add("Part number is required");

        if (part.PartNumber?.Length > 64)
            errors.Add("Part number cannot exceed 64 characters");

        if (part.Quantity < 0)
            errors.Add("Quantity cannot be negative");

        return new ValidationResult
        {
            IsValid = errors.Count == 0,
            Errors = errors
        };
    }
}

public record ValidationResult
{
    public bool IsValid { get; init; }
    public IReadOnlyList<string> Errors { get; init; } = Array.Empty<string>();
}
```

### Exercise 4: Add Unit Tests (Intermediate)

```csharp
// In tests/PartsDb.Tests/PartValidatorTests.cs
public class PartValidatorTests
{
    private readonly PartValidator _validator = new();

    [Fact]
    public void Validate_WithEmptyPartNumber_ReturnsError()
    {
        // Arrange
        var part = new Part { PartNumber = "", Name = "Test" };

        // Act
        var result = _validator.Validate(part);

        // Assert
        Assert.False(result.IsValid);
        Assert.Contains(result.Errors, e => e.Contains("Part number"));
    }

    [Theory]
    [InlineData(-1)]
    [InlineData(-100)]
    public void Validate_WithNegativeQuantity_ReturnsError(int quantity)
    {
        var part = new Part { PartNumber = "PN001", Quantity = quantity };

        var result = _validator.Validate(part);

        Assert.False(result.IsValid);
    }
}
```

### Exercise 5: Create a REST API (Advanced)

Extend the project with an ASP.NET Core Web API.

```csharp
// Create new project: PartsDb.Api

// Controllers/PartsController.cs
[ApiController]
[Route("api/[controller]")]
public class PartsController : ControllerBase
{
    private readonly IPartsService _partsService;

    public PartsController(IPartsService partsService)
    {
        _partsService = partsService;
    }

    [HttpGet]
    public async Task<ActionResult<IEnumerable<Part>>> GetParts()
    {
        var parts = await _partsService.GetPartsAsync();
        return Ok(parts);
    }

    [HttpGet("{id}")]
    public async Task<ActionResult<Part>> GetPart(int id)
    {
        var part = await _partsService.GetPartByIdAsync(id);

        if (part == null)
            return NotFound();

        return Ok(part);
    }

    [HttpPost]
    public async Task<ActionResult<Part>> CreatePart(Part part)
    {
        var id = await _partsService.CreatePartAsync(part);
        part.Id = id;

        return CreatedAtAction(nameof(GetPart), new { id }, part);
    }
}
```

---

## 6. Learning Resources

### Official Microsoft Documentation

| Topic | URL |
|-------|-----|
| C# Documentation | https://learn.microsoft.com/en-us/dotnet/csharp/ |
| .NET Fundamentals | https://learn.microsoft.com/en-us/dotnet/fundamentals/ |
| ASP.NET Core | https://learn.microsoft.com/en-us/aspnet/core/ |
| Blazor | https://learn.microsoft.com/en-us/aspnet/core/blazor/ |
| .NET MAUI | https://learn.microsoft.com/en-us/dotnet/maui/ |
| Entity Framework Core | https://learn.microsoft.com/en-us/ef/core/ |

### Recommended Books

#### For Beginners
1. **"C# 12 and .NET 8 - Modern Cross-Platform Development"** by Mark J. Price
   - Comprehensive introduction to C# and .NET
   - Hands-on projects throughout
   - Covers web, desktop, and mobile development

2. **"Head First C#"** by Andrew Stellman & Jennifer Greene
   - Visual, engaging learning style
   - Great for absolute beginners
   - Focuses on practical application

#### For Intermediate Developers
3. **"C# in Depth"** by Jon Skeet
   - Deep dive into C# language features
   - Explains the "why" behind design decisions
   - Essential for understanding advanced concepts

4. **"Dependency Injection Principles, Practices, and Patterns"** by Steven van Deursen & Mark Seemann
   - Master DI and SOLID principles
   - Applicable beyond just .NET

#### For Advanced Developers
5. **"CLR via C#"** by Jeffrey Richter
   - Understand how .NET works under the hood
   - Performance optimization
   - Advanced runtime concepts

6. **"Concurrency in C# Cookbook"** by Stephen Cleary
   - Master async/await
   - Parallel programming patterns
   - Real-world concurrency solutions

### Online Courses & Tutorials

| Platform | Course | Level |
|----------|--------|-------|
| Microsoft Learn | [C# Learning Path](https://learn.microsoft.com/en-us/training/paths/csharp-first-steps/) | Beginner |
| Pluralsight | C# Fundamentals | Beginner |
| Udemy | Complete C# Masterclass | Beginner-Intermediate |
| YouTube | [Nick Chapsas](https://www.youtube.com/@nickchapsas) | Intermediate-Advanced |
| YouTube | [Raw Coding](https://www.youtube.com/@RawCoding) | Intermediate |

### Project-Based Learning Ideas

1. **Inventory Alert System**
   - Add email notifications when stock is low
   - Learn: Background services, SMTP, scheduling

2. **Barcode Scanner Integration**
   - Scan barcodes to look up parts
   - Learn: Device APIs, camera integration

3. **REST API + Mobile App**
   - Create API backend + separate mobile frontend
   - Learn: API design, authentication, mobile development

4. **Data Import/Export**
   - Import from CSV/Excel, export reports
   - Learn: File handling, data transformation

5. **Multi-User Support**
   - Add authentication and authorization
   - Learn: Identity, roles, security

### Community Resources

- **Stack Overflow** - Q&A for specific problems
- **Reddit** - r/csharp, r/dotnet
- **Discord** - C# Discord server
- **GitHub** - Explore open-source .NET projects

---

## Quick Reference

### Common LINQ Methods

```csharp
var parts = new List<Part> { /* ... */ };

// Filtering
var resistors = parts.Where(p => p.Category == "Resistors");

// Projection (select specific fields)
var names = parts.Select(p => p.Name);

// Ordering
var sorted = parts.OrderBy(p => p.Name);
var sortedDesc = parts.OrderByDescending(p => p.Quantity);

// Aggregation
int totalQuantity = parts.Sum(p => p.Quantity);
double avgPrice = parts.Average(p => p.UnitPrice ?? 0);
int count = parts.Count();

// First/Single
var first = parts.First();                    // Throws if empty
var firstOrNull = parts.FirstOrDefault();     // Returns null if empty
var single = parts.Single(p => p.Id == 1);    // Throws if not exactly one

// Checking
bool anyLowStock = parts.Any(p => p.Quantity < 10);
bool allInStock = parts.All(p => p.Quantity > 0);

// Grouping
var byCategory = parts.GroupBy(p => p.Category);
```

### String Operations

```csharp
string text = "  Hello, World!  ";

text.Trim()                    // "Hello, World!"
text.ToUpper()                 // "  HELLO, WORLD!  "
text.ToLower()                 // "  hello, world!  "
text.Contains("World")         // true
text.StartsWith("  Hello")     // true
text.Replace("World", "C#")    // "  Hello, C#!  "
text.Split(',')                // ["  Hello", " World!  "]

string.IsNullOrEmpty(text)     // false
string.IsNullOrWhiteSpace("")  // true

// String interpolation
int count = 5;
string msg = $"Found {count} items";

// Verbatim string (ignore escape sequences)
string path = @"C:\Users\Name\Documents";

// Raw string literal (C# 11+)
string json = """
    {
        "name": "Part",
        "count": 5
    }
    """;
```

### Common Patterns

```csharp
// Null checking
if (part is not null)
{
    Console.WriteLine(part.Name);
}

// Pattern matching
string GetStockStatus(Part part) => part.Quantity switch
{
    0 => "Out of Stock",
    < 10 => "Low Stock",
    < 50 => "In Stock",
    _ => "Well Stocked"  // Default case
};

// Using statement (auto-dispose)
using var connection = new SqliteConnection(connectionString);
// Connection is automatically closed when scope ends

// Try-catch
try
{
    await DoSomethingAsync();
}
catch (SqliteException ex) when (ex.ErrorCode == 19)
{
    // Handle specific error
}
catch (Exception ex)
{
    _logger.LogError(ex, "Operation failed");
    throw;  // Re-throw to preserve stack trace
}
```

---

Happy coding! Remember: the best way to learn is by doing. Start with the exercises above, then try building your own features!
