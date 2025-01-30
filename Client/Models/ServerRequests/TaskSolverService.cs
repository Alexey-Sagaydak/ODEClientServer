using System.Net.Http;
using System.Text;

namespace Client;

public class TaskSolverService : ITaskSolverService
{
    private readonly string _serverUrl;

    public TaskSolverService(string serverUrl)
    {
        _serverUrl = serverUrl;
    }

    public async Task<string> SolveTaskAsync(TaskData taskData)
    {
        try
        {
            using var client = new HttpClient();
            var content = new StringContent(taskData.ToJson(), Encoding.UTF8, "application/json");
            var response = await client.PostAsync($"{_serverUrl}solve", content);

            if (response.IsSuccessStatusCode)
            {
                return await response.Content.ReadAsStringAsync();
            }
            else
            {
                return $"Ошибка: {response.StatusCode}";
            }
        }
        catch (Exception ex)
        {
            return $"Ошибка соединения: {ex.Message}";
        }
    }
}
