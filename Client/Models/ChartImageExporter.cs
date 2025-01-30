using System.IO;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using LiveCharts.Wpf;

namespace Client
{
    public static class ChartImageExporter
    {
        /// <summary>
        /// Экспортирует Chart в изображение с белым фоном и сохраняет по указанному пути.
        /// </summary>
        /// <param name="chart">Экземпляр CartesianChart для экспорта.</param>
        /// <param name="filePath">Путь сохранения изображения.</param>
        /// <param name="dpiX">Горизонтальное разрешение (по умолчанию 288).</param>
        /// <param name="dpiY">Вертикальное разрешение (по умолчанию 288).</param>
        public static void ExportToImage(CartesianChart chart, string filePath, double dpiX = 288, double dpiY = 288)
        {
            if (chart == null)
                throw new ArgumentNullException(nameof(chart), "Chart не должен быть null.");

            Rect bounds = VisualTreeHelper.GetDescendantBounds(chart);

            RenderTargetBitmap renderBitmap = new RenderTargetBitmap(
                (int)(bounds.Width * dpiX / 96.0),
                (int)(bounds.Height * dpiY / 96.0),
                dpiX,
                dpiY,
                PixelFormats.Pbgra32);

            DrawingVisual drawingVisual = new DrawingVisual();
            using (DrawingContext ctx = drawingVisual.RenderOpen())
            {
                ctx.DrawRectangle(Brushes.White, null, new Rect(new Point(), bounds.Size));

                VisualBrush visualBrush = new VisualBrush(chart);
                ctx.DrawRectangle(visualBrush, null, new Rect(new Point(), bounds.Size));
            }

            renderBitmap.Render(drawingVisual);

            using (var fileStream = new FileStream(filePath, FileMode.Create))
            {
                BitmapEncoder encoder = new JpegBitmapEncoder();
                encoder.Frames.Add(BitmapFrame.Create(renderBitmap));
                encoder.Save(fileStream);
            }
        }
    }
}
