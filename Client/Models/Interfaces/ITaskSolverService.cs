using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Client;

public interface ITaskSolverService
{
    /// <summary>
    /// Отправляет данные задачи на сервер и получает ответ.
    /// </summary>
    /// <param name="taskData">Данные задачи.</param>
    /// <returns>Ответ от сервера.</returns>
    Task<string> SolveTaskAsync(TaskData taskData);
}
