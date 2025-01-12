using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

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
