using LiveCharts.Wpf;
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

        MainWindowViewModel viewModel = new(Chart);
        DataContext = viewModel;
        viewModel.RequestClose += (sender, e) => Close();

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

    private void EquationSelector_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        string selectedEquation = ((ComboBoxItem)((ComboBox)sender).SelectedItem).Content.ToString();

        ParametersPanel.Children.Clear();
        FirstDependencySelector.Items.Clear();
        SecondDependencySelector.Items.Clear();

        switch (selectedEquation)
        {
            case "Уравнение Ван дер Поля":
                EquationFormula.Text = "y₀' = y₁\n"
                                     + "y₁' = (μ (1 - y₀²) y₁ - y₀) / p";
                AddParameterField("μ", 6.0m);
                AddParameterField("p", 1.0m);
                AddParameterField("н.у. y₀", 2.0m);
                AddParameterField("н.у. y₁", 0.0m);
                ConfigureDependencySelectors("y₀", "y₁", "t");
                break;

            case "Принужденный осциллятор":
                EquationFormula.Text = "y₀' = y₁\n"
                                     + "y₁' = -ω²y₀ - γy₁ + F cos(ωₖ t)";
                AddParameterField("ω", 2.0m);
                AddParameterField("γ", 0.1m);
                AddParameterField("F", 1.0m);
                AddParameterField("ωₖ", 1.5m);
                AddParameterField("н.у. y₀", 1.0m);
                AddParameterField("н.у. y₁", 0.0m);
                ConfigureDependencySelectors("y₀", "y₁", "t");
                break;

            case "Система Робертсона":
                EquationFormula.Text = "y₀' = -0.04y₀ + 10⁴y₁y₂\n"
                                     + "y₁' = 0.04y₀ - 10⁴y₁y₂ - 3×10⁷y₁²\n"
                                     + "y₂' = 3×10⁷y₁²";
                AddParameterField("н.у. y₀", 1.0m);
                AddParameterField("н.у. y₁", 0.0m);
                AddParameterField("н.у. y₂", 0.0m);
                ConfigureDependencySelectors("y₀", "y₁", "y₂", "t");
                break;

            default:
                EquationFormula.Text = "Выберите уравнение";
                break;
        }
    }

    private void AddParameterField(string name, decimal defaultValue)
    {
        StackPanel parameterField = new StackPanel {
            Orientation = Orientation.Horizontal,
            Margin = new Thickness
            {
                Left = 5,
                Right = 0,
                Top = 0,
                Bottom = 5,
            }
        };
        parameterField.Children.Add(new TextBlock { Text = $"{name}: ", Width = 70, FontSize = 16 });
        parameterField.Children.Add(new MaterialDesignThemes.Wpf.DecimalUpDown
            {
                Value = defaultValue,
                ValueStep = 0.1m,
                Minimum = -50,
                Maximum = 50,
                Width = 150,
                FontSize = 16,
                Margin = new Thickness(5),
                ToolTip = "Выберите значение"
            }
        );
        ParametersPanel.Children.Add(parameterField);
    }

    private void ConfigureDependencySelectors(params string[] variables)
    {
        FirstDependencySelector.Items.Clear();
        SecondDependencySelector.Items.Clear();

        foreach (var variable in variables)
        {
            FirstDependencySelector.Items.Add(variable);
            SecondDependencySelector.Items.Add(variable);
        }
    }

    private void SolveButton_Click(object sender, RoutedEventArgs e)
    {
        MessageBox.Show("Численное решение будет реализовано позже!");
    }
}
