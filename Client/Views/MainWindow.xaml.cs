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
    public MainWindow()
    {
        InitializeComponent();

        MainWindowViewModel viewModel = new();
        DataContext = viewModel;

        EquationSelector.SelectedIndex = 0;
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
                AddParameterField("μ", "6.0");
                AddParameterField("p", "1.0");
                AddParameterField("Нач. условие y₀", "2.0");
                AddParameterField("Нач. условие y₁", "0.0");
                ConfigureDependencySelectors("y₀", "y₁", "t");
                break;

            case "Принужденный осциллятор":
                EquationFormula.Text = "y₀' = y₁\n"
                                     + "y₁' = -ω²y₀ - γy₁ + F cos(ωₖ t)";
                AddParameterField("ω", "2.0");
                AddParameterField("γ", "0.1");
                AddParameterField("F", "1.0");
                AddParameterField("ωₖ", "1.5");
                AddParameterField("Нач. условие y₀", "1.0");
                AddParameterField("Нач. условие y₁", "0.0");
                ConfigureDependencySelectors("y₀", "y₁", "t");
                break;

            case "Система Робертсона":
                EquationFormula.Text = "y₁' = -0.04y₁ + 10⁴y₂y₃\n"
                                     + "y₂' = 0.04y₁ - 10⁴y₂y₃ - 3×10⁷y₂²\n"
                                     + "y₃' = 3×10⁷y₂²";
                AddParameterField("Нач. условие y₁", "1.0");
                AddParameterField("Нач. условие y₂", "0.0");
                AddParameterField("Нач. условие y₃", "0.0");
                ConfigureDependencySelectors("y₀", "y₁", "y₂", "t");
                break;

            default:
                EquationFormula.Text = "Выберите уравнение";
                break;
        }
    }

    private void AddParameterField(string name, string defaultValue)
    {
        StackPanel parameterField = new StackPanel { Orientation = Orientation.Horizontal };
        parameterField.Children.Add(new TextBlock { Text = $"{name}: ", Width = 100 });
        parameterField.Children.Add(new TextBox { Text = defaultValue, Width = 150 });
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
