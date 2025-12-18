using PartsDb.Shared.Models;

namespace PartsDb.Shared.Services;

public interface IPartsService
{
    Task<IEnumerable<Part>> GetPartsAsync(int limit = 100, int offset = 0);
    Task<Part?> GetPartByIdAsync(int id);
    Task<Part?> GetPartByNumberAsync(string partNumber);
    Task<IEnumerable<Part>> SearchPartsAsync(string query);
    Task<int> CreatePartAsync(Part part);
    Task UpdatePartAsync(Part part);
    Task DeletePartAsync(int id);
    Task UpdateQuantityAsync(int partId, int change, TransactionType type, string? reference = null);
    Task<IEnumerable<Part>> GetLowStockPartsAsync();
    Task<IEnumerable<Category>> GetCategoriesAsync();
    Task<int> CreateCategoryAsync(Category category);
    Task ImportImageAsync(int partId, Stream imageStream, string filename, bool isPrimary = false);
    Task<byte[]?> GetImageAsync(int partId, ImageSize size);
}
