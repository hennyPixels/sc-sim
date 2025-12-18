using SixLabors.ImageSharp;
using SixLabors.ImageSharp.Formats.Webp;
using SixLabors.ImageSharp.Processing;
using SixLabors.ImageSharp.PixelFormats;

namespace PartsDb.Shared.ImageProcessing;

public class ImageProcessor : IImageProcessor
{
    private const int MaxFullImageSize = 1024;
    private const int ThumbnailSize = 150;
    private const int EmbeddedSize = 64;
    private const int FullImageQuality = 85;
    private const int ThumbnailQuality = 75;
    private const int EmbeddedQuality = 50;

    public async Task<ProcessedImage> ProcessImageAsync(Stream imageStream)
    {
        using var image = await Image.LoadAsync<Rgba32>(imageStream);

        var result = new ProcessedImage
        {
            Width = image.Width,
            Height = image.Height
        };

        // Create full-size image (max 1024px)
        result.FullImage = await ResizeAndEncodeAsync(image.Clone(x => x), MaxFullImageSize, FullImageQuality);

        // Create thumbnail (150px)
        result.Thumbnail = await ResizeAndEncodeAsync(image.Clone(x => x), ThumbnailSize, ThumbnailQuality);

        // Create embedded image (64px, grayscale, heavily compressed)
        result.EmbeddedImage = await CreateEmbeddedImageInternalAsync(image);

        return result;
    }

    public async Task<byte[]> CreateThumbnailAsync(Stream imageStream, int maxSize = 150)
    {
        using var image = await Image.LoadAsync<Rgba32>(imageStream);
        return await ResizeAndEncodeAsync(image, maxSize, ThumbnailQuality);
    }

    public async Task<byte[]> CreateEmbeddedImageAsync(Stream imageStream, int maxSize = 64)
    {
        using var image = await Image.LoadAsync<Rgba32>(imageStream);
        return await CreateEmbeddedImageInternalAsync(image, maxSize);
    }

    private static async Task<byte[]> ResizeAndEncodeAsync(Image<Rgba32> image, int maxSize, int quality)
    {
        // Calculate new dimensions maintaining aspect ratio
        var (newWidth, newHeight) = CalculateDimensions(image.Width, image.Height, maxSize);

        if (newWidth != image.Width || newHeight != image.Height)
        {
            image.Mutate(x => x.Resize(newWidth, newHeight));
        }

        using var ms = new MemoryStream();
        var encoder = new WebpEncoder
        {
            Quality = quality,
            FileFormat = WebpFileFormatType.Lossy
        };

        await image.SaveAsync(ms, encoder);
        return ms.ToArray();
    }

    private static async Task<byte[]> CreateEmbeddedImageInternalAsync(Image<Rgba32> image, int maxSize = 64)
    {
        var (newWidth, newHeight) = CalculateDimensions(image.Width, image.Height, maxSize);

        // Clone and process for embedded systems
        using var processedImage = image.Clone(x => x
            .Resize(newWidth, newHeight)
            .Grayscale());

        // For embedded systems, we create a custom compressed format
        // Header: 2 bytes width, 2 bytes height, then 4-bit grayscale pixels
        var pixelData = new List<byte>
        {
            (byte)(newWidth & 0xFF),
            (byte)((newWidth >> 8) & 0xFF),
            (byte)(newHeight & 0xFF),
            (byte)((newHeight >> 8) & 0xFF)
        };

        // Convert to 4-bit grayscale (2 pixels per byte)
        byte currentByte = 0;
        bool highNibble = true;

        for (int y = 0; y < newHeight; y++)
        {
            for (int x = 0; x < newWidth; x++)
            {
                var pixel = processedImage[x, y];
                // Already grayscale, so R=G=B, take any channel
                byte gray4bit = (byte)(pixel.R >> 4); // Convert 8-bit to 4-bit

                if (highNibble)
                {
                    currentByte = (byte)(gray4bit << 4);
                    highNibble = false;
                }
                else
                {
                    currentByte |= gray4bit;
                    pixelData.Add(currentByte);
                    highNibble = true;
                }
            }
        }

        // Handle odd number of pixels
        if (!highNibble)
        {
            pixelData.Add(currentByte);
        }

        // Simple RLE compression for embedded format
        return await Task.FromResult(CompressRle(pixelData.ToArray()));
    }

    private static byte[] CompressRle(byte[] data)
    {
        if (data.Length < 5) return data;

        var compressed = new List<byte>
        {
            data[0], data[1], data[2], data[3] // Header unchanged
        };

        int i = 4;
        while (i < data.Length)
        {
            byte current = data[i];
            int count = 1;

            while (i + count < data.Length && data[i + count] == current && count < 127)
            {
                count++;
            }

            if (count >= 3)
            {
                // RLE: 0x80 | count, value
                compressed.Add((byte)(0x80 | count));
                compressed.Add(current);
                i += count;
            }
            else
            {
                // Literal
                compressed.Add(current);
                i++;
            }
        }

        // Only use compressed if smaller
        return compressed.Count < data.Length - 4 + compressed.Count
            ? compressed.ToArray()
            : data;
    }

    private static (int width, int height) CalculateDimensions(int originalWidth, int originalHeight, int maxSize)
    {
        if (originalWidth <= maxSize && originalHeight <= maxSize)
        {
            return (originalWidth, originalHeight);
        }

        double ratio;
        if (originalWidth > originalHeight)
        {
            ratio = (double)maxSize / originalWidth;
        }
        else
        {
            ratio = (double)maxSize / originalHeight;
        }

        return ((int)(originalWidth * ratio), (int)(originalHeight * ratio));
    }
}
