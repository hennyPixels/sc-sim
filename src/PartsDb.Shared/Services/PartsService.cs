using PartsDb.Shared.Data;
using PartsDb.Shared.ImageProcessing;
using PartsDb.Shared.Models;

namespace PartsDb.Shared.Services;

public class PartsService : IPartsService
{
    private readonly PartsDbContext _context;
    private readonly IImageProcessor _imageProcessor;

    public PartsService(PartsDbContext context, IImageProcessor imageProcessor)
    {
        _context = context;
        _imageProcessor = imageProcessor;
    }

    public Task<IEnumerable<Part>> GetPartsAsync(int limit = 100, int offset = 0)
        => _context.GetPartsAsync(limit, offset);

    public Task<Part?> GetPartByIdAsync(int id)
        => _context.GetPartByIdAsync(id);

    public Task<Part?> GetPartByNumberAsync(string partNumber)
        => _context.GetPartByNumberAsync(partNumber);

    public Task<IEnumerable<Part>> SearchPartsAsync(string query)
    {
        if (string.IsNullOrWhiteSpace(query))
            return _context.GetPartsAsync();

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

    public async Task ImportImageAsync(int partId, Stream imageStream, string filename, bool isPrimary = false)
    {
        var processedImage = await _imageProcessor.ProcessImageAsync(imageStream);

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
            FileSize = processedImage.FullImage?.Length ?? 0,
            IsPrimary = isPrimary
        };

        await _context.SaveImageAsync(partImage);
    }

    public async Task<byte[]?> GetImageAsync(int partId, ImageSize size)
    {
        var image = await _context.GetImageAsync(partId, size);
        return image?.ImageFull;
    }
}
