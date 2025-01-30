using System.IO;
using ClosedXML.Excel;

namespace Client;

public class GraphStorage
{
    private Dictionary<string, SimulationResult> _graphs;

    public GraphStorage()
    {
        _graphs = new Dictionary<string, SimulationResult>();
    }

    public void AddGraph(string name, SimulationResult graph)
    {
        if (string.IsNullOrWhiteSpace(name))
        {
            throw new ArgumentException("Имя графика не может быть пустым.");
        }

        if (_graphs.ContainsKey(name))
        {
            throw new InvalidOperationException($"График с именем '{name}' уже существует.");
        }

        _graphs[name] = graph;
    }

    public bool RemoveGraph(string name)
    {
        return _graphs.Remove(name);
    }

    public void Clear()
    {
        _graphs.Clear();
    }

    public SimulationResult GetGraph(string name)
    {
        if (!_graphs.ContainsKey(name))
        {
            throw new KeyNotFoundException($"График с именем '{name}' не найден.");
        }

        return _graphs[name];
    }

    public List<string> GetAllGraphNames()
    {
        return _graphs.Keys.ToList();
    }

    public int GetGraphCount()
    {
        return _graphs.Count;
    }

    public void SaveGraphsToCsv(string filePath)
    {
        using (var workbook = new XLWorkbook())
        {
            foreach (var (name, graph) in _graphs)
            {
                var worksheet = workbook.Worksheets.Add(name);

                for (int i = 0; i < graph.Axes.Count; i++)
                {
                    worksheet.Cell(1, i + 1).Value = graph.Axes[i];
                }

                for (int i = 0; i < graph.Points.Count; i++)
                {
                    var point = graph.Points[i];
                    for (int j = 0; j < point.Length; j++)
                    {
                        worksheet.Cell(i + 2, j + 1).Value = point[j];
                    }
                }
            }

            workbook.SaveAs(filePath);
        }
    }

    public void SaveGraphsToTxt(string directoryPath)
    {
        if (!Directory.Exists(directoryPath))
        {
            Directory.CreateDirectory(directoryPath);
        }

        foreach (var (name, graph) in _graphs)
        {
            string filePath = Path.Combine(directoryPath, $"{name}.txt");

            using (var writer = new StreamWriter(filePath))
            {
                writer.WriteLine(string.Join("\t", graph.Axes));

                foreach (var point in graph.Points)
                {
                    writer.WriteLine(string.Join("\t", point));
                }
            }
        }
    }
}
