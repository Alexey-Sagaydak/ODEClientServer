using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

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
