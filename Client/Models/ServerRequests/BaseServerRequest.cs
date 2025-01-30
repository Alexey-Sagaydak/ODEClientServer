namespace Client;

public abstract class BaseServerRequest : IServerRequest
{
    public abstract Task<bool> ExecuteAsync(string serverUrl);
}
