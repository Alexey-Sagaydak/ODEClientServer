﻿using LiveCharts.Wpf;
using LiveCharts;
using System.Reflection.Emit;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Client;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow : Window
{
    public SeriesCollection SeriesCollection { get; private set; }
    public string[] Labels { get; set; }
    public Func<double, string> YFormatter { get; set; }

    public MainWindow()
    {
        InitializeComponent();

        MainWindowViewModel viewModel = new(Chart, ErrorSnackbar);
        DataContext = viewModel;
        viewModel.RequestClose += (sender, e) => Close();
        FirstRB.IsChecked = true;

        EquationSelector.SelectedIndex = 0;
        SetInitialZoom();
    }

    private void SetInitialZoom()
    {
        foreach (var axis in Chart.AxisX)
        {
            axis.MinValue = 0;
            axis.MaxValue = 10;
        }

        foreach (var axis in Chart.AxisY)
        {
            axis.MinValue = 0;
            axis.MaxValue = 10;
        }
    }

    private void RadioButton_Checked(object sender, RoutedEventArgs e)
    {
        if (sender is RadioButton radioButton && radioButton.Tag is string tag)
        {
            if (DataContext is MainWindowViewModel viewModel)
            {
                viewModel.SelectedMethodString = tag;
            }
        }
    }
}
