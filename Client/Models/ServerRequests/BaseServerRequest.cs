using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Client;

public abstract class BaseServerRequest : IServerRequest
{
    public abstract Task<bool> ExecuteAsync(string serverUrl);
}
