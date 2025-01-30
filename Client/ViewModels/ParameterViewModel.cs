namespace Client;

public class ParameterViewModel : ViewModelBase
{
    public string Key { get; }
    public string Name { get; }

    private decimal _value;
    public decimal Value
    {
        get => _value;
        set => SetProperty(ref _value, value);
    }

    public ParameterViewModel(string key, string name, decimal value)
    {
        Key = key;
        Name = name;
        Value = value;
    }
}
