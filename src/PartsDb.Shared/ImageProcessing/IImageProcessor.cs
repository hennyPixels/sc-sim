namespace PartsDb.Shared.ImageProcessing;

public interface IImageProcessor
{
    Task<ProcessedImage> ProcessImageAsync(Stream imageStream);
    Task<byte[]> CreateThumbnailAsync(Stream imageStream, int maxSize = 150);
    Task<byte[]> CreateEmbeddedImageAsync(Stream imageStream, int maxSize = 64);
}

public class ProcessedImage
{
    public byte[]? FullImage { get; set; }
    public byte[]? Thumbnail { get; set; }
    public byte[]? EmbeddedImage { get; set; }
    public int Width { get; set; }
    public int Height { get; set; }
}
