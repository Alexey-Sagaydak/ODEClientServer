namespace Client;

public enum SolutionMethod
{
    ExplicitEuler,
    RungeKutta2,
    RK23S,
    STEKS,
    DISPD,
    DISPF,
    DISPS
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

    public static readonly decimal MIN_VALUE = -10e7m;
    public static readonly decimal MAX_VALUE = 10e7m;

    public static readonly int LOW_UPDATE_TIME = 1000;
    public static readonly int MEDIUM_UPDATE_TIME = 400;
    public static readonly int HIGH_UPDATE_TIME = 100;

    public static readonly int LOW_QUALITY = 100;
    public static readonly int MEDIUM_QUALITY = 200;
    public static readonly int HIGH_QUALITY = 400;

    public static readonly Dictionary<SolutionMethod, string> Methods = new()
    {
        { SolutionMethod.ExplicitEuler, "Метод Эйлера" },
        { SolutionMethod.RungeKutta2, "RK21" },
        { SolutionMethod.RK23S, "RK23S" },
        { SolutionMethod.STEKS, "STEKS" },
        { SolutionMethod.DISPD, "DISPD" },
        { SolutionMethod.DISPF, "DISPF" },
        { SolutionMethod.DISPS, "DISPS" }
    };
}
