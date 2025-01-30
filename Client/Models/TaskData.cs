using System.Text.Json;

namespace Client;

public class TaskData
{
    public string Method { get; set; }
    public string Equation { get; set; }
    public string GraphTitle { get; set; }
    public Dictionary<string, decimal> Parameters { get; set; }

    public string ToJson()
    {
        return JsonSerializer.Serialize(this, new JsonSerializerOptions
        {
            WriteIndented = true
        });
    }
}
