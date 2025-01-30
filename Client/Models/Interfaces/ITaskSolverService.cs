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
