using System.Net.Http;

namespace Client;

public class PingRequest : BaseServerRequest
{
    public override async Task<bool> ExecuteAsync(string serverUrl)
    {
        try
        {
            using var client = new HttpClient();
            client.Timeout = TimeSpan.FromSeconds(2);
            var response = await client.GetAsync(serverUrl);
            return response.IsSuccessStatusCode;
        }
        catch
        {
            return false;
        }
    }
}
