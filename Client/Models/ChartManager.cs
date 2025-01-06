using LiveCharts;
using LiveCharts.Wpf;
using System.Windows;

namespace Client;

public class ChartManager
{
    private readonly CartesianChart _chart;

    public ChartManager(CartesianChart chart)
    {
        _chart = chart;
        _chart.Series = new SeriesCollection();
    }

    public void AddChart(string graphTitle, List<Point> dataPoints)
    {
        var chartValues = new ChartValues<double>(dataPoints.Select(p => p.Y));
        var xLabels = dataPoints.Select(p => p.X).ToList();

        var lineSeries = new LineSeries
        {
            
            Title = graphTitle,
            Values = chartValues,
            PointGeometry = null,
            LineSmoothness = 1,
            Fill = System.Windows.Media.Brushes.Transparent
        };

        _chart.Series.Add(lineSeries);

        if (_chart.AxisX.Count == 0)
        {
            _chart.AxisX.Add(new Axis
            {
                Labels = xLabels.Select(x => x.ToString()).ToArray(),
                Separator = new Separator { Step = 1 }
            });
        }

        _chart.Update(true, true);
    }

    public void UpdateScale()
    {
        double globalXMin = double.MaxValue;
        double globalXMax = double.MinValue;
        double globalYMin = double.MaxValue;
        double globalYMax = double.MinValue;

        foreach (var series in _chart.Series)
        {
            if (series is LineSeries lineSeries)
            {
                var values = lineSeries.Values as ChartValues<ChartPoint>;
                if (values != null)
                {
                    foreach (var point in values)
                    {
                        globalXMin = Math.Min(globalXMin, point.X);
                        globalXMax = Math.Max(globalXMax, point.X);
                        globalYMin = Math.Min(globalYMin, point.Y);
                        globalYMax = Math.Max(globalYMax, point.Y);
                    }
                }
            }
        }

        double xPadding = (globalXMax - globalXMin) * 0.3;
        double yPadding = (globalYMax - globalYMin) * 0.3;

        foreach (var axis in _chart.AxisX)
        {
            axis.MinValue = globalXMin - xPadding;
            axis.MaxValue = globalXMax + xPadding;
        }

        foreach (var axis in _chart.AxisY)
        {
            axis.MinValue = globalYMin - yPadding;
            axis.MaxValue = globalYMax + yPadding;
        }

        _chart.Update(true, false);
    }

    public void ClearCharts()
    {
        _chart.Series.Clear();
        _chart.Update(true, true);
    }
}
