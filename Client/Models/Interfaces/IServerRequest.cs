namespace Client;

public interface IServerRequest
{
    /// <summary>
    /// Выполняет запрос к серверу.
    /// </summary>
    /// <param name="serverUrl">URL сервера.</param>
    /// <returns>Статус выполнения запроса.</returns>
    Task<bool> ExecuteAsync(string serverUrl);
}
