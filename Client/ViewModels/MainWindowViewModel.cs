﻿using LiveCharts.Wpf;
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
using LiveCharts;
using LiveCharts.Defaults;

namespace Client;

public class MainWindowViewModel : ViewModelBase
{
    private double _xMin;
    private double _xMax;
    private double _yMin;
    private double _yMax;

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

    public double XMin
    {
        get => _xMin;
        set
        {
            if (_xMin != value)
            {
                _xMin = value;
                OnPropertyChanged(nameof(XMin));
            }
        }
    }

    public double XMax
    {
        get => _xMax;
        set
        {
            if (_xMax != value)
            {
                _xMax = value;
                OnPropertyChanged(nameof(XMax));
            }
        }
    }

    public double YMin
    {
        get => _yMin;
        set
        {
            if (_yMin != value)
            {
                _yMin = value;
                OnPropertyChanged(nameof(YMin));
            }
        }
    }

    public double YMax
    {
        get => _yMax;
        set
        {
            if (_yMax != value)
            {
                _yMax = value;
                OnPropertyChanged(nameof(YMax));
            }
        }
    }

    public MainWindowViewModel(CartesianChart chart)
    {
        Chart = chart;
        chartManager = new(Chart);
        XMin = 0;
        XMax = 10;
        YMin = 0;
        YMax = 10;

        DrawTestGraph();
    }

    private void DrawTestGraph()
    {
        // Первый график
        var dataPoints1 = new List<Point>
        {
            new Point(0, 1),
            new Point(1, 2),
            new Point(2, 3),
            new Point(3, 4),
            new Point(4, 5)
        };
        chartManager.AddChart("Linear Growth", dataPoints1);

        // Второй график
        var dataPoints2 = new List<Point>
        {
            new Point(0, 1),
            new Point(1, 1),
            new Point(2, 2),
            new Point(3, 6),
            new Point(4, 24)
        };
        chartManager.AddChart("Factorial Growth", dataPoints2);

        // Третий график
        var dataPoints3 = new List<Point>
        {
            new Point(0, 1),
            new Point(1, 0.5),
            new Point(2, 0.25),
            new Point(3, 0.125),
            new Point(4, 0.0625)
        };
        chartManager.AddChart("Exponential Decay", dataPoints3);

        UpdateScale();
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
        get => _resetZoomCommand ??= new RelayCommand(_ => UpdateScale());
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

    private void ApplyAxisLimits()
    {
        OnPropertyChanged(nameof(XMin));
        OnPropertyChanged(nameof(XMax));
        OnPropertyChanged(nameof(YMin));
        OnPropertyChanged(nameof(YMax));

        foreach (var axis in Chart.AxisX)
        {
            axis.MinValue = XMin;
            axis.MaxValue = XMax;
        }

        foreach (var axis in Chart.AxisY)
        {
            axis.MinValue = YMin;
            axis.MaxValue = YMax;
        }
    }

    public void UpdateScale()
    {
        double globalXMin = double.MaxValue;
        double globalXMax = double.MinValue;
        double globalYMin = double.MaxValue;
        double globalYMax = double.MinValue;

        foreach (var series in Chart.Series)
        {
            if (series is LineSeries lineSeries && lineSeries.Values is ChartValues<ObservablePoint> values)
            {
                foreach (var point in values)
                {
                    globalXMin = Math.Min(globalXMin, point.X);
                    globalXMax = Math.Max(globalXMax, point.X);
                    globalYMin = Math.Min(globalYMin, point.Y);
                    globalYMax = Math.Max(globalYMax, point.Y);
                }
            }
        }

        double xPadding = (globalXMax - globalXMin) * 0.1;
        double yPadding = (globalYMax - globalYMin) * 0.1;

        foreach (var axis in Chart.AxisX)
        {
            XMin = globalXMin - xPadding;
            XMax = globalXMax + xPadding;
        }

        foreach (var axis in Chart.AxisY)
        {
            YMin = globalYMin - yPadding;
            YMax = globalYMax + yPadding;
        }

        ApplyAxisLimits();
        Chart.Update(true, true);
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
