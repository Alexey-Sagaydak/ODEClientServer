using Microsoft.Windows.Themes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Client;

public enum SolutionMethod
{
    ExplicitEuler,
    RungeKutta2,
    RK23S,
    STEKS,
    DISPD,
    DISPF
}

public enum EquationType
{
    Unknown,
    VanDerPol,
    ForcedOscillator,
    RobertsonSystem
}

public static class Constants
{
    public static readonly string SOCKET = "127.0.0.1:7777";
}
