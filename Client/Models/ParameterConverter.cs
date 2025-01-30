using System.Collections.ObjectModel;

namespace Client;

public static class ParameterConverter
{
    /// <summary>
    /// Преобразует ObservableCollection в Dictionary.
    /// </summary>
    /// <param name="parameters">Коллекция параметров.</param>
    /// <returns>Словарь, где ключ — строка, а значение — decimal.</returns>
    public static Dictionary<string, decimal> ToDictionary(ObservableCollection<ParameterViewModel> parameters)
    {
        if (parameters == null)
            throw new ArgumentNullException(nameof(parameters));

        return parameters.ToDictionary(
            parameter => parameter.Key,
            parameter => parameter.Value
        );
    }
}
