using Microsoft.Extensions.Logging;
using PartsDb.Shared.Data;
using PartsDb.Shared.ImageProcessing;
using PartsDb.Shared.Services;

namespace PartsDb.Maui;

public static class MauiProgram
{
    public static MauiApp CreateMauiApp()
    {
        var builder = MauiApp.CreateBuilder();
        builder
            .UseMauiApp<App>()
            .ConfigureFonts(fonts =>
            {
                fonts.AddFont("OpenSans-Regular.ttf", "OpenSansRegular");
            });

        builder.Services.AddMauiBlazorWebView();

#if DEBUG
        builder.Services.AddBlazorWebViewDeveloperTools();
        builder.Logging.AddDebug();
#endif

        // Database path
        var dbPath = Path.Combine(FileSystem.AppDataDirectory, "parts.db");

        // Register services
        builder.Services.AddSingleton(new PartsDbContext(dbPath));
        builder.Services.AddSingleton<IImageProcessor, ImageProcessor>();
        builder.Services.AddSingleton<IPartsService, PartsService>();

        var app = builder.Build();

        // Initialize database
        Task.Run(async () =>
        {
            var context = app.Services.GetRequiredService<PartsDbContext>();
            await context.InitializeAsync();
        }).Wait();

        return app;
    }
}
