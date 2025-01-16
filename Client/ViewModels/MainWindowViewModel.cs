﻿using LiveCharts.Wpf;
using MaterialDesignThemes.Wpf;
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
using System.Numerics;
using System.Windows.Controls.Ribbon.Primitives;
using LiveCharts.Events;

namespace Client;

public class MainWindowViewModel : ViewModelBase
{
    private readonly IServerRequest _ping_request;
    private readonly string _serverUrl = $"http://{Constants.SOCKET}/";

    private string _selectedXAxis;
    private string _selectedYAxis;
    private List<string> _availableAxes;

    private double _xMin;
    private double _xMax;
    private double _yMin;
    private double _yMax;

    private double _lastMin;
    private double _lastMax;

    private bool _draw_anyway;

    private string _selectedMethodString;
    private SolutionMethod _selectedMethod;

    private string _selectedEquationString;
    private EquationType _selectedEquation;

    private string _equationFormula;
    private string _graphTitle;

    private const string _sourceCode = @"https://github.com/Alexey-Sagaydak/ODEClientServer";
    private const string _documentation = @"https://github.com/Alexey-Sagaydak/ODEClientServer/blob/master/README.md";

    public string ConnectionStatus => IsConnected ? "Подключен к серверу" : "Отсутствует подключение к серверу";
    public string IconKind => IsConnected ? "NetworkOutline" : "NetworkOffOutline";

    private bool _isConnected;

    private RelayCommand _exitCommand;
    private RelayCommand _viewSourceCodeCommand;
    private RelayCommand _zoomInCommand;
    private RelayCommand _zoomOutCommand;
    private RelayCommand _resetZoomCommand;
    private RelayCommand _helpCommand;
    private RelayCommand _saveAsImageCommand;
    private RelayCommand _deleteChartsCommand;
    private RelayCommand _aboutCommand;
    private RelayCommand _solveODE;

    private ChartManager chartManager;
    private Snackbar ErrorSnackbar;
    private GraphStorage storage;
    public string XAxis { get; set; } = "y0";
    public string YAxis { get; set; } = "y1";


    private const double ZoomStep = 0.2;

    private readonly ITaskSolverService _taskSolverService;
    private string _serverResponse;
    private System.Timers.Timer _updateTimer;

    public string ServerResponse
    {
        get => _serverResponse;
        set => SetProperty(ref _serverResponse, value);
    }

    public CartesianChart Chart { get; set; }
    public event EventHandler RequestClose;
    public ObservableCollection<ParameterViewModel> Parameters { get; } = [];

    public MainWindowViewModel(CartesianChart chart, Snackbar errorSnackbar)
    {
        Chart = chart;
        chartManager = new(Chart);
        XMin = 0;
        XMax = 10;
        YMin = 0;
        YMax = 10;
        _lastMin = double.MinValue;
        _lastMax = double.MaxValue;
        SelectedEquationString = EquationMapper.GetEquationString(EquationType.VanDerPol);
        ErrorSnackbar = errorSnackbar;
        _draw_anyway = false;

        storage = new GraphStorage();
        _ping_request = new PingRequest();
        _taskSolverService = new TaskSolverService(_serverUrl);

        LoadAxes();
        InitializePeriodicUpdate();
        StartPingLoop();
    }

    private async void StartPingLoop()
    {
        while (true)
        {
            IsConnected = await _ping_request.ExecuteAsync(_serverUrl);
            OnPropertyChanged(nameof(ConnectionStatus));
            OnPropertyChanged(nameof(IconKind));
            await Task.Delay(5000);
        }
    }

    public List<string> AvailableAxes
    {
        get => _availableAxes;
        set
        {
            _availableAxes = value;
            OnPropertyChanged(nameof(AvailableAxes));
        }
    }

    public string SelectedXAxis
    {
        get => _selectedXAxis;
        set
        {
            _selectedXAxis = value;
            OnPropertyChanged(nameof(SelectedXAxis));
            _draw_anyway = true;
            UpdateScale();
        }
    }

    public string SelectedYAxis
    {
        get => _selectedYAxis;
        set
        {
            _selectedYAxis = value;
            OnPropertyChanged(nameof(SelectedYAxis));
            _draw_anyway = true;
            UpdateScale();
        }
    }

    public bool IsConnected
    {
        get => _isConnected;
        set => SetProperty(ref _isConnected, value);
    }

    public string GraphTitle
    {
        get => _graphTitle;
        set => SetProperty(ref _graphTitle, value);
    }

    public string SelectedMethodString
    {
        get => _selectedMethodString;
        set
        {
            if (_selectedMethodString != value)
            {
                _selectedMethodString = value;
                UpdateSolverEnumField(value);
            }
        }
    }

    public string EquationFormula
    {
        get => _equationFormula;
        set
        {
            if (_equationFormula != value)
            {
                _equationFormula = value;
                OnPropertyChanged(nameof(EquationFormula));
            }
        }
    }

    public SolutionMethod SelectedMethod
    {
        get => _selectedMethod;
        private set
        {
            if (_selectedMethod != value)
            {
                _selectedMethod = value;
            }
        }
    }

    public string SelectedEquationString
    {
        get => _selectedEquationString;
        set
        {
            if (_selectedEquationString != value)
            {
                _selectedEquationString = value;
                UpdateEqEnumField(value);
                OnPropertyChanged(nameof(SelectedEquationString));
            }
        }
    }

    public EquationType SelectedEquation
    {
        get => _selectedEquation;
        private set
        {
            if (_selectedEquation != value)
            {
                _selectedEquation = value;
                UpdateEquationFormula();
            }
        }
    }

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

    public void LoadAxes()
    {
        var allAxes = storage.GetAllGraphNames()
            .SelectMany(graphName => storage.GetGraph(graphName).Axes)
            .Distinct()
            .ToList();

        if (AvailableAxes != null && AvailableAxes.SequenceEqual(allAxes))
        {
            return;
        }

        var previousXAxis = SelectedXAxis;
        var previousYAxis = SelectedYAxis;

        AvailableAxes = allAxes;

        if (!AvailableAxes.Contains(previousXAxis))
            SelectedXAxis = AvailableAxes.FirstOrDefault();
        else
            SelectedXAxis = previousXAxis;

        if (!AvailableAxes.Contains(previousYAxis))
            SelectedYAxis = AvailableAxes.Skip(1).FirstOrDefault() ?? SelectedXAxis;
        else
            SelectedYAxis = previousYAxis;
    }

    private void InitializePeriodicUpdate()
    {
        _updateTimer = new System.Timers.Timer(Constants.CHART_UPDATE_TIME);
        _updateTimer.AutoReset = true;
        _updateTimer.Elapsed += (sender, e) => Application.Current.Dispatcher.Invoke(() =>
        {
            DrawGraphs();
        });
        _updateTimer.Start();
    }

    private void UpdateEquationFormula()
    {
        Parameters.Clear();

        switch (SelectedEquation)
        {
            case EquationType.VanDerPol:
                EquationFormula = "y₀' = y₁\n"
                                + "y₁' = (μ (1 - y₀²) y₁ - y₀) / p";
                GraphTitle = "График Ван дер Поля";
                Parameters.Add(new ParameterViewModel("mu", "μ", 6.0m));
                Parameters.Add(new ParameterViewModel("p", "p", 1.0m));
                Parameters.Add(new ParameterViewModel("y0_init", "н.у. y₀", 2.0m));
                Parameters.Add(new ParameterViewModel("y1_init", "н.у. y₁", 0.0m));
                break;

            case EquationType.ForcedOscillator:
                EquationFormula = "y₀' = y₁\n"
                                + "y₁' = -ω²y₀ - γy₁ + F cos(ωₖ t)";
                GraphTitle = "График Принужденного осциллятора";
                Parameters.Add(new ParameterViewModel("omega", "ω", 2.0m));
                Parameters.Add(new ParameterViewModel("gamma", "γ", 0.1m));
                Parameters.Add(new ParameterViewModel("F", "F", 1.0m));
                Parameters.Add(new ParameterViewModel("omega_k", "ωₖ", 1.5m));
                Parameters.Add(new ParameterViewModel("y0_init", "н.у. y₀", 1.0m));
                Parameters.Add(new ParameterViewModel("y1_init", "н.у. y₁", 0.0m));
                break;

            case EquationType.RobertsonSystem:
                EquationFormula = "y₀' = -k₁y₀ + k₂y₁y₂\n"
                                + "y₁' = k₁y₀ - k₂y₁y₂ - k₃y₁²\n"
                                + "y₂' = k₃y₁²";
                GraphTitle = "График Системы Робертсона";
                Parameters.Add(new ParameterViewModel("k1", "k₁", 0.04m));
                Parameters.Add(new ParameterViewModel("k2", "k₂", 100m));
                Parameters.Add(new ParameterViewModel("k3", "k₃", 300000m));
                Parameters.Add(new ParameterViewModel("y0_init", "н.у. y₀", 1.0m));
                Parameters.Add(new ParameterViewModel("y1_init", "н.у. y₁", 0.0m));
                Parameters.Add(new ParameterViewModel("y2_init", "н.у. y₂", 0.0m));
                break;

            default:
                EquationFormula = "Выберите уравнение";
                GraphTitle = "Неизвестный график";
                break;
        }

        Parameters.Add(new ParameterViewModel("t0", "t₀", 0.0m));
        Parameters.Add(new ParameterViewModel("t1", "t₁", 20.0m));
        Parameters.Add(new ParameterViewModel("tolerance", "ε", 0.0001m));
    }

    public void DrawGraphs()
    {
        double minX = double.MaxValue, maxX = double.MinValue;

        foreach (var axis in Chart.AxisX)
        {
            if (!double.IsNaN(axis.MinValue) && !double.IsNaN(axis.MaxValue))
            {
                minX = Math.Min(minX, axis.MinValue);
                maxX = Math.Max(maxX, axis.MaxValue);
            }
        }

        double shift = Math.Abs(maxX - minX) * 0.05;
        minX -= shift;
        maxX += shift;

        if (_lastMin == minX && _lastMax == maxX)
        {
            if (!_draw_anyway) return;
        }

        _draw_anyway = false;

        chartManager.ClearCharts();

        foreach (var graphName in storage.GetAllGraphNames())
        {
            var graph = storage.GetGraph(graphName);

            int xIndex = graph.Axes.IndexOf(SelectedXAxis);
            int yIndex = graph.Axes.IndexOf(SelectedYAxis);

            if (xIndex == -1 || yIndex == -1)
            {
                MessageBox.Show($"Не найдены данные для осей X: {XAxis} и Y: {YAxis} в графике '{graphName}'");
                continue;
            }

            var visiblePoints = graph.Points
                .Where(point => point[xIndex] >= minX && point[xIndex] <= maxX)
                .ToList();

            var downsampledPoints = DownsamplePoints(visiblePoints, Constants.MAX_POINTS_PER_CHART, xIndex, yIndex);

            chartManager.AddChart(graphName, downsampledPoints);
        }

        _lastMin = minX;
        _lastMax = maxX;
    }

    private List<Point> DownsamplePoints(List<double[]> points, int maxPoints, int xIndex, int yIndex)
    {
        if (points.Count <= maxPoints)
        {
            return points.Select(p => new Point(p[xIndex], p[yIndex])).ToList();
        }

        var step = (double)points.Count / maxPoints;
        var downsampled = new List<Point>();

        for (int i = 0; i < maxPoints; i++)
        {
            var index = (int)(i * step);
            downsampled.Add(new Point(points[index][xIndex], points[index][yIndex]));
        }

        return downsampled;
    }

    public RelayCommand SaveAsImageCommand
    {
        get => _saveAsImageCommand ??= new RelayCommand(SaveFile);
    }

    public RelayCommand DeleteChartsCommand
    {
        get => _deleteChartsCommand ??= new RelayCommand(_ =>
        {
            storage.Clear();

            chartManager.ClearCharts();

            AvailableAxes = new List<string>();
            SelectedXAxis = null;
            SelectedYAxis = null;
        });
    }

    public RelayCommand SolveODE
    {
        get => _solveODE ??= new RelayCommand(_ => Solve());
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

    public RelayCommand AboutCommand
    {
        get => _aboutCommand ??= new RelayCommand(OpenAboutWindow);
    }

    private void ShowErrorMessage(string message)
    {
        Application.Current.Dispatcher.Invoke(() =>
        {
            ErrorSnackbar.MessageQueue?.Enqueue(message);
        });
    }

    private async void Solve()
    {
        if (!ValidateParameters(out string errorMessage))
        {
            ShowErrorMessage(errorMessage);
            return;
        }

        Console.WriteLine($"Выбранный метод решения: {SelectedMethod.ToString()}");
        Console.WriteLine($"Выбранное уравнение: {SelectedEquation.ToString()}");
        Console.WriteLine($"Название графика: {GraphTitle}");

        Console.WriteLine("Параметры:");
        foreach (var parameter in Parameters)
        {
            Console.WriteLine($"  {parameter.Key}: {parameter.Value}");
        }

        var taskData = new TaskData
        {
            Method = SelectedMethod.ToString(),
            Equation = SelectedEquation.ToString(),
            GraphTitle = GraphTitle,
            Parameters = ParameterConverter.ToDictionary(Parameters)
        };

        Console.WriteLine("Отправка данных на сервер...");
        ServerResponse = await _taskSolverService.SolveTaskAsync(taskData);
        Console.WriteLine($"Ответ от сервера: {ServerResponse}");

        storage.AddGraph($"{storage.GetGraphCount() + 1}. {GraphTitle}", new SimulationResult(ServerResponse));
        LoadAxes();
        UpdateScale();
        _draw_anyway = true;
    }

    private bool ValidateParameters(out string errorMessage)
    {
        errorMessage = string.Empty;

        foreach (var parameter in Parameters)
        {
            if (parameter.Value < Constants.MIN_VALUE || parameter.Value > Constants.MAX_VALUE)
            {
                errorMessage = $"\"{parameter.Name}\" выходит за допустимые пределы ({Constants.MIN_VALUE}, {Constants.MAX_VALUE}).";
                return false;
            }
        }

        if (Parameters.FirstOrDefault(p => p.Key == "t1")?.Value <= Parameters.FirstOrDefault(p => p.Key == "t0")?.Value)
        {
            errorMessage = "t₀ должно быть меньше t₁.";
            return false;
        }

        return true;
    }

    private void OpenAboutWindow(object obj)
    {
        var aboutWindow = new AboutWindow();
        aboutWindow.ShowDialog();
    }

    private void Zoom(double factor)
    {
        if (Chart == null) return;
        double minX = double.MaxValue, maxX = double.MinValue;

        foreach (var axis in Chart.AxisX)
        {
            AdjustAxis(axis, factor);

            if (!double.IsNaN(axis.MinValue) && !double.IsNaN(axis.MaxValue))
            {
                minX = Math.Min(minX, axis.MinValue);
                maxX = Math.Max(maxX, axis.MaxValue);
            }
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
        if (storage == null || string.IsNullOrEmpty(SelectedXAxis) || string.IsNullOrEmpty(SelectedYAxis))
            return;

        double globalXMin = double.MaxValue;
        double globalXMax = double.MinValue;
        double globalYMin = double.MaxValue;
        double globalYMax = double.MinValue;
        bool hasData = false;

        foreach (var graphName in storage.GetAllGraphNames())
        {
            var graph = storage.GetGraph(graphName);

            int xIndex = graph.Axes.IndexOf(SelectedXAxis);
            int yIndex = graph.Axes.IndexOf(SelectedYAxis);

            if (xIndex == -1 || yIndex == -1)
                continue;

            foreach (var point in graph.Points)
            {
                hasData = true;

                globalXMin = Math.Min(globalXMin, point[xIndex]);
                globalXMax = Math.Max(globalXMax, point[xIndex]);
                globalYMin = Math.Min(globalYMin, point[yIndex]);
                globalYMax = Math.Max(globalYMax, point[yIndex]);
            }
        }

        if (!hasData)
            return;

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

    private void UpdateSolverEnumField(string method)
    {
        if (Enum.TryParse(method, out SolutionMethod parsedMethod))
        {
            SelectedMethod = parsedMethod;
        }
    }

    private void UpdateEqEnumField(string equation)
    {
        SelectedEquation = EquationMapper.GetEquationEnum(equation);
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
