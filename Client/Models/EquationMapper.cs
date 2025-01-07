using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace Client;

public class EquationMapper
{
    private static readonly Dictionary<string, EquationType> StringToEnumMap = new()
    {
        { "System.Windows.Controls.ComboBoxItem: Уравнение Ван дер Поля", EquationType.VanDerPol },
        { "System.Windows.Controls.ComboBoxItem: Принужденный осциллятор", EquationType.ForcedOscillator },
        { "System.Windows.Controls.ComboBoxItem: Система Робертсона", EquationType.RobertsonSystem }
    };

    private static readonly Dictionary<EquationType, string> EnumToStringMap = new()
    {
        { EquationType.VanDerPol, "System.Windows.Controls.ComboBoxItem: Уравнение Ван дер Поля" },
        { EquationType.ForcedOscillator, "System.Windows.Controls.ComboBoxItem: Принужденный осциллятор" },
        { EquationType.RobertsonSystem, "System.Windows.Controls.ComboBoxItem: Система Робертсона" }
    };

    public static EquationType GetEquationEnum(string equation)
    {
        return StringToEnumMap.TryGetValue(equation, out var enumValue)
            ? enumValue
            : EquationType.Unknown;
    }

    public static string GetEquationString(EquationType equationType)
    {
        return EnumToStringMap.TryGetValue(equationType, out var stringValue)
            ? stringValue
            : "Неизвестное уравнение";
    }
}
