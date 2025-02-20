using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

namespace Client;

public partial class HistoryWindow : Window
{
    public ObservableCollection<HistoryEntry> HistoryEntries { get; set; }

    public HistoryWindow()
    {
        InitializeComponent();
        HistoryEntries = new ObservableCollection<HistoryEntry>();
        FillFakeHistory();
        DataContext = this;
    }

    private void FillFakeHistory()
    {
        string[] equations = { "Ван дер Поля", "Принужденный осциллятор", "Система Робертсона" };
        string[] methods = { "RK21", "RK23S", "STEKS", "DISPD", "DISPF", "DISPS" };

        var parametersForEquations = new Dictionary<string, string>
        {
            { "Ван дер Поля", "μ=6.0, p=1.0, н.у. y₀=2.0, н.у. y₁=0.0" },
            { "Принужденный осциллятор", "ω=2.0, γ=0.1, F=1.0, ωₖ=1.5, н.у. y₀=1.0, н.у. y₁=0.0" },
            { "Система Робертсона", "k₁=0.04, k₂=100, k₃=300000, н.у. y₀=1.0, н.у. y₁=0.0, н.у. y₂=0.0" }
        };

        Random rnd = new Random();

        for (int i = 0; i < 20; i++)
        {
            int eqIndex = rnd.Next(equations.Length);
            string task = equations[eqIndex];
            string method = methods[rnd.Next(methods.Length)];
            string parameters = parametersForEquations[task];

            int daysAgo = rnd.Next(0, 30);
            DateTime randomTime = DateTime.Now.AddDays(-daysAgo)
                .AddHours(rnd.Next(0, 24))
                .AddMinutes(rnd.Next(0, 60))
                .AddSeconds(rnd.Next(0, 60));
            string solutionTime = randomTime.ToString("dd.MM.yyyy HH:mm:ss");

            HistoryEntries.Add(new HistoryEntry
            {
                TaskName = task,
                MethodName = method,
                MethodParameters = parameters,
                SolutionTime = solutionTime
            });
        }
    }

    private void DownloadButton_Click(object sender, RoutedEventArgs e)
    {
        Button btn = sender as Button;
        if (btn != null)
        {
            HistoryEntry entry = btn.DataContext as HistoryEntry;
            if (entry != null)
            {
                MessageBox.Show($"Скачивание решения для задачи '{entry.TaskName}' методом '{entry.MethodName}'",
                                "Информация", MessageBoxButton.OK, MessageBoxImage.Information);
            }
        }
    }

    private void DeleteButton_Click(object sender, RoutedEventArgs e)
    {
        Button btn = sender as Button;
        if (btn != null)
        {
            HistoryEntry entry = btn.DataContext as HistoryEntry;
            if (entry != null)
            {
                if (MessageBox.Show($"Удалить запись для задачи '{entry.TaskName}'?",
                    "Подтверждение", MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes)
                {
                    HistoryEntries.Remove(entry);
                }
            }
        }
    }

    private void DeleteAllButton_Click(object sender, RoutedEventArgs e)
    {
        if (MessageBox.Show("Удалить все записи истории?",
            "Подтверждение", MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes)
        {
            HistoryEntries.Clear();
        }
    }
}

public class HistoryEntry
{
    public string TaskName { get; set; }
    public string MethodName { get; set; }
    public string MethodParameters { get; set; }
    public string SolutionTime { get; set; }
}
