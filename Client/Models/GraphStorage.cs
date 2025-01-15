using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

    public void PrintAllGraphs()
    {
        Console.WriteLine("Графики в хранилище:");
        foreach (var name in _graphs.Keys)
        {
            Console.WriteLine($"- {name}");
        }
    }
}
