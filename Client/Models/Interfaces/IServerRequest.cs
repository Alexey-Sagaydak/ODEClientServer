using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
