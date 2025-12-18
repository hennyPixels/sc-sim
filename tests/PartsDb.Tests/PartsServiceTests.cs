using PartsDb.Shared.Data;
using PartsDb.Shared.ImageProcessing;
using PartsDb.Shared.Models;
using PartsDb.Shared.Services;
using Xunit;

namespace PartsDb.Tests;

public class PartsServiceTests : IDisposable
{
    private readonly string _dbPath;
    private readonly PartsDbContext _context;
    private readonly PartsService _service;

    public PartsServiceTests()
    {
        _dbPath = Path.Combine(Path.GetTempPath(), $"partsdb_test_{Guid.NewGuid()}.db");
        _context = new PartsDbContext(_dbPath);
        _context.InitializeAsync().Wait();
        _service = new PartsService(_context, new ImageProcessor());
    }

    public void Dispose()
    {
        _context.Dispose();
        if (File.Exists(_dbPath))
        {
            File.Delete(_dbPath);
        }
    }

    [Fact]
    public async Task CreatePart_ReturnsValidId()
    {
        var part = new Part
        {
            PartNumber = "TEST-001",
            Name = "Test Part",
            Description = "A test part for unit testing",
            Quantity = 10
        };

        var id = await _service.CreatePartAsync(part);

        Assert.True(id > 0);
    }

    [Fact]
    public async Task GetPartById_ReturnsCorrectPart()
    {
        var part = new Part
        {
            PartNumber = "TEST-002",
            Name = "Another Test Part",
            Quantity = 5
        };

        var id = await _service.CreatePartAsync(part);
        var retrieved = await _service.GetPartByIdAsync(id);

        Assert.NotNull(retrieved);
        Assert.Equal("TEST-002", retrieved.PartNumber);
        Assert.Equal("Another Test Part", retrieved.Name);
        Assert.Equal(5, retrieved.Quantity);
    }

    [Fact]
    public async Task GetPartByNumber_ReturnsCorrectPart()
    {
        var part = new Part
        {
            PartNumber = "UNIQUE-123",
            Name = "Unique Part"
        };

        await _service.CreatePartAsync(part);
        var retrieved = await _service.GetPartByNumberAsync("UNIQUE-123");

        Assert.NotNull(retrieved);
        Assert.Equal("Unique Part", retrieved.Name);
    }

    [Fact]
    public async Task SearchParts_FindsMatchingParts()
    {
        await _service.CreatePartAsync(new Part { PartNumber = "RES-100", Name = "100 Ohm Resistor", Manufacturer = "Vishay" });
        await _service.CreatePartAsync(new Part { PartNumber = "RES-220", Name = "220 Ohm Resistor", Manufacturer = "Vishay" });
        await _service.CreatePartAsync(new Part { PartNumber = "CAP-100", Name = "100uF Capacitor", Manufacturer = "Nichicon" });

        var results = await _service.SearchPartsAsync("Resistor");

        Assert.Equal(2, results.Count());
    }

    [Fact]
    public async Task UpdateQuantity_ChangesQuantityCorrectly()
    {
        var part = new Part
        {
            PartNumber = "QTY-TEST",
            Name = "Quantity Test Part",
            Quantity = 10
        };

        var id = await _service.CreatePartAsync(part);

        await _service.UpdateQuantityAsync(id, 5, TransactionType.In);
        var updated = await _service.GetPartByIdAsync(id);

        Assert.Equal(15, updated?.Quantity);

        await _service.UpdateQuantityAsync(id, -3, TransactionType.Out);
        updated = await _service.GetPartByIdAsync(id);

        Assert.Equal(12, updated?.Quantity);
    }

    [Fact]
    public async Task GetLowStockParts_ReturnsOnlyLowStock()
    {
        await _service.CreatePartAsync(new Part { PartNumber = "LOW-001", Name = "Low Stock Item", Quantity = 2, MinQuantity = 5 });
        await _service.CreatePartAsync(new Part { PartNumber = "OK-001", Name = "OK Stock Item", Quantity = 10, MinQuantity = 5 });

        var lowStock = await _service.GetLowStockPartsAsync();

        Assert.Single(lowStock);
        Assert.Equal("LOW-001", lowStock.First().PartNumber);
    }

    [Fact]
    public async Task DeletePart_RemovesPart()
    {
        var part = new Part
        {
            PartNumber = "DEL-001",
            Name = "Part to Delete"
        };

        var id = await _service.CreatePartAsync(part);
        await _service.DeletePartAsync(id);

        var retrieved = await _service.GetPartByIdAsync(id);
        Assert.Null(retrieved);
    }

    [Fact]
    public async Task GetCategories_ReturnsDefaultCategories()
    {
        var categories = await _service.GetCategoriesAsync();

        Assert.NotEmpty(categories);
        Assert.Contains(categories, c => c.Name == "Resistors");
        Assert.Contains(categories, c => c.Name == "Capacitors");
    }
}
