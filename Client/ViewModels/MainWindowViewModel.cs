using LiveCharts.Wpf;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using System.Windows;
using LiveCharts.Wpf.Charts.Base;
using System.Windows.Media;

namespace Client;

public class MainWindowViewModel
{
    private const string _sourceCode = @"https://github.com/Alexey-Sagaydak/ODEClientServer";
    private const string _documentation = @"https://github.com/Alexey-Sagaydak/ODEClientServer/blob/master/README.md";

    private RelayCommand _exitCommand;
    private RelayCommand _viewSourceCodeCommand;
    private RelayCommand _zoomInCommand;
    private RelayCommand _zoomOutCommand;
    private RelayCommand _resetZoomCommand;
    private RelayCommand _helpCommand;
    private RelayCommand _saveAsImageCommand;
    private RelayCommand _deleteChartsCommand;

    private ChartManager chartManager;

    private const double ZoomStep = 0.2;

    public CartesianChart Chart { get; set; }
    public event EventHandler RequestClose;

    public MainWindowViewModel(CartesianChart chart)
    {
        Chart = chart;
        chartManager = new(Chart);

        DrawTestGraph();
    }

    private void DrawTestGraph()
    {
        List<Point> dataPoints = new List<Point>
        {
            new Point(-4, 16),
            new Point(-3, 9),
            new Point(-2, 4),
            new Point(-1, 2),
            new Point(0, 1),
            new Point(1, 2),
            new Point(2, 4),
            new Point(3, 9),
            new Point(4, 16),
        };

        string graphTitle = "x^2";

        chartManager.AddChart(graphTitle, dataPoints);
        chartManager.UpdateScale();
    }

    public RelayCommand SaveAsImageCommand
    {
        get => _saveAsImageCommand ??= new RelayCommand(SaveFile);
    }
    
    public RelayCommand DeleteChartsCommand
    {
        get => _deleteChartsCommand ??= new RelayCommand(_ => chartManager.ClearCharts());
    }

    public RelayCommand ExitCommand
    {
        get => _exitCommand ??= new RelayCommand(Exit);
    }

    public RelayCommand ViewSourceCodeCommand
    {
        get => _viewSourceCodeCommand ??= new RelayCommand(_ => HtmlHelper.OpenInBrowser(_sourceCode));
    }

    public RelayCommand HelpCommand
    {
        get => _helpCommand ??= new RelayCommand(_ => HtmlHelper.OpenInBrowser(_documentation));
    }

    public RelayCommand ZoomInCommand
    {
        get => _zoomInCommand ??= new RelayCommand(_ => Zoom(1 - ZoomStep));
    }

    public RelayCommand ZoomOutCommand
    {
        get => _zoomOutCommand ??= new RelayCommand(_ => Zoom(1 + ZoomStep));
    }

    public RelayCommand ResetZoomCommand
    {
        get => _resetZoomCommand ??= new RelayCommand(_ => chartManager.UpdateScale());
    }

    private void Zoom(double factor)
    {
        if (Chart == null) return;

        foreach (var axis in Chart.AxisX)
        {
            AdjustAxis(axis, factor);
        }

        foreach (var axis in Chart.AxisY)
        {
            AdjustAxis(axis, factor);
        }
    }

    private void AdjustAxis(Axis axis, double factor)
    {
        if (double.IsNaN(axis.MinValue) || double.IsNaN(axis.MaxValue)) return;

        var range = axis.MaxValue - axis.MinValue;
        var mid = (axis.MaxValue + axis.MinValue) / 2;
        axis.MinValue = mid - range * factor / 2;
        axis.MaxValue = mid + range * factor / 2;
    }

    private void SaveFile(object obj)
    {
        var saveFileDialog = new SaveFileDialog
        {
            Filter = "JPEG Image|*.jpg|All Files|*.*",
            DefaultExt = ".jpg",
            FileName = "chart_image"
        };

        if (saveFileDialog.ShowDialog() == true)
        {
            var filePath = saveFileDialog.FileName;

            SaveChartAsImage(filePath);
        }
    }

    private void SaveChartAsImage(string filePath)
    {
        try
        {
            ChartImageExporter.ExportToImage(Chart, filePath, 288, 288);

            var result = MessageBox.Show("Изображение сохранено успешно! Открыть файл для просмотра?",
                                         "Успех",
                                         MessageBoxButton.YesNo,
                                         MessageBoxImage.Question);

            if (result == MessageBoxResult.Yes)
            {
                System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
                {
                    FileName = filePath,
                    UseShellExecute = true
                });
            }
        }
        catch (Exception ex)
        {
            MessageBox.Show($"Ошибка при сохранении изображения: {ex.Message}", "Ошибка", MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }

    public void Exit(object obj = null)
    {
        OnRequestClose();
    }
    private void OnRequestClose()
    {
        RequestClose?.Invoke(this, EventArgs.Empty);
    }
}
