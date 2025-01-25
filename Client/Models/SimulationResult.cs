using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace Client;

public class SimulationResult
{
    public List<string> Axes { get; private set; }

    public List<double[]> Points { get; private set; }

    public SimulationResult(string json)
    {
        var rawData = JsonConvert.DeserializeObject<RawResult>(json);

        Axes = new List<string> { "t" };
        if (rawData.Results.Any())
        {
            int valuesCount = rawData.Results.First().Values.Count;
            for (int i = 0; i < valuesCount; i++)
            {
                Axes.Add($"y{i}");
            }
        }

        Points = rawData.Results
            .Select(r => new[] { r.T }.Concat(r.Values).ToArray())
            .ToList();
    }

    private class RawResult
    {
        [JsonProperty("results")]
        public List<RawPoint> Results { get; set; }
    }

    private class RawPoint
    {
        [JsonProperty("t")]
        public double T { get; set; }

        [JsonProperty("values")]
        public List<double> Values { get; set; }
    }
}
